
#include <tbt/Global.h>

using namespace std;


namespace tbt
{

	Global globalConfig;


	cl_device_type getType(const cl::Device &device)
	{
		cl_device_type deviceType;
		device.getInfo(CL_DEVICE_TYPE, &deviceType);
		return deviceType;
	}


	cl_uint getMaxComputeUnits(const cl::Device &device)
	{
		cl_uint nComputeUnits;
		device.getInfo(CL_DEVICE_MAX_COMPUTE_UNITS, &nComputeUnits);
		return nComputeUnits;
	}


	size_t getMaxWorkGroupSize(const cl::Device &device)
	{
		size_t maxWorkGroupSize;
		device.getInfo(CL_DEVICE_MAX_WORK_GROUP_SIZE, &maxWorkGroupSize);
		return maxWorkGroupSize;
	}


	cl_ulong getLocalMemSize(const cl::Device &device)
	{
		cl_ulong localMemSize;
		device.getInfo(CL_DEVICE_LOCAL_MEM_SIZE, &localMemSize);
		return localMemSize;
	}


	cl::Platform getStdPlatform(cl_device_type deviceType)
	{
		// Get a AMD/GPU and Intel/CPU platform and context.
		cl::vector< cl::Platform > platformList;
		cl::Platform::get(&platformList);

		int indexAMD = -1, indexIntel = -1, indexNvidia = -1;
		for(int i = 0; i < (int)platformList.size(); ++i) {
			string platformName;
			platformList[i].getInfo((cl_platform_info)CL_PLATFORM_NAME, &platformName);
			if(platformName == "AMD Accelerated Parallel Processing")
				indexAMD = i;
			else if(platformName == "Intel(R) OpenCL")
				indexIntel = i;
			else if(platformName == "NVIDIA CUDA")
				indexNvidia = i;
		}

		if((deviceType & (CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU)) == (CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU) && indexAMD == -1)
			throw Error("No OpenCL platform for CPUs and GPUs found!", Error::ecNoOpenCLPlatformFound);
		
		if(deviceType & CL_DEVICE_TYPE_GPU && indexAMD == -1 && indexNvidia == -1)
			throw Error("No OpenCL platform for GPUs found!", Error::ecNoOpenCLPlatformFound);
		
		if(deviceType & CL_DEVICE_TYPE_CPU && indexAMD == -1 && indexIntel == -1)
			throw Error("No OpenCL platform for CPUs found!", Error::ecNoOpenCLPlatformFound);

		if((deviceType & (CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU)) == (CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU))
			return platformList[indexAMD];
		else if(deviceType == CL_DEVICE_TYPE_GPU)
			return platformList[(indexAMD != -1) ? indexAMD : indexNvidia];
		else
			return (indexIntel != -1) ? platformList[indexIntel] : platformList[indexAMD];
	}


	ostream &displayPlatformInfo(ostream &os)
	{
		cl::Platform platform = getPlatform();

		string platformName;
		platform.getInfo((cl_platform_info)CL_PLATFORM_NAME,    &platformName);
		os << "    name:       " << platformName << endl;

		string platformVendor;
		platform.getInfo((cl_platform_info)CL_PLATFORM_VENDOR,  &platformVendor);
		os << "    vendor:     " << platformVendor << endl;

		string platformVersion;
		platform.getInfo((cl_platform_info)CL_PLATFORM_VERSION, &platformVersion);
		os << "    version:    " << platformVersion << endl;

		string platformProfile;
		platform.getInfo((cl_platform_info)CL_PLATFORM_PROFILE, &platformProfile);
		os << "    profile:    " << platformProfile << endl;

		string platformExtensions;
		platform.getInfo((cl_platform_info)CL_PLATFORM_EXTENSIONS, &platformExtensions);
		os << "    extensions: " << platformExtensions << endl;

		return os;
	}


	void Global::createContext(cl_device_type deviceType, const cl::Platform &platform, cl_command_queue_properties properties)
	{
		m_platform = platform;

		cl_context_properties cprops[3] = {
			CL_CONTEXT_PLATFORM,
			(cl_context_properties)(platform)(),
			0
		};

		globalConfig.m_context = cl::Context(deviceType, cprops);
		globalConfig.m_devCons.init(globalConfig.m_context, properties);

		for(int i = m_devCons.numDevices()-1; i >= 0; i--) {
			switch(m_devCons[i]->getType())
			{
			case CL_DEVICE_TYPE_CPU:
				m_cpuDeviceIndex = i;
			case CL_DEVICE_TYPE_GPU:
				m_gpuDeviceIndex = i;
			}
		}
	}


	void createContext(cl_device_type deviceType, cl_command_queue_properties properties)
	{
		globalConfig.createContext(deviceType, getStdPlatform(deviceType), properties);
	}

}
