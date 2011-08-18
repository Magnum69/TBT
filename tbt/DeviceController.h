
#ifndef _TBT_DEVICE_CONTROLLER_H
#define _TBT_DEVICE_CONTROLLER_H

#include <tbt/tbthc.h>


namespace tbt
{

#define TBT_NUM_EXTENSION_STRINGS 6

	//! Device controller encapsulating a device with a command queue.
	/**
	 * \ingroup context
	 *
	 * \section OpenCL Extensions
	 *
	 * TBT allows to automatically set some OpenCL extensions whithout the need to put the corresponding
	 * pragmas in the source code. Depending on the OpenCL paltform, some extensions may have different names.
	 * e.g., the extension for 64-bit floating point support is called <tt>cl_khr_fp64</tt> for Intel CPUs,
	 * but cl_amd_fp64 for AMD graphics cards. TBT chooses the correct name as supported by the device.
	 *
	 * A TBT Module can specify required and optional extensions. If a required extensions is not supported by a
	 * device, an exception will be thrown. In order to query if an optional extension could be enabled, TBT
	 * defines a macro in OpenCL programs that allows to use conditional compilation for providing different
	 * code depending on the availability of the extension.
	 *
	 * TBT specifies extensions as a bitvector, where each bit represents an extension. For each extensions,
	 * a macro is defined which sets the corresponding bit; these macros can be combined with or (<tt>|</tt>)
	 * to make up the bitvector. The following table lists the available extensions and predefined macros.
	 *
	 * <table>
	 * <tr><th>Extension</th><th>Description</th><th>Define in OpenCL program</th><th>OpenCL Extensions</th></tr>
	 * <tr>
	 *   <td><tt>TBT_EXT_FP64</tt></td>
	 *   <td>Enable support for double precision floating-point.</td>
	 *   <td><tt>TBT_EXT_FP64</tt></td>
	 *   <td><tt>cl_khr_fp64</tt><br><tt>cl_amd_fp64</tt></td>
	 * </tr><tr>
	 *   <td><tt>TBT_EXT_PRINTF</tt></td>
	 *   <td>Enables support for printf-function in OpenCL kernels.</td>
	 *   <td><tt>TBT_EXT_PRINTF</tt></td>
	 *   <td><tt>cl_intel_printf</tt><br><tt>cl_amd_printf</tt></td>
	 * </tr><tr>
	 *   <td><tt>TBT_EXT_INT64_BASE_ATOMICS</tt></td>
	 *   <td>Enables support for basic atomic operations (<tt>atom_add</tt>, <tt>atom_sub</tt>,
	 *       <tt>atom_xchg</tt>, <tt>atom_inc</tt>, <tt>atom_dec</tt>, <tt>atom_cmpxchg</tt>)
	 *       on 64-bit signed and unsigned integers stored in local or global memory.</td>
	 *   <td><tt>TBT_EXT_INT64_BASE_ATOMICS</tt></td>
	 *   <td><tt>cl_khr_int64_base_atomics</tt></td>
	 * </tr>
	 * <tr>
	 *   <td><tt>TBT_EXT_INT64_EXTENDED_ATOMICS</tt></td>
	 *   <td>Enables support for extended atomic operations (<tt>atom_min</tt>, <tt>atom_max</tt>,
	 *       <tt>atom_and</tt>, <tt>atom_or</tt>, <tt>atom_xor</tt>)
	 *       on 64-bit signed and unsigned integers stored in local or global memory.</td>
	 *   <td><tt>TBT_EXT_INT64_EXTENDED_ATOMICS</tt></td>
	 *   <td><tt>cl_khr_int64_extended_atomics</tt></td>
	 * </tr>
	 * </table>
	 */
	class DeviceController
	{
		cl::Device       m_device;  //!< the associated device.
		cl::Context      m_context; //!< the associated context.
		cl::CommandQueue m_queue;   //!< the command queue for device in context.

		cl_device_type m_deviceType;        //!< the type of the associated device.
		cl_uint        m_maxComputeUnits;   //!< the number of parallel compute units on the associated device.
		size_t         m_maxWorkGroupSize;  //!< the maximum number of work-items in a work-group on the associated device.
		cl_ulong       m_localMemSize;      //!< the size of the local memory arena on the associated device.
		cl_bool        m_hostUnifiedMemory; //!< has the device and the host a unified memory subsystem?
		cl_uint        m_memBaseAddrAlign;  //!< the minimum alignment of memory base addresses.
		cl_command_queue_properties m_scqProperties; //!< the supported command-queue properties supported by the associated device.
		cl_uint m_supportedExtensions;

