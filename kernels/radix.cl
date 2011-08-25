
//#pragma OPENCL EXTENSION cl_intel_printf : enable


#define RADIX 8
#define BASE (1 << RADIX)

#define MAX_LOCAL_WORK 256

/*---------------------------------------------------------
                          prescanReduce
  ---------------------------------------------------------*/

  // assume work-group size = 256

__kernel
__attribute__((reqd_work_group_size(MAX_LOCAL_WORK, 1, 1)))
void prescanReduce(
	__global uint const * restrict a,
	__global uint       * restrict sum,
	uint m,
	uint n)
{
	size_t localID = get_local_id(0);
	size_t groupID = get_group_id(0);

	__local uint lsum[MAX_LOCAL_WORK];
	
	m >>= 2;
	__global uint4 *a4 = (__global uint4 *)a;

	size_t startIndex = m*groupID;
	size_t stopIndex = startIndex+m;

	uint4 sum4 = 0;
	for(size_t i = startIndex; i < stopIndex; i += MAX_LOCAL_WORK)
		sum4 += a4[i+localID];

	lsum[localID] = sum4.x+sum4.y+sum4.z+sum4.w;


	barrier(CLK_LOCAL_MEM_FENCE);

	if(localID < 64) {
		uint t1 = lsum[localID    ] + lsum[localID+ 64];
		uint t2 = lsum[localID+128] + lsum[localID+192];
		lsum[localID] = t1 + t2;
	}

	barrier(CLK_LOCAL_MEM_FENCE);  // not required for wave-front size >= 64

	if(localID < 16) {
		uint t1 = lsum[localID   ] + lsum[localID+16];
		uint t2 = lsum[localID+32] + lsum[localID+48];
		lsum[localID] = t1 + t2;
		//printf("group = %d, local = %d: %d\n", (int)groupID, (int)localID, (int)lsum[localID]);
	}

	barrier(CLK_LOCAL_MEM_FENCE);  // not required for wave-front size >= 16

	if(localID < 4) {
		uint t1 = lsum[localID   ] + lsum[localID+ 4];
		uint t2 = lsum[localID+ 8] + lsum[localID+12];
		lsum[localID] = t1 + t2;
	}

	barrier(CLK_LOCAL_MEM_FENCE);  // not required for wave-front size >= 4

	if(localID == 0)
		sum[groupID] = lsum[0]+lsum[1]+lsum[2]+lsum[3];
}


/*---------------------------------------------------------
                           prescanSum

	gcount:  input array of size m * #groups elements
	psum:    prescan array (each work item writes sum in interval to psum[globalID])
	m:       size of interval (#elements in gcount)
	n:       total size of gcount (#elements)

	Experimentation:
	- try to use 4-vectors (uint4) (make sure m is dividable by 4)
  ---------------------------------------------------------*/

__kernel
void prescanSum(
	__global uint const * restrict gcount,
	__global uint       * restrict psum,
	uint m,
	uint n)
{
	size_t globalID  = get_global_id(0);

	int startIndex = globalID * m;
	int stopIndex  = min(startIndex + m, n);

	uint sum = 0;
	for(int i = startIndex; i < stopIndex; i++)
		sum += gcount[i];

	psum[globalID] = sum;
}

__kernel
void prescanSum4(
	__global uint const * restrict gcount,
	__global uint       * restrict psum,
	uint m,
	uint n)
{
	size_t globalID  = get_global_id(0);
	m >>= 2;

	__global uint4 *gcount4 = (__global uint4 *)gcount;

	int startIndex = globalID * m;
	int stopIndex  = min(startIndex + m, n/4);

	uint4 sum4 = 0;
	for(int i = startIndex; i < stopIndex; i++)
		sum4 += gcount4[i];

	psum[globalID] = sum4.x + sum4.y + sum4.z + sum4.w;
}


/*---------------------------------------------------------
                        prescanWithOffset

	gcount:  array of size m * #groups elements, where we compute prescan
	psum:    aux. prescan array (each work item reads its offset from psum[globalID])
	m:       size of interval (#elements in gcount)
	n:       total size of gcount (#elements)
  ---------------------------------------------------------*/

