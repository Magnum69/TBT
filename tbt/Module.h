
#ifndef _TBT_MODULE_H
#define _TBT_MODULE_H

#include "tbthc.h"


namespace tbt
{
	//! Base class for OpenCL modules.
	class Module
	{
		static cl::Program s_program;  //!< the OpenCL program of the module.
		LARGE_INTEGER m_timer;         //!< stores high-performance counter.


	public:
		//! Constructs a module.
		Module() { }

		//! Builds the program from sources \a progName, which are relative to path of executable.
		/**
		 * This function reads the OpenCL sources from file \a progName and builds the program.
		 * It also performs automatic caching of program binaries (if enabled in global options)
		 * and reads the cached program if available (instead of compiling the sources).
		 *
		 * @param progName  file name of the OpenCL program; this file name is realtive to the path
		 *                  to the currently running executable.
		 *
		 * @see Global for configuring program caching options.
		 */
		static void buildProgramFromSourceRel(const char *progName);

		//! Returns this module's program.
		static cl::Program getProgram() { return s_program; }

		//! Returns true if the current program has been loaded.
		static bool isProgramLoaded()  { return s_program() != NULL; }

		//! Creates a kernel \a kernelName from this module's program.
		static cl::Kernel createKernel(const char *kernelName) {
			return cl::Kernel(s_program, kernelName);
		}

		//! Creates a buffer for \a nElements elements of type \a T.
		/**
		 * @param nElements  number of elements in the buffer.
		 * @param flags      a bit-field that is used to specify allocation and usage information such as the memory arena that should be used to allocate the buffer object and how it will be used.
		 *                   Possible values: CL_MEM_READ_WRITE, CL_MEM_WRITE_ONLY, CL_MEM_READ_ONLY, CL_MEM_USE_HOST_PTR, CL_MEM_ALLOC_HOST_PTR, CL_MEM_COPY_HOST_PTR
		 * @param hostPtr    a pointer to the buffer data that may already be allocated by the application. The size of the buffer that \a hostPtr points to must be greater than or equal to \a nElements * sizeof(\a T).
		 */
		template<class T>
		cl::Buffer createArrayBuffer(
			size_t nElements,
			cl_mem_flags flags = CL_MEM_READ_WRITE,
			void *hostPtr = 0)
		{
			return cl::Buffer(m_context, flags, n*sizeof(T), hostPtr);
		}

		//! Enqueues kernel with a 1D-range to command queue.
		/**
		 * @param queue       the command queue in which the kernel is enqueued.
		 * @param kernel      the kernel to be enqueued.
		 * @param globalWork  the number of global work-items.
		 * @param localWork   the number of work items that make up a work group; if set to 0, the OpenCL implementation will decide how to break the global work-items.
		 * @param events      specifies events that need to complete before this command can be executed. If events is 0, then this command does not wait on any event to complete.
		 * @param ev          returns an event object that identifies this particular kernel execution instance. 
		 */
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

		//! Returns how long an event took to execute (difference between event end and event start) in milliseconds.
		static double getEventTime(cl::Event ev);

		//! Start timer (for easy measuring of runtime).
		void startTimer();

		//! Read current elapsed time (from startTimer() until now).
		double readTimer();
	};

}


#endif
