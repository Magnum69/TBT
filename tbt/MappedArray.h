
#ifndef _TBT_MAPPED_ARRAY_H
#define _TBT_MAPPED_ARRAY_H

#include <tbt/DeviceArray.h>
#include <tbt/Utility.h>


namespace tbt
{

	//! Array mapped to an OpenCL device.
	template<class T>
	class MappedArray : public DeviceArray<T>
	{
		T     *m_ptr;                //!< pointer to memory allocated for array.
		//cl::Buffer m_buffer;         //!< the allocated OpenCL buffer object.
		//size_t m_nElements;          //!< the number of elements in the array.
		//DeviceController *m_devCon;  //!< the associated device controller.

	public:
		//! Constructs an invalid mapped array.
		/**
		 * The constructed mapped array is neither associated with an OpenCL buffer object nor a
		 * device controller. Hence, it cannot be used for loading or storing data.
		 */
		MappedArray() : DeviceArray<T>(), m_ptr(0) { }

		//! Constructs a mapped array for \a n elements of type \a T associated with device controller \a devCon.
		/**
		 * @param devCon  must be a valid device controller that will be associated with the device array.
		 * @param n       is the number of elements (of type \a T) in the constructed device array.
		 * @param flags   are the memory flags (restricting the access of kernels to the corresponding device memory)
		 *                that will be used for creating the OpenCL buffer object; \a flags can be one of
		 *                CL_MEM_READ_WRITE, CL_MEM_READ_ONLY and CL_MEM_WRITE_ONLY.
		 */
		MappedArray(DeviceController *devCon, size_t n, cl_mem_flags flags = CL_MEM_READ_WRITE)
			: DeviceArray<T>()
		{
			m_nElements = n;
			m_devCon = devCon;
			m_ptr = (n > 0) ? (T *) Utility::alignedMalloc(n*sizeof(T),devCon->getMemBaseAddrAlign() >> 3) : 0;
			m_buffer = cl::Buffer( devCon->getContext(),
				CL_MEM_USE_HOST_PTR | (flags & (CL_MEM_READ_WRITE | CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY)), n*sizeof(T), m_ptr );
		}

		//! Destructor. Releases allocated memory.
		~MappedArray() {
			Utility::alignedFree(m_ptr);
		}

		//! Returns a reference to the <i>i</i>-th element in the array.
		const T &operator[](size_t i) const {
			return m_ptr[i];
		}

		//! Returns a reference to the <i>i</i>-th element in the array.
		T &operator[](size_t i) {
			return m_ptr[i];
		}


		/** @name Transfering data between host and device
		 *  These methods transfer data from host to device memory, and vice versa. They enqueue map-buffer
		 *  commands to the command queue for the associated device. If the device is the CPU, the device memory
		 *  is the host memory and no actual data transfer operations are necessary.
		 */
		//@{

		//! Enqueues a blocking command for mapping the array on the device to the array on the host; transfers data if necessary.
		/**
		 * This methods enqueues a blocking, (reading) map-buffer command. Once this method returns, the memory
		 * transfer to the host is completed. If the device is the CPU, no memory transfer is necessary.
		 */
		void mapDeviceToHostBlocking() {
			m_devCon->getCommandQueue().enqueueMapBuffer(m_buffer, CL_TRUE, CL_MAP_READ, 0, m_nElements*sizeof(T));
		}

		//! Enqueues a command for mapping the array on the device to the array on the host; transfers data if necessary.
		/**
		 * This methods enqueues a non-blocking, (reading) map-buffer command and returns an event object associated with
		 * this map command that can be queried (or waited for). If the device is the CPU, no memory transfer is necessary.
		 *
		 * @param eventMap   if not 0, returns an event object that identifies the map command.
		 */
		void mapDeviceToHost(cl::Event *eventMap = 0) {
			m_devCon->getCommandQueue().enqueueMapBuffer(m_buffer, CL_FALSE, CL_MAP_READ, 0, m_nElements*sizeof(T), 0, eventMap);
		}

		//! Enqueues a blocking command for mapping the array on the host to the array on the device; transfers data if necessary.
		/**
		 * This methods enqueues a blocking, (writing) map-buffer command. Once this method returns, the memory
		 * transfer to the device is completed. If the device is the CPU, no memory transfer is necessary.
		 */
		void mapHostToDeviceBlocking() {
			m_devCon->getCommandQueue().enqueueMapBuffer(m_buffer, CL_TRUE, CL_MAP_WRITE, 0, m_nElements*sizeof(T));
		}

		//! Enqueues a command for mapping the array on the host to the array on the device; transfers data if necessary.
		/**
		 * This methods enqueues a non-blocking, (writing) map-buffer command and returns an event object associated with
		 * this map command that can be queried (or waited for). If the device is the CPU, no memory transfer is necessary.
		 *
		 * @param eventMap   if not 0, returns an event object that identifies the map command.
		 */
		void mapHostToDevice(cl::Event *eventMap = 0) {
			m_devCon->getCommandQueue().enqueueMapBuffer(m_buffer, CL_FALSE, CL_MAP_WRITE, 0, m_nElements*sizeof(T), 0, eventMap);
		}

		//@}
	};

}

#endif