		int m_extString[TBT_NUM_EXT];

		static const char *s_strExtension[TBT_NUM_EXTENSION_STRINGS];
		static cl_uint     s_valExtension[TBT_NUM_EXTENSION_STRINGS];
		static const char *s_defineExtension[TBT_NUM_EXT];

	public:
		/** @name Constructor
		 *  Device controllers are usually constructed by the class Global when creating a global context,
		 * so it is not necessary to construct a device controller manually.
		 */
		//@{

		//! Constructs a device controller for \a device and \a context.
		/**
		 * @param[in] device      a valid OpenCL device in \a context that will be associated with this device controller.
		 * @param[in] context     a valid OpenCL context.
		 * @param[in] properties  properties set of the created command queue for \a device.
		 */
		DeviceController(cl::Device device, cl::Context context, cl_command_queue_properties properties = 0);

		//@}


		/** @name Underlying OpenCL Objects
		 *  These methods provide access to the underlying OpenCL objects, namely the associated OpenCL device and context,
		 *  and the created command queue.
		 */
		//@{

		//! Returns the associated OpenCL device.
		/**
		 * @return the device associated with this device controller.
		 */
		cl::Device getDevice() const { return m_device; }

		//! Returns the command queue.
		/**
		 * @return the command queue of this device controller.
		 */
		cl::CommandQueue getCommandQueue() { return m_queue; }

		//! Returns the associated OpenCL context.
		/**
		 * @return the context associated with this device controller.
		 */
		cl::Context getContext() const { return m_context; }

		//@}


		/** @name Device Information
		 *  These methods return information about the device associated with this device controller.
		 *  Some important information are already queried and cached when a device controller is constructed,
		 *  other information are queried at request.
		 */
		//@{

		//! Returns the type of the associated OpenCL device.
		/**
		 * @return the type (CL_DEVICE_TYPE) of the device associated with this device controller.
		 */
		cl_device_type getType() const { return m_deviceType; }

		//! Returns the name of the associated OpenCL device.
		/**
		 * @return the name (CL_DEVICE_NAME) of the device associated with this device controller.
		 */
		std::string getName() const;

		//! Returns the vendor name of the associated OpenCL device.
		/**
		 * @return the vendor name (CL_DEVICE_VENDOR) of the device associated with this device controller.
		 */
		std::string getVendor() const;

		//! Returns the vendor ID of the associated OpenCL device.
		/**
		 * @return the vendor ID (CL_DEVICE_VENDOR_ID) of the device associated with this device controller.
		 */
		cl_uint getVendorID() const;

		//! Returns the number of parallel compute units on the associated OpenCL device.
		/**
		 * @return the number of compute units (CL_DEVICE_MAX_COMPUTE_UNITS) of the device associated with this device controller.
		 */
		cl_uint getMaxComputeUnits() const { return m_maxComputeUnits; }

		//! Returns the maximum number of work-items in a work-group on the associated device.
		/**
		 * @return the maximum number of work-items in a work-group (CL_DEVICE_MAX_WORK_GROUP_SIZE)
		 *         of the device associated with this device controller.
		 */
		size_t getMaxWorkGroupSize() const { return m_maxWorkGroupSize; }

		//! Returns the size of the local memory arena on the associated device.
		/**
		 * @return the size of the local memory arena (CL_DEVICE_LOCAL_MEM_TYPE) on the device associated with this device controller.
		 */
		cl_ulong getLocalMemSize() const { return m_localMemSize; }
		
		//! Returns true if the host and the associated device have a unified memory subsystem.
		/**
		 * @return true if the host and the associated device have a unified memory subsystem
		 *         (CL_DEVICE_HOST_UNIFIED_MEMORY), false otherwise.
		 */
		cl_uint getHostUnifiedMemory() const { return m_hostUnifiedMemory; }

