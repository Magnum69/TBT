
#ifndef _TBT_DEVICE_CONTROLLER_H
#define _TBT_DEVICE_CONTROLLER_H

#include <tbt/tbthc.h>


namespace tbt
{
	//! Device controller encapsulating a device with a command queue.
	class DeviceController
	{
		cl::Device       m_device;  //!< the associated device.
		cl::Context      m_context; //!< the associated context.
		cl::CommandQueue m_queue;   //!< the command queue for device in context.

		cl_device_type m_deviceType;       //!< the type of the associated device.
		cl_uint        m_maxComputeUnits;  //!< the number of parallel compute units on the associated device.
		size_t         m_maxWorkGroupSize; //!< the maximum number of work-items in a work-group on the associated device.
		cl_ulong       m_localMemSize;     //!< the size of the local memory arena on the associated device.

	public:
		//! Constructs a device controller for \a device and \a context.
		/**
		 * @param device      a valid OpenCL device in \a context that will be associated with this device controller.
		 * @param context     a valid OpenCL context.
		 * @param properties  properties set of the created command queue for \a device.
		 */
		DeviceController(cl::Device device, cl::Context context, cl_command_queue_properties properties = 0);

		//! Returns the associated OpenCL device.
		cl::Device getDevice() { return m_device; }

		//! Returns the command queue.
		cl::CommandQueue getCommandQueue() { return m_queue; }

		//! Returns the associated OpenCL context.
		cl::Context getContext() { return m_context; }

		//! Returns the type of the associated OpenCL device.
		cl_device_type getType() const { return m_deviceType; }

		//! Returns the number of parallel compute units on the associated OpenCL device.
		cl_uint getMaxComputeUnits() const { return m_maxComputeUnits; }

		//! Returns the maximum number of work-items in a work-group on the associated device.
		size_t getMaxWorkGroupSize() const { return m_maxWorkGroupSize; }

		//! Returns the size of the local memory arena on the associated device.
		cl_ulong getLocalMemSize() const { return m_localMemSize; }
	};


	class GlobalDeviceControllers
	{
		cl::vector<DeviceController*> m_devCons;

	public:
		GlobalDeviceControllers() { }
		~GlobalDeviceControllers();

		void init(const cl::Context &context, cl_command_queue_properties properties = 0);

		int numDevices() const { return (int)m_devCons.size(); }

		DeviceController *operator[](int i) { return m_devCons[i]; }
	};

}

#endif
