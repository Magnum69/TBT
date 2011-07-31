
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
		cl_uint        m_memBaseAddrAlign;

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


		/** @name Device information methods
		 *  These methods return information about the device associated with this device controller.
		 */
		//@{

		//! Returns the type of the associated OpenCL device.
		cl_device_type getType() const { return m_deviceType; }

		//! Returns the name of the associated OpenCL device.
		std::string getName() const;

		//! Returns the number of parallel compute units on the associated OpenCL device.
		cl_uint getMaxComputeUnits() const { return m_maxComputeUnits; }

		//! Returns the maximum number of work-items in a work-group on the associated device.
		size_t getMaxWorkGroupSize() const { return m_maxWorkGroupSize; }

		//! Returns the size of the local memory arena on the associated device.
		cl_ulong getLocalMemSize() const { return m_localMemSize; }
		
		cl_uint getMemBaseAddrAlign() const { return m_memBaseAddrAlign; }

		//@}


		/** @name Command queue methods
		 *  These methods enqueue commands to the command queue for the device associated with this
		 *  device controller, or perform a flush or finish on this command queue.
		 */
		//@{

		//! Enqueues a command to execute a 1D-range kernel on this device.
		/**
		 * @param kernel      is a valid kernel object. The OpenCL context associated with kernel and
		 *                    this device must be the same.
		 * @param globalWork  is the number of global work-items in dimension 1
		 *                    (and 1 for dimensions 2 and 3).
		 * @param localWork   is the number of work-items that make-up a work-group in dimension 1
		 *                    (and 1 for dimensions 2 and 3). \a localWork can be 0 in which case the
		 *                    OpenCL implementation will determine how to break the global work-items
		 *                    into appropriate work-group instances.
		 * @param events      specify events that need to complete before this particular command can be
		 *                    executed. If \a events is 0 (the default), then this command does not
		 *                    wait on any event to complete.
		 * @param ev          returns an event object that identifies this particular kernel execution
		 *                    instance. If event is 0 (the default), no event will be created for this kernel
		 *                    execution instance and therefore it will not be possible to query or queue a
		 *                    wait for this kernel execution instance.
		 */
		void enqueue1DRangeKernel(
			const cl::Kernel &kernel,
			size_t globalWork,
			size_t localWork = 0,
			const cl::vector<cl::Event> *events = 0,
			cl::Event *ev = 0);

		//!	Issues all previously queued commands to the device.
		void flush() { m_queue.flush(); }

		//! Blocks until all previously queued commands are issued to the device and have completed.
		void finish() { m_queue.finish(); }

		//@}
	};


	//! Encapsulates the globally available device controllers.
	/**
	 * It is not necessary to manually create instances of this classes. The global device
	 * controllers are available through globalConfig.
	 *
	 * \see class Global, global configuration variable globalConfig.
	 */
	class GlobalDeviceControllers
	{
		cl::vector<DeviceController*> m_devCons;  //!< Maintains the device controllers.

	public:
		//! Constructs an instance of GlobalDeviceControllers.
		/**
		 * Does not create or add any device controllers.
		 */
		GlobalDeviceControllers() { }

		//! Destructor. Releases all device controllers.
		~GlobalDeviceControllers();

		//! Initializes the global device cotrollers for \a context and (optional) \a properties for the created command queues.
		/**
		 * Creates device controllers for all devices in the context, using the passed properties
		 * for command queues.
		 *
		 * @param context     must be a valid OpenCL context.
		 * @param properties  must be a bit-field of command line properties; possible values are
		 *                    CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE and CL_QUEUE_PROFILING_ENABLE.
		 */
		void init(const cl::Context &context, cl_command_queue_properties properties = 0);

		//! Returns the number of device controllers.
		int numDevices() const { return (int)m_devCons.size(); }

		//! Returns the <i>i</i>-th device controller.
		DeviceController *operator[](int i) { return m_devCons[i]; }
	};

}

#endif
