
#include <tbt/Module.h>

#include <iostream>
#include <fstream>

using namespace std;


namespace tbt {


	cl::Program Module::s_program;


	double Module::getEventTime(cl::Event ev)
	{
		cl_ulong startTime = ev.getProfilingInfo<CL_PROFILING_COMMAND_START>();
		cl_ulong endTime   = ev.getProfilingInfo<CL_PROFILING_COMMAND_END>();
		return 1.e-6 * (endTime - startTime);
	}


	void Module::startTimer()
	{
		QueryPerformanceCounter(&m_timer);
	}


	double Module::readTimer()
	{
		LARGE_INTEGER curTimer, frequeny;
		QueryPerformanceCounter(&curTimer);
		QueryPerformanceFrequency(&frequeny);

		return 1e3 * double(curTimer.QuadPart - m_timer.QuadPart) / frequeny.QuadPart;
	}


	void Module::buildProgramFromSourceRel(const char *progName)
	{
		s_program = Utility::buildProgram(progName);
	}

}
