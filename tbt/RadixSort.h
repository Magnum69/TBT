#ifndef _TBT_RADIX_SORT_H
#define _TBT_RADIX_SORT_H


#include <tbt/Module.h>


namespace tbt
{

	//! Radix-sort module.
	class RadixSort : public Module
	{
		static cl::Kernel  m_kernelCounting;
		static cl::Kernel  m_kernelPermute;

		static cl::Kernel  m_kernelPrescanSum;
		static cl::Kernel  m_kernelPrescan;
		static cl::Kernel  m_kernelPrescanWithOffset;

		static cl::Kernel m_kernelPrescanUpSweep;
		static cl::Kernel m_kernelPrescanDownSweep;

		cl_uint m_nElements;
		cl_uint m_numGroups;
		cl_uint m_numPrescanGroups;
		cl_uint m_prescanInterval;

		cl::Buffer m_buffer_a;
		cl::Buffer m_buffer_b;
		cl::Buffer m_buffer_gcount;
		cl::Buffer m_buffer_psum;

		double m_tKernelCounting;
		double m_tKernelPrescanSum;
		double m_tKernelPrescan;
		double m_tKernelPrescanWithOffset;
		double m_tKernelPermute;

		double m_totalTime;

	public:
		RadixSort() { }

		bool run(cl::CommandQueue queue, cl_uint *a, cl_uint n);

		double totalTimeKernelCounting         () const { return m_tKernelCounting; }
		double totalTimeKernelPrescanSum       () const { return m_tKernelPrescanSum;  }
		double totalTimeKernelPrescan          () const { return m_tKernelPrescan;  }
		double totalTimeKernelPrescanWithOffset() const { return m_tKernelPrescanWithOffset;  }
		double totalTimeKernelPermute          () const { return m_tKernelPermute;  }

		double totalTimeKernels() const {
			return m_tKernelCounting + m_tKernelPrescanSum + m_tKernelPrescan + m_tKernelPrescanWithOffset + m_tKernelPermute;
		}

		double totalTime() const { return m_totalTime; }


	private:
		void runSingle(cl::CommandQueue queue, cl::Buffer &bufferSrc, cl::Buffer &bufferTgt, cl_uint shift);

		static void assureKernelsLoaded();
	};

}

#endif
