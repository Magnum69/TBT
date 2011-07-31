
#ifndef _TBT_DEVICE_ARRAY_H
#define _TBT_DEVICE_ARRAY_H

#include <tbt/DeviceController.h>
#include <tbt/HostArray.h>


namespace tbt
{

	//! Array stored on an OpenCL device.
	template<class T>
	class DeviceArray
	{
	protected:
		cl::Buffer m_buffer;         //!< the allocated OpenCL buffer object.
		size_t m_nElements;          //!< the number of elements in the array.
		DeviceController *m_devCon;  //!< the associated device controller.

	public:
		//! Constructs an invalid device array.
		/**
		 * The constructed device array is neither associated with an OpenCL buffer object nor a
		 * device controller. Hence, it cannot be used for loading or storing data.
		 */
		DeviceArray() : m_nElements(0), m_devCon(0) { }

		//! Copy constructor. The constructed device array will share the OpenCL buffer object.
		DeviceArray(const DeviceArray<T> &da) : m_buffer(da.m_buffer), m_nElements(da.m_nElements), m_devCon(da.m_devCon) { }

		//! Constructs a device array for \a n elements of type \a T associated with device controller \a devCon.
		/**
		 * @param devCon  must be a valid device controller that will be associated with the device array.
		 * @param n       is the number of elements (of type \a T) in the constructed device array.
		 * @param flags   are the memory flags (restricting the access of kernels to the corresponding device memory)
		 *                that will be used for creating the OpenCL buffer object; \a flags can be one of
		 *                CL_MEM_READ_WRITE, CL_MEM_READ_ONLY and CL_MEM_WRITE_ONLY.
		 */
		DeviceArray(DeviceController *devCon, size_t n, cl_mem_flags flags = CL_MEM_READ_WRITE)
			: m_buffer( devCon->getContext(), flags & (CL_MEM_READ_WRITE | CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY), n*sizeof(T) ),
			  m_nElements(n),
			  m_devCon(devCon)
		{ }

		//! Returns the device controller associated with this device array.
		DeviceController *getDeviceController() const {
			return m_devCon;
		}

		//! Returns the coresponding OpenCL buffer object.
		cl::Buffer &getBuffer() { return m_buffer; }

		//! Converts a device array to an OpenCL buffer object.
		operator cl::Buffer() { return m_buffer; }

		//! Returns the number of elements in the array.
		size_t size() const {
			return m_nElements;
		}


		/** @name Transfering data between host and device
		 *  These methods load data from host to device memory, or store the data in device memory
		 *  in host memory. They enqueue read- and write-buffer commands to the command queue
		 *  for the associated device.
		 */
		//@{

		//! Loads data from C-array \a ptr onto the device.
		/**
		 * This methods enqueues a blocking write-buffer command. Once this method returns, the memory
		 * transfer to the device is completed.
		 *
		 * @param ptr  must point to an allocated region of memory that is large enough to store the whole array.
		 */
		void loadFromBlocking(T *ptr) {
			m_devCon->getCommandQueue().enqueueWriteBuffer(m_buffer, CL_TRUE, 0, m_nElements*sizeof(T), ptr);
		}

		//! Loads data from host array \a a onto the device.
		/**
		 * This methods enqueues a blocking write-buffer command. Once this method returns, the memory
		 * transfer to the device is completed.
		 *
		 * @param a  must be a host array that is large enough to store the whole array.
		 */
		void loadFromBlocking(const HostArray<T> &a) {
			m_devCon->getCommandQueue().enqueueWriteBuffer(m_buffer, CL_TRUE, 0, m_nElements*sizeof(T), &a[0]);
		}

		//! Enqueues a command for loading data from a C-array \a ptr onto the device.
		/**
		 * This methods enqueues a non-blocking write-buffer command and returns an event object associated with
		 * this write command that can be queried (or waited for).
		 *
		 * @param ptr  must point to an allocated region of memory that is large enough to store the whole array.
		 * @param eventLoad   if not 0, returns an event object that identifies the write command.
		 */
		void loadFrom(T *ptr, cl::Event *eventLoad = 0) {
			m_devCon->getCommandQueue().enqueueWriteBuffer(m_buffer, CL_FALSE, 0, m_nElements*sizeof(T), ptr, 0, eventLoad);
		}

		//! Enqueues a command for loading data from host array \a a onto the device.
		/**
		 * This methods enqueues a non-blocking write-buffer command and returns an event object associated with
		 * this write command that can be queried (or waited for).
		 *
		 * @param a           must be a host array that is large enough to store the whole array.
		 * @param eventLoad   if not 0, returns an event object that identifies the write command.
		 */
		void loadFrom(const HostArray<T> &a, cl::Event *eventLoad = 0) {
			m_devCon->getCommandQueue().enqueueWriteBuffer(m_buffer, CL_FALSE, 0, m_nElements*sizeof(T), &a[0], 0, eventLoad);
		}

		//! Stores the data on the device in C-array \a ptr.
		/**
		 * This methods enqueues a blocking read-buffer command. Once this method returns, the memory
		 * transfer to the host is completed.
		 *
		 * @param ptr  must point to an allocated region of memory that is large enough to hold the whole array.
		 */
		void storeToBlocking(T *ptr) {
			m_devCon->getCommandQueue().enqueueReadBuffer(m_buffer, CL_TRUE, 0, m_nElements*sizeof(T), ptr);
		}

		//! Stores the data on the device in host array \a a.
		/**
		 * This methods enqueues a blocking read-buffer command. Once this method returns, the memory
		 * transfer to the host is completed.
		 *
		 * @param a  must be a host array that is large enough to hold the whole array.
		 */
		void storeToBlocking(HostArray<T> &a) {
			m_devCon->getCommandQueue().enqueueReadBuffer(m_buffer, CL_TRUE, 0, m_nElements*sizeof(T), &a[0]);
		}

		//! Enqueues a command for storing the data on the device in C-array \a ptr.
		/**
		 * This methods enqueues a non-blocking read-buffer command and returns an event object associated with
		 * this read command that can be queried (or waited for).
		 *
		 * @param ptr          must point to an allocated region of memory that is large enough to hold the whole array.
		 * @param eventStore   if not 0, returns an event object that identifies the read command.
		 */
		void storeTo(T *ptr, cl::Event *eventStore = 0) {
			m_devCon->getCommandQueue().enqueueReadBuffer(m_buffer, CL_FALSE, 0, m_nElements*sizeof(T), ptr, 0, eventStore);
		}

		//! Enqueues a command for storing the data on the device in host array \a a.
		/**
		 * This methods enqueues a non-blocking read-buffer command and returns an event object associated with
		 * this read command that can be queried (or waited for).
		 *
		 * @param a            must be a host array that is large enough to hold the whole array.
		 * @param eventStore   if not 0, returns an event object that identifies the read command.
		 */
		void storeTo(HostArray<T> &a, cl::Event *eventStore = 0) {
			m_devCon->getCommandQueue().enqueueReadBuffer(m_buffer, CL_FALSE, 0, m_nElements*sizeof(T), &a[0], 0, eventStore);
		}

		//@}
	};

}

#endif
