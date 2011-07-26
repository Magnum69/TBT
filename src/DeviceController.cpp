
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
	}

}
