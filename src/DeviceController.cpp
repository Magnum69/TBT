
#include <tbt/DeviceController.h>
#include <tbt/Global.h>
#include <tbt/Utility.h>


namespace tbt
{

	const char *DeviceController::s_strExtension[TBT_NUM_EXTENSION_STRINGS] = {
		"cl_khr_fp64",
		"cl_amd_fp64",
		"cl_intel_printf",
		"cl_amd_printf",
		"cl_khr_int64_base_atomics",
		"cl_khr_int64_extended_atomics"
	};

	cl_uint DeviceController::s_valExtension[TBT_NUM_EXTENSION_STRINGS] = {
		TBT_EXT_FP64,
		TBT_EXT_FP64,
		TBT_EXT_PRINTF,
		TBT_EXT_PRINTF,
		TBT_EXT_INT64_BASE_ATOMICS,
		TBT_EXT_INT64_EXTENDED_ATOMICS
	};

	const char *DeviceController::s_defineExtension[TBT_NUM_EXT] = {
		"TBT_EXT_FP64",
		"TBT_EXT_PRINTF",
		"TBT_EXT_INT64_BASE_ATOMICS",
		"TBT_EXT_INT64_EXTENDED_ATOMICS"
	};


	DeviceController::DeviceController(cl::Device device, cl::Context context, cl_command_queue_properties properties)
		: m_device(device), m_context(context)
	{
		m_queue = cl::CommandQueue(m_context, m_device, properties);

		device.getInfo(CL_DEVICE_TYPE,                &m_deviceType);
		device.getInfo(CL_DEVICE_MAX_COMPUTE_UNITS,   &m_maxComputeUnits);
		device.getInfo(CL_DEVICE_MAX_WORK_GROUP_SIZE, &m_maxWorkGroupSize);
		device.getInfo(CL_DEVICE_LOCAL_MEM_SIZE,      &m_localMemSize);
		device.getInfo(CL_DEVICE_HOST_UNIFIED_MEMORY, &m_hostUnifiedMemory);
		device.getInfo(CL_DEVICE_MEM_BASE_ADDR_ALIGN, &m_memBaseAddrAlign);
		device.getInfo(CL_DEVICE_QUEUE_PROPERTIES,    &m_scqProperties);

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


	const std::string DeviceController::getOpenCLHeader(cl_uint requiredExt, cl_uint optionalExt) const
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
