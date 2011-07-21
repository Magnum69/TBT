
#include <tbt/RadixSort.h>

#include <iostream>
#include <fstream>
#include <iomanip>


#define RADIX 8
#define BASE (1 << RADIX)


using namespace std;



RadixSort::RadixSort()
{
	try {
		buildProgramFromSourceRel("radix.cl");
	} catch(tbt::Error err) {
		cerr << err.what() << endl;
	}

	// create kernels
	m_kernelCounting = createKernel("radixCounting_gpu");
	m_kernelPermute  = createKernel("radixPermute_gpu");

	m_kernelPrescanSum        = createKernel("prescanSum4");
	m_kernelPrescan           = createKernel("prescan_gpu");
	m_kernelPrescanWithOffset = createKernel("prescanWithOffset");

	m_kernelPrescanUpSweep   = createKernel("prescanUpSweep_gpu");
	m_kernelPrescanDownSweep = createKernel("prescanDownSweep_gpu");
}


#define NUM_THREADS 16
#define ELEMENTS_PER_THREAD 64
#define LOCAL_WORK 64
#define TOTAL_GROUP_ELEMENTS (NUM_THREADS*ELEMENTS_PER_THREAD)

bool RadixSort::run(cl::CommandQueue queue, cl_uint *a, cl_uint n)
{
	m_nElements = n;
	m_numGroups = n / (TOTAL_GROUP_ELEMENTS);

	//m_numPrescanGroups = min(256,m_maxWorkGroupSize);
	m_numPrescanGroups = (m_numGroups*BASE >= 4*256*256) ? 256*256 : 256;
	m_prescanInterval = (m_numGroups*BASE + m_numPrescanGroups-1) / m_numPrescanGroups;
	cl_uint rem = m_prescanInterval % 4;
	if(rem > 0) m_prescanInterval += 4-rem;

	cout << "n = " << m_nElements << endl;
	cout << "m = " << m_prescanInterval << endl;
	cout << "num groups         = " << m_numGroups << endl;
	cout << "num prescan groups = " << m_numPrescanGroups << endl;

	// create buffers
	try {
		//m_buffer_a      = cl::Buffer(m_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, n*sizeof(cl_uint), a);
		m_buffer_a      = cl::Buffer(tbt::getContext(), CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, n*sizeof(cl_uint), a);
		m_buffer_b      = cl::Buffer(tbt::getContext(), CL_MEM_READ_WRITE, n*sizeof(cl_uint));
		m_buffer_gcount = cl::Buffer(tbt::getContext(), CL_MEM_READ_WRITE, m_numGroups*BASE*sizeof(cl_uint));

		if(m_numPrescanGroups > 256)
			m_buffer_psum = cl::Buffer(tbt::getContext(), CL_MEM_READ_WRITE, 256*sizeof(cl_uint));

	} catch(cl::Error err) {
		cerr << "Could not allocate buffers!" << endl;
		return false;
	}

	// set kernel args (that do not change)
	m_kernelCounting.setArg(1, m_buffer_gcount);
	m_kernelPermute .setArg(2, m_buffer_gcount);

	m_kernelPrescanSum.setArg(0, m_buffer_gcount);
	m_kernelPrescanSum.setArg(2, m_prescanInterval);
	m_kernelPrescanSum.setArg(3, m_numGroups*BASE);

	m_kernelPrescanWithOffset.setArg(0, m_buffer_gcount);
	m_kernelPrescanWithOffset.setArg(2, m_prescanInterval);
	m_kernelPrescanWithOffset.setArg(3, m_numGroups*BASE);

	if(m_numPrescanGroups > 256) {
		m_kernelPrescan         .setArg(0, m_buffer_psum);
		m_kernelPrescanUpSweep  .setArg(1, m_buffer_psum);
		m_kernelPrescanDownSweep.setArg(1, m_buffer_psum);
	}

	// call for all 4 digits
	m_tKernelCounting = m_tKernelPermute = m_tKernelPrescan = m_tKernelPrescanSum = m_tKernelPrescanWithOffset = 0.0;

	runSingle(queue, m_buffer_a, m_buffer_b,  0);
	runSingle(queue, m_buffer_b, m_buffer_a,  8);
	runSingle(queue, m_buffer_a, m_buffer_b, 16);
	runSingle(queue, m_buffer_b, m_buffer_a, 24);

	void *ptr = queue.enqueueMapBuffer(m_buffer_a, CL_TRUE, CL_MAP_READ, 0, m_nElements*sizeof(cl_uint));
	//m_queue.enqueueReadBuffer(m_buffer_a, CL_TRUE, 0, m_nElements*sizeof(cl_uint), a);

	return true;
}