		//! Returns the minimum alignment of memory base addresses of the associated device.
		/**
		 * @return the minimum alignment of memory base addresses (CL_DEVICE_MEM_BASE_ADDR_ALIGN)
		 * of the device associated with this device controller.
		 */
		cl_uint getMemBaseAddrAlign() const { return m_memBaseAddrAlign; }

		//! Returns the supported command-queue properties of the associated device.
		/**
		 * @return the supported command-queue properties (CL_DEVICE_QUEUE_PROPERTIES)
		 *         of the device associated with this device controller. These may differ from the
		 *         properties of the command-queue of this device controller.
		 */
		cl_command_queue_properties getSupportedCommandQueueProperties() const { return m_scqProperties; }

		//! Returns the OpenCL version supported by the associated device.
		/**
		 * @return the supported OpenCL version (CL_DEVICE_VERSION) of the device associated
		 *         with this device controller.
		 */
		std::string getVersion() const;

		//! Returns the OpenCL software driver version of the associated device.
		/**
		 * @return the OpenCL software driver version (CL_DRIVER_VERSION) of the device associated
		 *         with this device controller.
		 */
		std::string getDriverVersion() const;

		//! Returns the OpenCL extensions supported by the associated device.
		/**
		 * @return a bitvector where each bit represents an extensions according to the definition by TBT;
		 *         currently supported are TBT_EXT_FP64, TBT_EXT_PRINTF, TBT_EXT_INT64_BASE_ATOMICS,
		 *         and TBT_EXT_INT64_EXTENDED_ATOMICS. The device may support further extensions not
		 *         yet supported by TBT.
		 */
		cl_uint getExtensions() const {
			return m_supportedExtensions;
		}

		//! Returns the header for OpenCL programs, which enables desired and supported OpenCL extensions.
		/**
		 * This header is used for building OpenCL programs for this device. It is automatically prepended
		 * to OpenCL programs and enables the OpenCL extensions as selected in \ref globalConfig if
		 * supported by the associated device.
		 */
		const std::string getOpenCLHeader(cl_uint requiredExt, cl_uint optionalExt) const;

		//@}


		/** @name Command Queue
		 *  These methods enqueue commands to the command queue for the device associated with this
		 *  device controller, or perform a flush or finish on this command queue.
		 */
		//@{

		//! Enqueues a command to execute a 1D-range kernel on this device.
		/**
		 * @param[in]     kernel      is a valid kernel object. The OpenCL context associated with kernel and
		 *                            this device must be the same.
		 * @param[in]     globalWork  is the number of global work-items in dimension 1
		 *                            (and 1 for dimensions 2 and 3).
		 * @param[in]     localWork   is the number of work-items that make-up a work-group in dimension 1
		 *                            (and 1 for dimensions 2 and 3). \a localWork can be 0 in which case the
		 *                            OpenCL implementation will determine how to break the global work-items
		 *                            into appropriate work-group instances.
		 * @param[in]     events      specify events that need to complete before this particular command can be
		 *                            executed. If \a events is 0 (the default), then this command does not
		 *                            wait on any event to complete.
		 * @param[in,out] ev          returns an event object that identifies this particular kernel execution
		 *                            instance. If event is 0 (the default), no event will be created for this kernel
		 *                            execution instance and therefore it will not be possible to query or queue a
		 *                            wait for this kernel execution instance.
		 */
		void enqueue1DRangeKernel(
			const cl::Kernel &kernel,
			size_t globalWork,
			size_t localWork = 0,
			const cl::vector<cl::Event> *events = 0,
			cl::Event *ev = 0);

		//! Enqueues a command to execute a kernel on this device.
		/**
		 * @param[in]     kernel      is a valid kernel object. The OpenCL context associated with kernel and
		 *                            this device must be the same.
		 * @param[in]     events      specify events that need to complete before this particular command can be
		 *                            executed. If \a events is 0 (the default), then this command does not
		 *                            wait on any event to complete.
		 * @param[in,out] ev          returns an event object that identifies this particular kernel execution
		 *                            instance. If event is 0 (the default), no event will be created for this kernel
		 *                            execution instance and therefore it will not be possible to query or queue a
		 *                            wait for this kernel execution instance.
		 */
		void enqueueTask(
			const cl::Kernel &kernel,
			const cl::vector<cl::Event> *events = 0,
			cl::Event *ev = 0)
		{
			m_queue.enqueueTask(kernel, events, ev);
		}

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
