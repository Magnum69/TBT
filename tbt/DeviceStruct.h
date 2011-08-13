
#ifndef _TBT_DEVICE_STRUCT_H
#define _TBT_DEVICE_STRUCT_H

#include <tbt/DeviceController.h>


namespace tbt
{

	//! Structure stored on an OpenCL device.
	/**
	 * \ingroup memobjects
	 */
	template<class T>
	class DeviceStruct
	{
	protected:
		cl::Buffer m_buffer;         //!< the allocated OpenCL buffer object.
		DeviceController *m_devCon;  //!< the associated device controller.

	public:
		/** @name Constructors
		 */
		//@{

		//! Constructs an invalid device structure.
		/**
		 * The constructed device structure is neither associated with an OpenCL buffer object nor a
		 * device controller. Hence, it cannot be used for loading or storing data.
		 */
		DeviceStruct() : m_devCon(0) { }

		//! Copy constructor. The constructed device structure will share the OpenCL buffer object.
		DeviceStruct(const DeviceStruct<T> &da) : m_buffer(da.m_buffer), m_devCon(da.m_devCon) { }

		//! Constructs a device structure of type \a T associated with device controller \a devCon.
		/**
		 * @param[in] devCon  must be a valid device controller that will be associated with the device structure.
		 * @param[in] flags   are the memory flags (restricting the access of kernels to the corresponding device memory)
		 *                    that will be used for creating the OpenCL buffer object; \a flags can be one of
		 *                    CL_MEM_READ_WRITE, CL_MEM_READ_ONLY and CL_MEM_WRITE_ONLY.
		 */
		DeviceStruct(DeviceController *devCon, cl_mem_flags flags = CL_MEM_READ_WRITE)
			: m_buffer( devCon->getContext(), flags & (CL_MEM_READ_WRITE | CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY), sizeof(T) ),
			  m_devCon(devCon)
		{ }

		//@}


		/** @name General Information
		 * These methods provide access to the associated device controller and the created OpenCL buffer object.
		 */
		//@{

		//! Returns the device controller associated with this device structure.
		/**
		 * @return the device controller associated with this device structure; will be 0 if the device strucutre is invalid.
		 */
		DeviceController *getDeviceController() const {
			return m_devCon;
		}

		//! Returns the corresponding OpenCL buffer object.
		/**
		 * @return the OpenCL buffer object of this device strucutre; will be invalid if the device array is invalid.
		 */
		cl::Buffer &getBuffer() { return m_buffer; }

		//! Converts a device strucutre to an OpenCL buffer object.
		/**
		 * @return the OpenCL buffer object of this device strucutre; will be invalid if the device array is invalid.
		 */
		operator cl::Buffer() { return m_buffer; }

		//@}


		/** @name Transfering Data between Host and Device Memory
		 *  These methods enqueue read- and write-buffer
		 *  commands to the command queue for the associated device. The suffix <strong>Blocking</strong>
		 *  indicates that the memory transfer is already completed once the method returns; the other methods
		 *  just enqueue a command for transfering the data (an event object can be used to wait for completion).
		 */
		//@{

		//! Loads \a x onto the device.
		/**
		 * This method enqueues a blocking write-buffer command. Once this method returns, the memory
		 * transfer to the device is completed.
		 *
		 * @param[in] x  is the value to be loaded onto the device.
		 */
		void loadBlocking(const T &x) {
			m_devCon->getCommandQueue().enqueueWriteBuffer(m_buffer, CL_TRUE, 0, sizeof(T), &x);
		}

		//! Enqueues a command for loading \a x onto the device.
		/**
		 * This method enqueues a non-blocking write-buffer command and returns an event object associated with
		 * this write command that can be queried (or waited for).
		 *
		 * @param[in]     x           is the value to be loaded onto the device.
		 * @param[in,out] eventLoad   if not 0, returns an event object that identifies the write command.
		 */
		void load(const T &x, cl::Event *eventLoad = 0) {
			m_devCon->getCommandQueue().enqueueWriteBuffer(m_buffer, CL_FALSE, 0, sizeof(T), &x, 0, eventLoad);
		}

		//! Stores the data on the device in \a x.
		/**
		 * This method enqueues a blocking read-buffer command. Once this method returns, the memory
		 * transfer to the host is completed.
		 *
		 * @param[out] x  will be assigned the value stored on the device.
		 */
		void storeBlocking(T &x) {
			m_devCon->getCommandQueue().enqueueReadBuffer(m_buffer, CL_TRUE, 0, sizeof(T), &x);
		}

		//! Enqueues a command for storing the data on the device in \a x.
		/**
		 * This method enqueues a non-blocking read-buffer command and returns an event object associated with
		 * this read command that can be queried (or waited for).
		 *
		 * @param[in,out] x            will be assigned the value stored on the device.
		 * @param[in,out] eventStore   if not 0, returns an event object that identifies the read command.
		 */
		void store(T &x, cl::Event *eventStore = 0) {
			m_devCon->getCommandQueue().enqueueReadBuffer(m_buffer, CL_FALSE, 0, sizeof(T), &x, 0, eventStore);
		}

		//@}
	};

}


#endif