void RadixSort::runSingle(cl::CommandQueue queue, cl::Buffer &bufferSrc, cl::Buffer &bufferTgt, cl_uint shift)
{
	m_kernelCounting         .setArg(0, bufferSrc);
	m_kernelCounting         .setArg(2, shift);
	m_kernelPermute          .setArg(0, bufferSrc);
	m_kernelPermute          .setArg(1, bufferTgt);
	m_kernelPermute          .setArg(3, shift);
	m_kernelPrescanSum       .setArg(1, bufferTgt);
	m_kernelPrescanWithOffset.setArg(1, bufferTgt);

	if(m_numPrescanGroups > 256) {
		m_kernelPrescanUpSweep  .setArg(0, bufferTgt);
		m_kernelPrescanDownSweep.setArg(0, bufferTgt);

	} else {
		m_kernelPrescan.setArg(0, bufferTgt);
	}
	
	//cout << "Running kernels..." << flush;
	
	cl::Event evKernelCounting;
	cl::Event evKernelPrescanSum;
	cl::Event evKernelPrescanUpSweep;
	cl::Event evKernelPrescan;
	cl::Event evKernelPrescanDownSweep;
	cl::Event evKernelPrescanWithOffset;
	cl::Event evKernelPermute;
	
	enqueue1DRangeKernel(queue, m_kernelCounting, m_numGroups*LOCAL_WORK, LOCAL_WORK, 0, &evKernelCounting);

	enqueue1DRangeKernel(queue, m_kernelPrescanSum, m_numPrescanGroups, 0, 0, &evKernelPrescanSum);

	if(m_numPrescanGroups > 256) {
		enqueue1DRangeKernel(queue, m_kernelPrescanUpSweep,   m_numPrescanGroups/4, LOCAL_WORK, 0, &evKernelPrescanUpSweep);
		enqueue1DRangeKernel(queue, m_kernelPrescan,          LOCAL_WORK,           LOCAL_WORK, 0, &evKernelPrescan);
		enqueue1DRangeKernel(queue, m_kernelPrescanDownSweep, m_numPrescanGroups/4, LOCAL_WORK, 0, &evKernelPrescanDownSweep);

	} else {
		//m_queue.enqueueNDRangeKernel(m_kernelPrescan, cl::NullRange, cl::NDRange(m_numPrescanGroups), cl::NDRange(m_numPrescanGroups), &eventsPrescan, &evKernelPrescan);
		enqueue1DRangeKernel(queue, m_kernelPrescan, LOCAL_WORK, LOCAL_WORK, 0, &evKernelPrescan);
	}

	enqueue1DRangeKernel(queue, m_kernelPrescanWithOffset, m_numPrescanGroups, 0, 0, &evKernelPrescanWithOffset);

	enqueue1DRangeKernel(queue, m_kernelPermute, m_numGroups*LOCAL_WORK, LOCAL_WORK, 0, &evKernelPermute);

	queue.finish();
	//cout << "done." << endl;

	m_tKernelCounting          += getEventTime(evKernelCounting);
	m_tKernelPrescanSum        += getEventTime(evKernelPrescanSum);
	m_tKernelPrescan           += getEventTime(evKernelPrescan);
	m_tKernelPrescanWithOffset += getEventTime(evKernelPrescanWithOffset);
	m_tKernelPermute           += getEventTime(evKernelPermute);

	if(m_numPrescanGroups > 256)
		m_tKernelPrescan += getEventTime(evKernelPrescanUpSweep) + getEventTime(evKernelPrescanDownSweep);


//	cl_uint *gcount = new cl_uint[m_nElements];
//	cout << "Reading gcount..." << flush;
//#ifdef USE_OLD_KERNELS
//	m_queue.enqueueReadBuffer(m_buffer_gcount, CL_TRUE, 0, m_nElements*sizeof(cl_uint), gcount);
//#else
//	m_queue.enqueueReadBuffer(m_buffer_gcount, CL_TRUE, 0, m_numGroups*BASE*sizeof(cl_uint), gcount);
//#endif
//	cout << "done." << endl;
//	
//	cl_uint *b = new cl_uint[m_nElements];
//	cout << "Reading bufferTgt..." << flush;
//	m_queue.enqueueReadBuffer(bufferTgt, CL_TRUE, 0, m_nElements*sizeof(cl_uint), b);
//	cout << "done." << endl;
//
//	if(m_nElements <= 1024) {
		//for(cl_uint i = 0; i < m_numGroups*BASE; ++i)
		//	cout << gcount[i] << "  ";
		//cout << endl;
//
//		for(cl_uint i = 0; i < m_nElements; ++i) {
//			cout << "b[" << setw(4) << i << "] = " << setw(10) << b[i];
//			cout << "   " << setw(3) << (b[i] & 0xffu)
//				<< " | " << setw(3) << ((b[i] >> 8) & 0xffu)
//				<< " | " << setw(3) << ((b[i] >> 16) & 0xffu)
//				<< " | " << setw(3) << ((b[i] >> 24) & 0xffu) << endl;
//		}
//	}
//
//	delete [] b;
//	delete [] gcount;
}