__kernel
void prescanWithOffset(
	__global uint       * restrict gcount,
	__global uint const * restrict psum,
	uint m,
	uint n)
{
	size_t globalID  = get_global_id(0);

	int startIndex = globalID * m;
	int stopIndex  = min(startIndex + m, n);

	uint sum = psum[globalID];
	for(int i = startIndex; i < stopIndex; i++) {
		uint x = gcount[i];
		gcount[i] = sum;
		sum += x;
	}
}



//*************************************

#define NUM_THREADS 16
#define ELEMENTS_PER_THREAD 64
#define LOCAL_WORK 64
#define TOTAL_GROUP_ELEMENTS (NUM_THREADS*ELEMENTS_PER_THREAD)

//-------------------------------------


/*---------------------------------------------------------
                         prescan_gpu

	psum:  array of size 4*LOCAL_WORK in which we compute a prescan

	Experimentation:
	- try to unroll for-loops
  ---------------------------------------------------------*/

__kernel
__attribute__((reqd_work_group_size(LOCAL_WORK, 1, 1)))
void prescan_gpu(__global uint * restrict psum)
{
	uint localID = (uint)get_local_id(0);

	__local uint lpsum[LOCAL_WORK];
	__global uint4 *psum4 = (__global uint4 *) psum;

	uint4 value4 = psum4[localID];
	uint v_0 = value4.s0;
	uint v_1 = value4.s1;
	uint v_2 = value4.s2;
	uint v_3 = value4.s3;

	v_1 += v_0;
	lpsum[localID] = v_1 + v_2 + v_3;

	barrier(CLK_LOCAL_MEM_FENCE);

	// up-sweep
	uint d2;
	for(uint d = 1; d < LOCAL_WORK/2; d = d2)
	{
		d2 = 2*d;
		if((localID+1) % d2 == 0) {
			lpsum[localID] += lpsum[localID-d];
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}

	// clear + first step of down-sweep
	if(localID == LOCAL_WORK-1) {
		uint indexLeft = localID - LOCAL_WORK/2;

		lpsum[localID] = lpsum[indexLeft];
		lpsum[indexLeft] = 0;
	}

	barrier(CLK_LOCAL_MEM_FENCE);

	// down-sweep
	for(uint d = LOCAL_WORK/2; d >= 2; d = d2)
	{
		d2 = d/2;
		if((localID+1) % d == 0) {
			uint indexLeft = localID-d2;

			uint t = lpsum[indexLeft];
			lpsum[indexLeft] = lpsum[localID];
			lpsum[localID] += t;
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}

	uint x = lpsum[localID];
	uint t = x + v_1;
	uint4 result4 = (uint4)(x, x + v_0, t, t + v_2);
	
	psum4[localID] = result4;
}


/*---------------------------------------------------------
                      prescanUpSweep_gpu

	gsum:  array of size 4*global work, in which we want to compute a prescan
	psum:  array of size 4*LOCAL_WORK

	Experimentation:
	- try to unroll for-loop
  ---------------------------------------------------------*/

__kernel
__attribute__((reqd_work_group_size(LOCAL_WORK, 1, 1)))
void prescanUpSweep_gpu(
	__global uint * restrict gsum,
	__global uint * restrict psum)
{
	uint   localID  = (uint)get_local_id(0);
	size_t globalID = get_global_id(0);
	size_t groupID  = get_group_id(0);

	__local uint lpsum[LOCAL_WORK];
	__global uint4 *gsum4 = (__global uint4 *) gsum;

	uint4 value4 = gsum4[globalID];
	value4.s1 += value4.s0;
	lpsum[localID] = value4.s1 + value4.s2 +value4.s3;

	barrier(CLK_LOCAL_MEM_FENCE);

	// up-sweep
	uint d2;
	for(uint d = 1; d < LOCAL_WORK; d = d2)
	{
		d2 = 2*d;
		if((localID+1) % d2 == 0) {
			lpsum[localID] += lpsum[localID-d];
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}

	value4.s3 = lpsum[localID];
	gsum4[globalID] = value4;
	
	if(localID == LOCAL_WORK-1)
		psum[groupID] = value4.s3;
}


/*---------------------------------------------------------
                     prescanDownSweep_gpu

	gsum:  array of size 4*global work, in which we want to compute a prescan
	psum:  array of size 4*LOCAL_WORK

	Experimentation:
	- try to unroll for-loop
  ---------------------------------------------------------*/

__kernel
__attribute__((reqd_work_group_size(LOCAL_WORK, 1, 1)))
void prescanDownSweep_gpu(
	__global uint       * restrict gsum,
	__global uint const * restrict psum)
{
	uint   localID  = (uint)get_local_id(0);
	size_t globalID = get_global_id(0);
	size_t groupID  = get_group_id(0);

	__local uint lpsum[LOCAL_WORK];
	__global uint4 *gsum4 = (__global uint4 *) gsum;

	uint4 value4 = gsum4[globalID];
	lpsum[localID] = (localID == LOCAL_WORK-1) ? psum[groupID] : value4.s3;

	barrier(CLK_LOCAL_MEM_FENCE);

	// down-sweep
	uint d2;
	for(uint d = LOCAL_WORK; d >= 2; d = d2)
	{
		d2 = d/2;
		if((localID+1) % d == 0) {
			uint indexLeft = localID-d2;

			uint t = lpsum[indexLeft];
			lpsum[indexLeft] = lpsum[localID];
			lpsum[localID] += t;
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}

	uint x = lpsum[localID];
	uint t = x + value4.s1;
	gsum4[globalID] = (uint4)(x, x + value4.s0, t, t + value4.s2);
}



/*---------------------------------------------------------
                      prescanSum_gpu
					    (not used)
  ---------------------------------------------------------*/

__kernel
void prescanSum_gpu(
	__global uint const * restrict gcount,
	__global uint       * restrict psum,
	uint m,
	uint n)
{
	size_t localID   = get_local_id(0);
	size_t globalID  = get_global_id(0);
	size_t groupID   = get_group_id(0);
	size_t localSize = get_local_size(0);

	__local uint lsum;

	if(localID == 0)
		lsum = 0;

	uint startIndex = (uint)(groupID*m+localID);
	uint stopIndex = min(startIndex+m, n);

	uint4 sum4 = 0;
	for(size_t i = startIndex; i < stopIndex; i += localSize)
		sum4 += gcount[i];

	uint sum = sum4.x + sum4.y + sum4.z + sum4.w;

	barrier(CLK_LOCAL_MEM_FENCE);

	atomic_add(&lsum, sum);

	barrier(CLK_LOCAL_MEM_FENCE);

	if(localID == 0)
		psum[groupID] = lsum;
}


/*---------------------------------------------------------
                      radixCounting_gpu

	Experimentation:
	- try to unroll for-loop
  ---------------------------------------------------------*/

__kernel __attribute__((reqd_work_group_size(LOCAL_WORK, 1, 1)))
void radixCounting_gpu(
	__global uint const * restrict a,
    __global uint       * restrict gcount,
    uint shift)
{
	size_t localID   = get_local_id(0);
	size_t globalID  = get_global_id(0);
	size_t groupID   = get_group_id(0);
	size_t numGroups = get_num_groups(0);

	__local uint count[BASE*NUM_THREADS];

	__global uint4 *a4 = (__global uint4 *) (a + groupID*TOTAL_GROUP_ELEMENTS);
	__local uint4 *count4  = (__local uint4 *) count;
	
	for(size_t i = 0; i < (BASE*NUM_THREADS/4); i += (BASE/4))
		count4[i+localID] = 0;
	
	barrier(CLK_LOCAL_MEM_FENCE);

	if(localID < NUM_THREADS)
	{
		__local uint *mycount = count + localID*BASE;
		uint4 mask4 = (uint4)(0xffu,0xffu,0xffu,0xffu);

		for(size_t i = 0; i < (TOTAL_GROUP_ELEMENTS/4); i += NUM_THREADS) {
			uint4 bucket4 = ( a4[i+localID] >> shift ) & mask4;

			mycount[bucket4.x]++;
			mycount[bucket4.y]++;
			mycount[bucket4.z]++;
			mycount[bucket4.w]++;
		}
	}
	
	barrier(CLK_LOCAL_MEM_FENCE);
	
	uint4 sum4 = count4[localID];
	for(size_t i = (BASE/4); i < (BASE*NUM_THREADS/4); i += (BASE/4))
		sum4 += count4[i+localID];
	
	gcount[(4*localID  ) * numGroups + groupID] = sum4.x;
	gcount[(4*localID+1) * numGroups + groupID] = sum4.y;
	gcount[(4*localID+2) * numGroups + groupID] = sum4.z;
	gcount[(4*localID+3) * numGroups + groupID] = sum4.w;
}


__kernel __attribute__((reqd_work_group_size(LOCAL_WORK, 1, 1)))
void radixCounting_gpu_atomic(
	__global uint const * restrict a,
    __global uint       * restrict gcount,
    uint shift)
{
	size_t localID   = get_local_id(0);
	size_t globalID  = get_global_id(0);
	size_t groupID   = get_group_id(0);
	size_t numGroups = get_num_groups(0);

	__local uint count[BASE];

	__global uint4 *a4 = (__global uint4 *) (a + groupID*TOTAL_GROUP_ELEMENTS);
	__local uint4 *count4  = (__local uint4 *) count;
	
	count4[localID] = 0;
	
	barrier(CLK_LOCAL_MEM_FENCE);

	uint4 mask4 = (uint4)(0xffu,0xffu,0xffu,0xffu);

	for(size_t i = 0; i < (TOTAL_GROUP_ELEMENTS/4); i += LOCAL_WORK) {
		uint4 bucket4 = ( a4[i+localID] >> shift ) & mask4;

		atomic_inc(&count[bucket4.x]);
		atomic_inc(&count[bucket4.y]);
		atomic_inc(&count[bucket4.z]);
		atomic_inc(&count[bucket4.w]);
	}
	
	barrier(CLK_LOCAL_MEM_FENCE);
	
	uint4 sum4 = count4[localID];
	
	gcount[(4*localID  ) * numGroups + groupID] = sum4.x;
	gcount[(4*localID+1) * numGroups + groupID] = sum4.y;
	gcount[(4*localID+2) * numGroups + groupID] = sum4.z;
	gcount[(4*localID+3) * numGroups + groupID] = sum4.w;
}


/*---------------------------------------------------------
                        radixPermute_gpu

	Experimentation:
	- try to unroll for-loop
	- don't store digits, rather recompute
  ---------------------------------------------------------*/

__kernel __attribute__((reqd_work_group_size(LOCAL_WORK, 1, 1)))
void radixPermute_gpu(
	__global uint const * restrict a,
    __global uint       * restrict b,
	__global uint const * restrict gcount,
	uint shift)
{
	size_t localID   = get_local_id(0);
	size_t globalID  = get_global_id(0);
	size_t groupID   = get_group_id(0);
	size_t numGroups = get_num_groups(0);

	__local uint count[NUM_THREADS*BASE];
	__local uint4 *count4  = (__local uint4 *) count;

	for(size_t i = 0; i < (BASE*NUM_THREADS/4); i += (BASE/4))
		count4[i+localID] = 0;
	
	barrier(CLK_LOCAL_MEM_FENCE);

	__local uint *mycount = count + localID*BASE;
	uint4 mask4 = (uint4)(0xffu,0xffu,0xffu,0xffu);

	uint4 value_0, value_1, value_2, value_3, value_4, value_5, value_6, value_7;
	uint4 value_8, value_9, value_a, value_b, value_c, value_d, value_e, value_f;
	uint4 digit_0, digit_1, digit_2, digit_3, digit_4, digit_5, digit_6, digit_7;
	uint4 digit_8, digit_9, digit_a, digit_b, digit_c, digit_d, digit_e, digit_f;

	if(localID < NUM_THREADS)
	{
		__global uint4 *mya4 = (__global uint4 *) (a + groupID*TOTAL_GROUP_ELEMENTS + localID*ELEMENTS_PER_THREAD);

		value_0 = mya4[0];
		digit_0 = (value_0 >> shift) & mask4;
		mycount[digit_0.x]++; mycount[digit_0.y]++; mycount[digit_0.z]++; mycount[digit_0.w]++;

		value_1 = mya4[1];
		digit_1 = (value_1 >> shift) & mask4;
		mycount[digit_1.x]++; mycount[digit_1.y]++; mycount[digit_1.z]++; mycount[digit_1.w]++;

		value_2 = mya4[2];
		digit_2 = (value_2 >> shift) & mask4;
		mycount[digit_2.x]++; mycount[digit_2.y]++; mycount[digit_2.z]++; mycount[digit_2.w]++;

		value_3 = mya4[3];
		digit_3 = (value_3 >> shift) & mask4;
		mycount[digit_3.x]++; mycount[digit_3.y]++; mycount[digit_3.z]++; mycount[digit_3.w]++;

		value_4 = mya4[4];
		digit_4 = (value_4 >> shift) & mask4;
		mycount[digit_4.x]++; mycount[digit_4.y]++; mycount[digit_4.z]++; mycount[digit_4.w]++;

		value_5 = mya4[5];
		digit_5 = (value_5 >> shift) & mask4;
		mycount[digit_5.x]++; mycount[digit_5.y]++; mycount[digit_5.z]++; mycount[digit_5.w]++;

		value_6 = mya4[6];
		digit_6 = (value_6 >> shift) & mask4;
		mycount[digit_6.x]++; mycount[digit_6.y]++; mycount[digit_6.z]++; mycount[digit_6.w]++;

		value_7 = mya4[7];
		digit_7 = (value_7 >> shift) & mask4;
		mycount[digit_7.x]++; mycount[digit_7.y]++; mycount[digit_7.z]++; mycount[digit_7.w]++;

		value_8 = mya4[8];
		digit_8 = (value_8 >> shift) & mask4;
		mycount[digit_8.x]++; mycount[digit_8.y]++; mycount[digit_8.z]++; mycount[digit_8.w]++;

		value_9 = mya4[9];
		digit_9 = (value_9 >> shift) & mask4;
		mycount[digit_9.x]++; mycount[digit_9.y]++; mycount[digit_9.z]++; mycount[digit_9.w]++;

		value_a = mya4[10];
		digit_a = (value_a >> shift) & mask4;
		mycount[digit_a.x]++; mycount[digit_a.y]++; mycount[digit_a.z]++; mycount[digit_a.w]++;

		value_b = mya4[11];
		digit_b = (value_b >> shift) & mask4;
		mycount[digit_b.x]++; mycount[digit_b.y]++; mycount[digit_b.z]++; mycount[digit_b.w]++;

		value_c = mya4[12];
		digit_c = (value_c >> shift) & mask4;
		mycount[digit_c.x]++; mycount[digit_c.y]++; mycount[digit_c.z]++; mycount[digit_c.w]++;

		value_d = mya4[13];
		digit_d = (value_d >> shift) & mask4;
		mycount[digit_d.x]++; mycount[digit_d.y]++; mycount[digit_d.z]++; mycount[digit_d.w]++;

		value_e = mya4[14];
		digit_e = (value_e >> shift) & mask4;
		mycount[digit_e.x]++; mycount[digit_e.y]++; mycount[digit_e.z]++; mycount[digit_e.w]++;

		value_f = mya4[15];
		digit_f = (value_f >> shift) & mask4;
		mycount[digit_f.x]++; mycount[digit_f.y]++; mycount[digit_f.z]++; mycount[digit_f.w]++;
	}

	uint4 sum4;
	sum4.x = gcount[(4*localID  ) * numGroups + groupID];
	sum4.y = gcount[(4*localID+1) * numGroups + groupID];
	sum4.z = gcount[(4*localID+2) * numGroups + groupID];
	sum4.w = gcount[(4*localID+3) * numGroups + groupID];

	barrier(CLK_LOCAL_MEM_FENCE);

	for(size_t i = 0; i < (BASE*NUM_THREADS/4); i += (BASE/4)) {
		uint4 t4 = count4[i+localID];
		count4[i+localID] = sum4;
		sum4 += t4;
	}

	barrier(CLK_LOCAL_MEM_FENCE);

	if(localID < NUM_THREADS)
	{
		 b[mycount[digit_0.x]++] = value_0.x;
		 b[mycount[digit_0.y]++] = value_0.y;
		 b[mycount[digit_0.z]++] = value_0.z;
		 b[mycount[digit_0.w]++] = value_0.w;

		 b[mycount[digit_1.x]++] = value_1.x;
		 b[mycount[digit_1.y]++] = value_1.y;
		 b[mycount[digit_1.z]++] = value_1.z;
		 b[mycount[digit_1.w]++] = value_1.w;

		 b[mycount[digit_2.x]++] = value_2.x;
		 b[mycount[digit_2.y]++] = value_2.y;
		 b[mycount[digit_2.z]++] = value_2.z;
		 b[mycount[digit_2.w]++] = value_2.w;

		 b[mycount[digit_3.x]++] = value_3.x;
		 b[mycount[digit_3.y]++] = value_3.y;
		 b[mycount[digit_3.z]++] = value_3.z;
		 b[mycount[digit_3.w]++] = value_3.w;

		 b[mycount[digit_4.x]++] = value_4.x;
		 b[mycount[digit_4.y]++] = value_4.y;
		 b[mycount[digit_4.z]++] = value_4.z;
		 b[mycount[digit_4.w]++] = value_4.w;

		 b[mycount[digit_5.x]++] = value_5.x;
		 b[mycount[digit_5.y]++] = value_5.y;
		 b[mycount[digit_5.z]++] = value_5.z;
		 b[mycount[digit_5.w]++] = value_5.w;

		 b[mycount[digit_6.x]++] = value_6.x;
		 b[mycount[digit_6.y]++] = value_6.y;
		 b[mycount[digit_6.z]++] = value_6.z;
		 b[mycount[digit_6.w]++] = value_6.w;

		 b[mycount[digit_7.x]++] = value_7.x;
		 b[mycount[digit_7.y]++] = value_7.y;
		 b[mycount[digit_7.z]++] = value_7.z;
		 b[mycount[digit_7.w]++] = value_7.w;

		 b[mycount[digit_8.x]++] = value_8.x;
		 b[mycount[digit_8.y]++] = value_8.y;
		 b[mycount[digit_8.z]++] = value_8.z;
		 b[mycount[digit_8.w]++] = value_8.w;

		 b[mycount[digit_9.x]++] = value_9.x;
		 b[mycount[digit_9.y]++] = value_9.y;
		 b[mycount[digit_9.z]++] = value_9.z;
		 b[mycount[digit_9.w]++] = value_9.w;

		 b[mycount[digit_a.x]++] = value_a.x;
		 b[mycount[digit_a.y]++] = value_a.y;
		 b[mycount[digit_a.z]++] = value_a.z;
		 b[mycount[digit_a.w]++] = value_a.w;

		 b[mycount[digit_b.x]++] = value_b.x;
		 b[mycount[digit_b.y]++] = value_b.y;
		 b[mycount[digit_b.z]++] = value_b.z;
		 b[mycount[digit_b.w]++] = value_b.w;

		 b[mycount[digit_c.x]++] = value_c.x;
		 b[mycount[digit_c.y]++] = value_c.y;
		 b[mycount[digit_c.z]++] = value_c.z;
		 b[mycount[digit_c.w]++] = value_c.w;

		 b[mycount[digit_d.x]++] = value_d.x;
		 b[mycount[digit_d.y]++] = value_d.y;
		 b[mycount[digit_d.z]++] = value_d.z;
		 b[mycount[digit_d.w]++] = value_d.w;

		 b[mycount[digit_e.x]++] = value_e.x;
		 b[mycount[digit_e.y]++] = value_e.y;
		 b[mycount[digit_e.z]++] = value_e.z;
		 b[mycount[digit_e.w]++] = value_e.w;

		 b[mycount[digit_f.x]++] = value_f.x;
		 b[mycount[digit_f.y]++] = value_f.y;
		 b[mycount[digit_f.z]++] = value_f.z;
		 b[mycount[digit_f.w]++] = value_f.w;
	}
}
