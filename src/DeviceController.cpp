
#include <tbt/DeviceController.h>
#include <tbt/Global.h>
#include <tbt/Utility.h>

using namespace std;


namespace tbt
{

	const char *DeviceController::s_strExtension[TBT_NUM_EXTENSION_STRINGS] = {
		"cl_khr_fp64",
		"cl_amd_fp64",
		"cl_khr_fp16 ",
		"cl_intel_printf",
		"cl_amd_printf",
		"cl_khr_int64_base_atomics",
		"cl_khr_int64_extended_atomics",
		"cl_khr_byte_addressable_store"
	};

	cl_uint DeviceController::s_valExtension[TBT_NUM_EXTENSION_STRINGS] = {
		TBT_EXT_FP64,
		TBT_EXT_FP64,
		TBT_EXT_FP16,
		TBT_EXT_PRINTF,
		TBT_EXT_PRINTF,
		TBT_EXT_INT64_BASE_ATOMICS,
		TBT_EXT_INT64_EXTENDED_ATOMICS,
		TBT_EXT_BYTE_ADDRESSABLE_STORE
	};

	const char *DeviceController::s_defineExtension[TBT_NUM_EXT] = {
		"TBT_EXT_FP64",
		"TBT_EXT_FP16",
		"TBT_EXT_PRINTF",
		"TBT_EXT_INT64_BASE_ATOMICS",
		"TBT_EXT_INT64_EXTENDED_ATOMICS",
		"TBT_EXT_BYTE_ADDRESSABLE_STORE"
	};


	DeviceController::DeviceController(cl::Device device, cl::Context context, cl_command_queue_properties properties)
		: m_device(device), m_context(context)
	{
		m_queue = cl::CommandQueue(m_context, m_device, properties);

		device.getInfo(CL_DEVICE_TYPE,                      &m_deviceType);
		device.getInfo(CL_DEVICE_MAX_COMPUTE_UNITS,         &m_maxComputeUnits);
		device.getInfo(CL_DEVICE_MAX_WORK_GROUP_SIZE,       &m_maxWorkGroupSize);
		device.getInfo(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,  &m_maxWorkItemDims);
		device.getInfo(CL_DEVICE_LOCAL_MEM_SIZE,            &m_localMemSize);
		device.getInfo(CL_DEVICE_LOCAL_MEM_TYPE,            &m_localMemType);
		device.getInfo(CL_DEVICE_GLOBAL_MEM_SIZE,           &m_globalMemSize);
		device.getInfo(CL_DEVICE_GLOBAL_MEM_CACHE_SIZE,     &m_globalMemCacheSize);
		device.getInfo(CL_DEVICE_GLOBAL_MEM_CACHE_TYPE,     &m_globalMemCacheType);
		device.getInfo(CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, &m_globalMemCachelineSize);
		device.getInfo(CL_DEVICE_MAX_MEM_ALLOC_SIZE,        &m_maxMemAllocSize);
		device.getInfo(CL_DEVICE_HOST_UNIFIED_MEMORY,       &m_hostUnifiedMemory);
		device.getInfo(CL_DEVICE_MEM_BASE_ADDR_ALIGN,       &m_memBaseAddrAlign);
		device.getInfo(CL_DEVICE_QUEUE_PROPERTIES,          &m_scqProperties);

		std::string extensions;
		device.getInfo(CL_DEVICE_EXTENSIONS, &extensions);

		m_supportedExtensions = 0;
		for(int i = 0; i < TBT_NUM_EXT; i++)
			m_extString[i] = -1;

		for(int i = 0; i < TBT_NUM_EXTENSION_STRINGS; i++) {
			if(extensions.find(s_strExtension[i]) != std::string::npos) {
				cl_uint val = s_valExtension[i];
				m_extString[Utility::firstBit(val)] = i;
				m_supportedExtensions |= val;
			}
		}
	}


