
#ifndef _TBT_MAPPED_STRUCT_H
#define _TBT_MAPPED_STRUCT_H

#include <tbt/DeviceStruct.h>
#include <tbt/Utility.h>


namespace tbt
{

	//! Structure mapped to an OpenCL device.
	/**
	 * \ingroup memobjects
	 */
	template<class T>
	class MappedStruct : public DeviceStruct<T>
	{
		T *m_ptr;  //!< pointer to memory allocated for structure.

	public:
		/** @name Constructors & Destructor
		 */
		//@{

		//! Constructs an invalid mapped structure.
		/**
		 * The constructed mapped structure is neither associated with an OpenCL buffer object nor a
		 * device controller. Hence, it cannot be used for loading or storing data.
		 */
		MappedStruct() : DeviceStruct<T>(), m_ptr(0) { }

		//! Constructs a mapped strucutre of type \a T associated with device controller \a devCon.
		/**
		 * @param devCon  must be a valid device controller that will be associated with the device structure.
		 * @param flags   are the memory flags (restricting the access of kernels to the corresponding device memory)
		 *                that will be used for creating the OpenCL buffer object; \a flags can be one of
		 *                CL_MEM_READ_WRITE, CL_MEM_READ_ONLY and CL_MEM_WRITE_ONLY.
		 */
		MappedStruct(DeviceController *devCon, cl_mem_flags flags = CL_MEM_READ_WRITE)
			: DeviceStruct<T>()
		{
			m_devCon = devCon;
			m_ptr = (T *) Utility::alignedMalloc(sizeof(T),devCon->getMemBaseAddrAlign() >> 3);
			m_buffer = cl::Buffer( devCon->getContext(),
				CL_MEM_USE_HOST_PTR | (flags & (CL_MEM_READ_WRITE | CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY)), sizeof(T), m_ptr );
		}

		//! Destructor. Releases allocated memory.
		~MappedStruct() {
			Utility::alignedFree(m_ptr);
		}

		//@}
		
		/** @name Access Operators
		 * These methods provide access to the elements in host memory. If array elements have been changed
		 * in device memory, they must first be explicitly transfered back to host memory (see
		 * <strong>Transfering Data Between Host and Device</strong> below).
		 */
		//@{

		const T &operator*() const { return *m_ptr; }

		T &operator*() { return *m_ptr; }

		const T *operator->() const { return m_ptr; }

		T *operator->() { return m_ptr; }

		//@}


		/** @name Transfering Data Between Host and Device
		 *  These methods transfer data from host to device memory, and vice versa. They enqueue map-buffer
		 *  commands to the command queue for the associated device. If the device is the CPU, the device memory
		 *  is the host memory and no actual data transfer operations are necessary.
		 */
		//@{

		//! Enqueues a blocking command for mapping the strucutre on the device to the structure on the host; transfers data if necessary.
		/**
		 * This methods enqueues a blocking, (reading) map-buffer command. Once this method returns, the memory
		 * transfer to the host is completed. If the device is the CPU, no memory transfer is necessary.
		 */
		void mapDeviceToHostBlocking() {
			m_devCon->getCommandQueue().enqueueMapBuffer(m_buffer, CL_TRUE, CL_MAP_READ, 0, sizeof(T));
		}

		//! Enqueues a command for mapping the strucutre on the device to the strucutre on the host; transfers data if necessary.
		/**
		 * This methods enqueues a non-blocking, (reading) map-buffer command and returns an event object associated with
		 * this map command that can be queried (or waited for). If the device is the CPU, no memory transfer is necessary.
		 *
		 * @param eventMap   if not 0, returns an event object that identifies the map command.
		 */
		void mapDeviceToHost(cl::Event *eventMap = 0) {
			m_devCon->getCommandQueue().enqueueMapBuffer(m_buffer, CL_FALSE, CL_MAP_READ, 0, sizeof(T), 0, eventMap);
		}

		//! Enqueues a blocking command for mapping the strucutre on the host to the strucutre on the device; transfers data if necessary.
		/**
		 * This methods enqueues a blocking, (writing) map-buffer command. Once this method returns, the memory
		 * transfer to the device is completed. If the device is the CPU, no memory transfer is necessary.
		 */
		void mapHostToDeviceBlocking() {
			m_devCon->getCommandQueue().enqueueMapBuffer(m_buffer, CL_TRUE, CL_MAP_WRITE, 0, sizeof(T));
		}

		//! Enqueues a command for mapping the structure on the host to the strucutre on the device; transfers data if necessary.
		/**
		 * This methods enqueues a non-blocking, (writing) map-buffer command and returns an event object associated with
		 * this map command that can be queried (or waited for). If the device is the CPU, no memory transfer is necessary.
		 *
		 * @param eventMap   if not 0, returns an event object that identifies the map command.
		 */
		void mapHostToDevice(cl::Event *eventMap = 0) {
			m_devCon->getCommandQueue().enqueueMapBuffer(m_buffer, CL_FALSE, CL_MAP_WRITE, 0, sizeof(T), 0, eventMap);
		}

		//@}
	};

}

#endif
