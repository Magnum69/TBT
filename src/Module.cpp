
#include <tbt/Module.h>

#include <iostream>
#include <fstream>

using namespace std;


namespace tbt {


	double Module::getEventTime(cl::Event ev)
	{
		cl_ulong startTime = ev.getProfilingInfo<CL_PROFILING_COMMAND_START>();
		cl_ulong endTime   = ev.getProfilingInfo<CL_PROFILING_COMMAND_END>();
		return 1.e-6 * (endTime - startTime);
	}


	void Module::buildProgramFromSourceRel(const char *progName)
	{
		m_program = Utility::buildProgram(progName);
	}

}