	const std::string DeviceController::createOpenCLHeader(cl_uint requiredExt, cl_uint optionalExt) const
	{
		if((requiredExt & m_supportedExtensions) != requiredExt)
			throw Error("Not all required extensions supported by device", Error::ecExtensionNotSupported);

		cl_uint allExt = requiredExt | optionalExt;
		std::string openclHeader;
		char buffer[256];

		cl_uint ext = 1;
		for(int i = 0; i < TBT_NUM_EXT; i++, ext <<= 1) {
			if(allExt & ext) {
				int j = m_extString[i];
				if(j != -1) {
					sprintf_s(buffer, 256, "#pragma OPENCL EXTENSION %s : enable\n#define %s\n", s_strExtension[j], s_defineExtension[i]);
					openclHeader += buffer;
				}
			}
		}

		return openclHeader;
	}


	std::string printDeviceType(cl_device_type type)
	{
		std::string str;
		bool printOr = false;

		if(type & CL_DEVICE_TYPE_CPU) {
			if(printOr) str += ", ";
			str += "CPU";
			printOr = true;
		}
		if(type & CL_DEVICE_TYPE_GPU) {
			if(printOr) str += ", ";
			str += "GPU";
			printOr = true;
		}
		if(type & CL_DEVICE_TYPE_ACCELERATOR) {
			if(printOr) str += ", ";
			str += "ACCELERATOR";
			printOr = true;
		}
		if(type & CL_DEVICE_TYPE_DEFAULT) {
			if(printOr) str += ", ";
			str += "DEFAULT";
			printOr = true;
		}

		return str;
	}

	std::string printCacheType(cl_device_mem_cache_type type)
	{
		switch(type) {
		case CL_NONE: return "NONE";
		case CL_READ_ONLY_CACHE: return "READ_ONLY_CACHE";
		case CL_READ_WRITE_CACHE: return "READ_WRITE_CACHE";
		default:
			return "unknonw";
		}
	}

	std::string printLocalMemType(cl_device_local_mem_type type)
	{
		switch(type) {
		case CL_LOCAL: return "LOCAL";
		case CL_GLOBAL: return "GLOBAL";
		default:
			return "unknonw";
		}
	}

	std::string printQueueProperties(cl_command_queue_properties props)
	{
		std::string str;
		bool printOr = false;

		if(props & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE) {
			if(printOr) str += ", ";
			str += "OUT_OF_ORDER_EXEC_MODE_ENABLE";
			printOr = true;
		}
		if(props & CL_QUEUE_PROFILING_ENABLE) {
			if(printOr) str += ", ";
			str += "PROFILING_ENABLE";
			printOr = true;
		}

		return str;
	}

	std::string printExecCapabilities(cl_device_exec_capabilities caps)
	{
		std::string str;
		bool printOr = false;

		if(caps & CL_EXEC_KERNEL) {
			if(printOr) str += ", ";
			str += "EXEC_KERNEL";
			printOr = true;
		}
		if(caps & CL_EXEC_NATIVE_KERNEL) {
			if(printOr) str += ", ";
			str += "EXEC_NATIVE_KERNEL";
			printOr = true;
		}

		return str;
	}

	
	std::ostream &DeviceController::displayInfo(std::ostream &os)
	{
		os << "    name:   " << getName() << endl;
		os << "    vendor: " << getVendor() << ", ID = " << getVendorID() << endl;
		os << "    type:   " << printDeviceType(getType()) << endl;
		os << endl;

		os << "    driver version:   " << getDriverVersion() << endl;
		os << "    OpenCL version:   " << getVersion() << endl;
		os << "    OpenCL C version: " << getOpenCLCVersion() << endl;
		os << "    profile:          " << getProfile() << endl;
		os << endl;
		os << "    execution capabilities: " << printExecCapabilities(getExecutionCapabilities()) << endl;
		os << "    queue properties:       " << printQueueProperties(getSupportedCommandQueueProperties()) << endl;
		os << "                   enabled: " << printQueueProperties(getCommandQueueProperties()) << endl;
		os << endl;

		os << "    compute units:          " << getMaxComputeUnits() << " (max. " << getMaxClockFrequency() << " MHz, ";
		if(isLitleEndian() == CL_TRUE) os << "little endian";  else os << "big endian";
		os << ")" << endl;

		os << "    local memory:           " << Utility::printBytes(getLocalMemSize()) << " (" << printLocalMemType(getLocalMemType()) << ")" << endl;

		os << "    global memory:          " << Utility::printBytes(getGlobalMemSize());
		if(getHostUnifiedMemory() == CL_TRUE) os << " (host unified)";
		os << endl;
		os << "        cache:              ";
		if(getGlobalMemCacheType() == CL_NONE) os << "NONE" << endl;
		else
			os << Utility::printBytes(getGlobalMemCacheSize()) << " (" << printCacheType(getGlobalMemCacheType()) <<
				", " << getGlobalMemCachelineSize() << " Bytes cache line)" << endl;

		os << "    max memory allocation:  " << Utility::printBytes(getMaxMemAllocSize()) << endl;
		os << "    base addr align:        " << getMemBaseAddrAlign() << " Bytes" << endl;

		os << endl;
		os << "    max work-group size:    " << getMaxWorkGroupSize() << endl;
		os << "    max work-item dims:     " << getMaxWorkItemDimensions() << endl;

		return os;
	}


