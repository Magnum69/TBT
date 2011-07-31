
#include <tbt/DeviceController.h>

namespace tbt
{

	DeviceController::DeviceController(cl::Device device, cl::Context context, cl_command_queue_properties properties)
		: m_device(device), m_context(context)
	{
		m_queue = cl::CommandQueue(m_context, m_device, properties);

		device.getInfo(CL_DEVICE_TYPE,                &m_deviceType);
		device.getInfo(CL_DEVICE_MAX_COMPUTE_UNITS,   &m_maxComputeUnits);
		device.getInfo(CL_DEVICE_MAX_WORK_GROUP_SIZE, &m_maxWorkGroupSize);
		device.getInfo(CL_DEVICE_LOCAL_MEM_SIZE,      &m_localMemSize);
		device.getInfo(CL_DEVICE_MEM_BASE_ADDR_ALIGN, &m_memBaseAddrAlign);
	}


	std::string DeviceController::getName() const
	{
		std::string name;
		m_device.getInfo(CL_DEVICE_NAME, &name);
		return name;
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
