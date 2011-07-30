
#include <tbt/RadixSort.h>

#include <iostream>
#include <fstream>
#include <iomanip>


#define RADIX 8
#define BASE (1 << RADIX)


#define NUM_THREADS 16
#define ELEMENTS_PER_THREAD 64
#define LOCAL_WORK 64
#define TOTAL_GROUP_ELEMENTS (NUM_THREADS*ELEMENTS_PER_THREAD)



using namespace std;


namespace tbt
{

	cl::Kernel RadixSort::m_kernelCounting;
	cl::Kernel RadixSort::m_kernelPermute;

	cl::Kernel RadixSort::m_kernelPrescanSum;
	cl::Kernel RadixSort::m_kernelPrescan;
	cl::Kernel RadixSort::m_kernelPrescanWithOffset;

	cl::Kernel RadixSort::m_kernelPrescanUpSweep;
	cl::Kernel RadixSort::m_kernelPrescanDownSweep;


	void RadixSort::assureKernelsLoaded()
	{
		if(!isProgramLoaded()) {
			buildProgramFromSourceRel("radix.cl");

			// create kernels
			m_kernelCounting = createKernel("radixCounting_gpu");
			m_kernelPermute  = createKernel("radixPermute_gpu");

			m_kernelPrescanSum        = createKernel("prescanSum4");
			m_kernelPrescan           = createKernel("prescan_gpu");
			m_kernelPrescanWithOffset = createKernel("prescanWithOffset");

			m_kernelPrescanUpSweep   = createKernel("prescanUpSweep_gpu");
			m_kernelPrescanDownSweep = createKernel("prescanDownSweep_gpu");
		}
	}


	void RadixSort::run(DeviceArray<cl_uint> &array_a)
	{
		m_devCon = array_a.getDeviceController();
		cl_uint n = (cl_uint)array_a.size();

		assureKernelsLoaded();

		startTimer();

		m_nElements = n;
		m_numGroups = n / (TOTAL_GROUP_ELEMENTS);

		//m_numPrescanGroups = min(256,m_maxWorkGroupSize);
		m_numPrescanGroups = (m_numGroups*BASE >= 4*256*256) ? 256*256 : 256;
		m_prescanInterval = (m_numGroups*BASE + m_numPrescanGroups-1) / m_numPrescanGroups;
		cl_uint rem = m_prescanInterval % 4;
		if(rem > 0) m_prescanInterval += 4-rem;

		//cout << "n = " << m_nElements << endl;
		//cout << "m = " << m_prescanInterval << endl;
		//cout << "num groups         = " << m_numGroups << endl;
		//cout << "num prescan groups = " << m_numPrescanGroups << endl;

		// create device arrays
		DeviceArray<cl_uint> array_b(m_devCon, n);
		m_array_gcount = new DeviceArray<cl_uint>(m_devCon, m_numGroups*BASE);

		if(m_numPrescanGroups > 256)
			m_array_psum = new DeviceArray<cl_uint>(m_devCon, 256);

		// set kernel args (which do not change)
		m_kernelCounting.setArg<cl::Buffer>(1, *m_array_gcount);
		m_kernelPermute .setArg<cl::Buffer>(2, *m_array_gcount);

		m_kernelPrescanSum.setArg<cl::Buffer>(0, *m_array_gcount);
		m_kernelPrescanSum.setArg<cl_uint>   (2, m_prescanInterval);
		m_kernelPrescanSum.setArg<cl_uint>   (3, m_numGroups*BASE);

		m_kernelPrescanWithOffset.setArg<cl::Buffer>(0, *m_array_gcount);
		m_kernelPrescanWithOffset.setArg<cl_uint>   (2, m_prescanInterval);
		m_kernelPrescanWithOffset.setArg<cl_uint>   (3, m_numGroups*BASE);

		if(m_numPrescanGroups > 256) {
			m_kernelPrescan         .setArg<cl::Buffer>(0, *m_array_psum);
			m_kernelPrescanUpSweep  .setArg<cl::Buffer>(1, *m_array_psum);
			m_kernelPrescanDownSweep.setArg<cl::Buffer>(1, *m_array_psum);
		}

		// call for all 4 digits
		m_tKernelCounting = m_tKernelPermute = m_tKernelPrescan = m_tKernelPrescanSum = m_tKernelPrescanWithOffset = 0.0;

		runSingle(array_a, array_b,  0);
		runSingle(array_b, array_a,  8);
		runSingle(array_a, array_b, 16);
		runSingle(array_b, array_a, 24);

		//void *ptr = queue.enqueueMapBuffer(m_array_a, CL_TRUE, CL_MAP_READ, 0, m_nElements*sizeof(cl_uint));
		//m_queue.enqueueReadBuffer(m_array_a, CL_TRUE, 0, m_nElements*sizeof(cl_uint), a);

		m_totalTime = readTimer();

		// clean-up
		delete m_array_gcount;
		delete m_array_psum;
		m_array_gcount = m_array_psum = 0;
		m_devCon = 0;
	}