	cl_command_queue_properties DeviceController::getCommandQueueProperties() const
	{
		cl_command_queue_properties props;
		m_queue.getInfo(CL_QUEUE_PROPERTIES, &props);
		return props;
	}


	std::string DeviceController::getName() const
	{
		std::string name;
		m_device.getInfo(CL_DEVICE_NAME, &name);
		return name;
	}


	std::string DeviceController::getVendor() const
	{
		std::string vendor;
		m_device.getInfo(CL_DEVICE_VENDOR, &vendor);
		return vendor;
	}


	std::string DeviceController::getVersion() const
	{
		std::string version;
		m_device.getInfo(CL_DEVICE_VERSION, &version);
		return version;
	}

	
	std::string DeviceController::getOpenCLCVersion() const
	{
		std::string version;
		m_device.getInfo(CL_DEVICE_OPENCL_C_VERSION, &version);
		return version;
	}

	
	std::string DeviceController::getProfile() const
	{
		std::string profile;
		m_device.getInfo(CL_DEVICE_PROFILE, &profile);
		return profile;
	}

	
	std::string DeviceController::getDriverVersion() const
	{
		std::string driverVersion;
		m_device.getInfo(CL_DRIVER_VERSION, &driverVersion);
		return driverVersion;
	}


	cl_uint DeviceController::getVendorID() const
	{
		cl_uint id;
		m_device.getInfo(CL_DEVICE_VENDOR_ID, &id);
		return id;
	}


	cl_uint DeviceController::getMaxClockFrequency() const
	{
		cl_uint freq;
		m_device.getInfo(CL_DEVICE_MAX_CLOCK_FREQUENCY, &freq);
		return freq;
	}


	cl_bool DeviceController::isLitleEndian() const
	{
		cl_bool littleEndian;
		m_device.getInfo(CL_DEVICE_ENDIAN_LITTLE, &littleEndian);
		return littleEndian;
	}


	size_t DeviceController::getWGSizeMultiple1D(const cl::Kernel &kernel)
	{
		cl::size_t<3> wfs;
		kernel.getWorkGroupInfo(m_device, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, &wfs);
		return wfs[0];
	}


	void DeviceController::enqueue1DRangeKernel(
		const cl::Kernel &kernel,
		size_t globalWork,
		size_t localWork,
		const cl::vector<cl::Event> *events,
		cl::Event *ev)
	{
		cl::NDRange localWorkRange = (localWork > 0) ? cl::NDRange(localWork) : cl::NullRange;
		m_queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(globalWork), localWorkRange, events, ev);
	}


	void GlobalDeviceControllers::init(const cl::Context &context, cl_command_queue_properties properties)
	{
		cl::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

		for(cl_uint i = 0; i < devices.size(); ++i)
			m_devCons.push_back(new DeviceController(devices[i], context, properties));
	}


	GlobalDeviceControllers::~GlobalDeviceControllers()
	{
		for(cl_uint i = 0; i < m_devCons.size(); ++i)
			delete m_devCons[i];
	}

}
