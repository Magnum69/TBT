
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


	Module::Module()
	{
		//m_queue.getInfo(CL_QUEUE_CONTEXT, &m_context);
		//m_queue.getInfo(CL_QUEUE_DEVICE,  &m_device);

		//cl_uint maxWorkItemDimensions;
		//m_device.getInfo(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, &maxWorkItemDimensions);

		//cl::vector<size_t> maxWorkItemSizes(maxWorkItemDimensions);
		//m_device.getInfo(CL_DEVICE_MAX_WORK_ITEM_SIZES, &maxWorkItemSizes);
		//m_maxWorkItemSize[0] = maxWorkItemSizes[0];
		//m_maxWorkItemSize[1] = maxWorkItemSizes[1];
		//m_maxWorkItemSize[2] = maxWorkItemSizes[2];
	}


	void Module::buildProgramFromSourceRel(const char *progName)
	{
		m_program = Utility::buildProgram(progName);

		//// get path to kernel source file
		//string kernelFullPath = getExePath();
		//kernelFullPath.append(progName);
		//ifstream file(kernelFullPath);
		//if(!file) {
		//	//cerr << "OclModule: Could not load kernel file: " << kernelFullPath << endl;
		//	std::string msg("OclModule: Could not read kernel file ");
		//	msg.append(kernelFullPath);
		//	throw OclException(msg.c_str(), OclException::excKernelFileNotFound);
		//}
	
		//// read source file
		//string progstr(istreambuf_iterator<char>(file), (istreambuf_iterator<char>()));
		//cl::Program::Sources source( 1, make_pair(progstr.c_str(), progstr.length()));

		//// build program
		//m_program = cl::Program(m_context, source);
		//cl::vector<cl::Device> devices(1, m_device);
		//try {
		//	m_program.build(devices);
		//} catch(cl::Error err) {
		//	cerr << "Compiler error: " << err.what() << "(" << err.err() << ")" << endl;
		//	string str =  m_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(m_device);
		//	//cout << "Program Info: " << str << endl;
		//	throw OclException("Could not compile kernels", OclException::excKernelCompileError);
		//}
	}

}
