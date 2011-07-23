
#ifndef _TBT_MODULE_H
#define _TBT_MODULE_H

#include "tbthc.h"


namespace tbt
{
	//! Base class for OpenCL modules.
	class Module
	{
		cl::Program      m_program;

	public:
		Module() { }

		void buildProgramFromSourceRel(const char *progName);

		cl::Program      getProgram()      { return m_program; }


		cl::Kernel createKernel(const char *kernelName) {
			return cl::Kernel(m_program, kernelName);
		}

		template<class T>
		cl::Buffer createArrayBuffer(
			size_t nElements,
			cl_mem_flags flags = CL_MEM_READ_WRITE,
			void *hostPtr = 0)
		{
			return cl::Buffer(m_context, flags, n*sizeof(T), hostPtr);
		}

		void enqueue1DRangeKernel(
			cl::CommandQueue queue,
			const cl::Kernel &kernel,
			size_t globalWork,
			size_t localWork = 0,
			const cl::vector<cl::Event> *events = 0,
			cl::Event *ev = 0)
		{
			cl::NDRange localWorkRange = (localWork > 0) ? cl::NDRange(localWork) : cl::NullRange;
			queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(globalWork), localWorkRange, events, ev);
		}

		static double getEventTime(cl::Event ev);
	};

}


#endif
