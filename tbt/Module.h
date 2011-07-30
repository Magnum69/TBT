
#ifndef _TBT_MODULE_H
#define _TBT_MODULE_H

#include "Global.h"


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

		//! Returns how long an event took to execute (difference between event end and event start) in milliseconds.
		static double getEventTime(cl::Event ev);

		//! Start timer (for easy measuring of runtime).
		void startTimer();

		//! Read current elapsed time (from startTimer() until now).
		double readTimer();
	};

}


#endif