	void RadixSort::runSingle(DeviceArray<cl_uint> &bufferSrc, DeviceArray<cl_uint> &bufferTgt, cl_uint shift)
	{
		// set kernel arguments
		m_kernelCounting         .setArg<cl::Buffer>(0, bufferSrc);
		m_kernelCounting         .setArg<cl_uint>   (2, shift);
		m_kernelPermute          .setArg<cl::Buffer>(0, bufferSrc);
		m_kernelPermute          .setArg<cl::Buffer>(1, bufferTgt);
		m_kernelPermute          .setArg<cl_uint>   (3, shift);
		m_kernelPrescanSum       .setArg<cl::Buffer>(1, bufferTgt);
		m_kernelPrescanWithOffset.setArg<cl::Buffer>(1, bufferTgt);

		if(m_numPrescanGroups > 256) {
			m_kernelPrescanUpSweep  .setArg<cl::Buffer>(0, bufferTgt);
			m_kernelPrescanDownSweep.setArg<cl::Buffer>(0, bufferTgt);

		} else {
			m_kernelPrescan.setArg<cl::Buffer>(0, bufferTgt);
		}
	
		// events for profiling	
		cl::Event evKernelCounting;
		cl::Event evKernelPrescanSum;
		cl::Event evKernelPrescanUpSweep;
		cl::Event evKernelPrescan;
		cl::Event evKernelPrescanDownSweep;
		cl::Event evKernelPrescanWithOffset;
		cl::Event evKernelPermute;
	
		// enqueue kernels
		m_devCon->enqueue1DRangeKernel(m_kernelCounting,   m_numGroups*LOCAL_WORK, LOCAL_WORK, 0, &evKernelCounting);
		m_devCon->enqueue1DRangeKernel(m_kernelPrescanSum, m_numPrescanGroups,              0, 0, &evKernelPrescanSum);

		if(m_numPrescanGroups > 256) {
			m_devCon->enqueue1DRangeKernel(m_kernelPrescanUpSweep,   m_numPrescanGroups/4, LOCAL_WORK, 0, &evKernelPrescanUpSweep);
			m_devCon->enqueue1DRangeKernel(m_kernelPrescan,          LOCAL_WORK,           LOCAL_WORK, 0, &evKernelPrescan);
			m_devCon->enqueue1DRangeKernel(m_kernelPrescanDownSweep, m_numPrescanGroups/4, LOCAL_WORK, 0, &evKernelPrescanDownSweep);

		} else
			m_devCon->enqueue1DRangeKernel(m_kernelPrescan, LOCAL_WORK, LOCAL_WORK, 0, &evKernelPrescan);

		m_devCon->enqueue1DRangeKernel(m_kernelPrescanWithOffset, m_numPrescanGroups,              0, 0, &evKernelPrescanWithOffset);
		m_devCon->enqueue1DRangeKernel(m_kernelPermute,           m_numGroups*LOCAL_WORK, LOCAL_WORK, 0, &evKernelPermute);

		// retrieve kernel runtimes
		m_devCon->finish();

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
	//	m_queue.enqueueReadBuffer(m_array_gcount, CL_TRUE, 0, m_nElements*sizeof(cl_uint), gcount);
	//#else
	//	m_queue.enqueueReadBuffer(m_array_gcount, CL_TRUE, 0, m_numGroups*BASE*sizeof(cl_uint), gcount);
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

}
