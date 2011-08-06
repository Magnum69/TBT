
#ifndef _TBT_GLOBAL_H
#define _TBT_GLOBAL_H

#include <tbt/DeviceController.h>


namespace tbt
{

	//! Maintenance of global platform and context, as well as some options.
	class Global
	{
		cl::Platform m_platform;  //!< the global OpenCL platform we use.
		cl::Context m_context;    //!< the global OpenCL context we use.

		GlobalDeviceControllers m_devCons;  //!< controllers for all devices in the global context.
		int m_cpuDeviceIndex;  //!< the index of the first CPU device in m_devCons (if any, otherwise -1).
		int m_gpuDeviceIndex;  //!< the index of the first GPU device in m_devCons (if any, otherwise -1).

		bool m_cacheProgramBinaries;           //!< shall we cache program (kernel) binaries at all?
		bool m_recompileProgramsIfNewerDriver; //!< shall we check driver version and recompile programs if newer?

	public:
		//! Constructs a global configuration object.
		Global() {
			m_cpuDeviceIndex = m_gpuDeviceIndex = -1;
			m_cacheProgramBinaries = true;
			m_recompileProgramsIfNewerDriver = true;
		}

		//! Creates a context from a given platform and device type, and creates device controllers.
		void createContext(cl_device_type deviceType, const cl::Platform &platform, cl_command_queue_properties properties = 0);

		//! Returns current setting of option cacheProgramBinaries.
		bool getCacheProgramBinaries() const { return m_cacheProgramBinaries; }

		//! Sets option cacheProgramBinaries to \a b.
		void setCacheProgramBinaries(bool b) { m_cacheProgramBinaries = b; }

		//! Returns current setting of option recompileProgramsIfNewerDriver.
		bool getRecompileProgramsIfNewerDriver() const { return m_recompileProgramsIfNewerDriver; }

		//! Sets option recompileProgramsIfNewerDriver to \a b.
		void setRecompileProgramsIfNewerDriver(bool b) { m_recompileProgramsIfNewerDriver = b; }

		//! Returns global OpenCL platform.
		cl::Platform getPlatform() { return m_platform; }

		//! Returns global OpenCL context.
		cl::Context getContext() { return m_context; }

		//! Returns the first device controller (if any, otherwise 0 is returned).
		DeviceController *getDeviceController() { return (m_devCons.numDevices() > 0) ? m_devCons[0] : 0; }

		//! Returns a device controller for a CPU device (if any, otherwise 0 is returned).
		DeviceController *getCPUDeviceController() { return (m_cpuDeviceIndex >= 0) ? m_devCons[m_cpuDeviceIndex] : 0; }

		//! Returns a device controller for a GPU device (if any, otherwise 0 is returned).
		DeviceController *getGPUDeviceController() { return (m_gpuDeviceIndex >= 0) ? m_devCons[m_gpuDeviceIndex] : 0; }
	};


	/**
	 * \defgroup context Platform and context
	 * \brief These functions allow to select the global platform and context, and provide access to
	 *        platform, context, and device controllers.
	 */
	//@{

	//! The (one and only) global configuration object.
	extern Global globalConfig;

	//! Create global context from given platform and device type.
	/**
	 * @param deviceType  is the desired device type; possible values are CL_DEVICE_TYPE_CPU and CL_DEVICE_TYPE_GPU.
	 * @param platform    must be a valid OpenCL platform.
	 * @param properties  specifies a list of properties for the created command-queues. This is a bit-field; possible
	 *                    properties are CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE and CL_QUEUE_PROFILING_ENABLE.
	 */
	inline void createContext(cl_device_type deviceType, const cl::Platform &platform, cl_command_queue_properties properties = 0) {
		globalConfig.createContext(deviceType, platform, properties);
	}

	//! Create global context from given device type.
	/**
	 * @param deviceType  is the desired device type; possible values are CL_DEVICE_TYPE_CPU and CL_DEVICE_TYPE_GPU.
	 * @param properties  specifies a list of properties for the created command-queues. This is a bit-field; possible
	 *                    properties are CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE and CL_QUEUE_PROFILING_ENABLE.
	 */
	void createContext(cl_device_type deviceType, cl_command_queue_properties properties = 0);

	//! Returns global OpenCL platform.
	inline cl::Platform getPlatform() { return globalConfig.getPlatform(); }

	//! Returns global OpenCL context.
	inline cl::Context getContext() { return globalConfig.getContext(); }

	//! Display information about global OpenCL platform.
	/**
	 * @param os is the C++-output stream on which the information about the platform is written.
	 * @return the output stream \a os.
	 */
	std::ostream &displayPlatformInfo(std::ostream &os = std::cout);

	//! Returns the first global device controller (if any, otherwise 0 is returned).
	inline DeviceController *getDeviceController() { return globalConfig.getDeviceController(); }

	//! Returns a global device controller for a CPU device (if any, otherwise 0 is returned).
	inline DeviceController *getCPUDeviceController() { return globalConfig.getCPUDeviceController(); }

	//! Returns a global device controller for a GPU device (if any, otherwise 0 is returned).
	inline DeviceController *getGPUDeviceController() { return globalConfig.getGPUDeviceController(); }

	//@}

}

#endif
