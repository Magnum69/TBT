
#ifndef _TBT_MAPPED_ARRAY_H
#define _TBT_MAPPED_ARRAY_H

#include <tbt/DeviceArray.h>
#include <tbt/Utility.h>


namespace tbt
{

	template<class T> class _MappedArrayConstIterator;
	template<class T> class _MappedArrayIterator;


	//! Array mapped to an OpenCL device.
	template<class T>
	class MappedArray : public DeviceArray<T>
	{
		T *m_ptr;  //!< pointer to memory allocated for array.

	public:

		//! Const-iterator for mapped arrays.
		typedef _MappedArrayConstIterator<T> const_iterator;

		//! Iterator for mapped arrays.
		typedef _MappedArrayIterator<T> iterator;

		//! Const-reference type for mapped arrays.
		typedef const T &const_reference;

		//! Reference type for mapped arrays.
		typedef T &reference;


		/** @name Constructors & Destructor
		 */
		//@{

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

		//@}


		/** @name Access Operators
		 * These methods provide access to the elements in host memory. If array elements have been changed
		 * in device memory, they must first be explicitly transfered back to host memory (see
		 * <strong>Transfering Data Between Host and Device</strong> below).
		 */
		//@{

		//! Returns a reference to the <i>i</i>-th element in the array.
		const_reference operator[](size_t i) const {
			return m_ptr[i];
		}

		//! Returns a reference to the <i>i</i>-th element in the array.
		reference operator[](size_t i) {
			return m_ptr[i];
		}

		//@}


		/** @name Iterators
		 * These methods return iterators to specific positions in the mapped array.
		 */
		//@{

		//! Returns an iterator pointing to the first element in the array.
		iterator begin();

		//! Returns a const-iterator pointing to the first element in the array.
		const_iterator begin() const;

		//! Returns an iterator pointing to one past the last element in the array.
		iterator end();

		//! Returns a const-iterator pointing to one past the last element in the array.
		const_iterator end() const;

		//! Returns an iterator pointing to the first element in the reversed array.
		/**
		 * The returned iterator also points to the last element in the array.
		 */
		iterator rbegin();

		//! Returns a const-iterator pointing to the first element in the reversed array.
		/**
		 * The returned iterator also points to the last element in the array.
		 */
		const_iterator rbegin() const;

		//! Returns an iterator pointing to one past the last element in the reversed array.
		/**
		 * The returned iterator also points to one before the first element in the array.
		 */
		iterator rend();

		//! Returns a const-iterator pointing to one past the last element in the reversed array.
		/**
		 * The returned iterator also points to one before the first element in the array.
		 */
		const_iterator rend() const;

		//! Returns an iterator pointing to the <i>i</i>-th element in the array.
		iterator at(index_t i);

		//! Returns a const-iterator pointing to the <i>i</i>-th element in the array.
		const_iterator at(index_t i) const;

		//@}


		/** @name Transfering Data Between Host and Device
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


	//! Const-iterator for mapped arrays.
	template<class T>
	class _MappedArrayConstIterator : public _DeviceArrayConstIterator<T>
	{
		//! The reference type for this iterator.
		friend class MappedArray<T>;

		//! Constructs an const-iterator pointing to \a index in \a devArray.
		/**
		 * This constructor can only be called from class DeviceArray.
		 */
		_MappedArrayConstIterator(index_t index, const MappedArray<T> *mappedArray) : _DeviceArrayConstIterator(index,mappedArray) { }

	public:
		//! The reference type for this iterator.
		typedef typename MappedArray<T>::const_reference reference;

		//! Constructs an invalid const-iterator.
		_MappedArrayConstIterator() : _DeviceArrayConstIterator() { }

		//! Copy constructor. Constructs a const-iterator pointing to the same position as \a iter.
		_MappedArrayConstIterator(const _MappedArrayConstIterator<T> &iter) : _DeviceArrayConstIterator(iter) { }

		//! Constructs a const-iterator pointing to the same position as iterator \a iter.
		_MappedArrayConstIterator(const _MappedArrayIterator<T> &iter) : _DeviceArrayConstIterator(iter) { }

		//! Returns the mapped array this iterator points to.
		/**
		 * Returns 0 if this is an invalid const-iterator.
		 */
		const MappedArray<T> *getMappedArray() const { return (const MappedArray<T> *)getDeviceArray(); }

		//! Assignment operator.
		_MappedArrayConstIterator<T> &operator=(const _MappedArrayConstIterator<T> &iter) {
			_DeviceArrayConstIterator<T>::operator=(iter);
			return *this;
		}

		//! Returns a reference to the element in the mapped array this iterator points to.
		reference operator*() const {
			return (*getMappedArray())[getIndex()];
		}

	};


	//! Iterator for mapped arrays.
	template<class T>
	class _MappedArrayIterator : public _DeviceArrayIterator<T>
	{
		friend class MappedArray<T>;

		//! Constructs an const-iterator pointing to \a index in \a devArray.
		/**
		 * This constructor can only be called from class DeviceArray.
		 */
		_MappedArrayIterator(index_t index, MappedArray<T> *mappedArray) : _DeviceArrayIterator(index,mappedArray) { }

	public:
		//! The reference type for this iterator.
		typedef typename MappedArray<T>::reference reference;

		//! Constructs an invalid iterator.
		_MappedArrayIterator() : _DeviceArrayIterator() { }

		//! Copy constructor. Constructs an iterator pointing to the same position as \a iter.
		_MappedArrayIterator(const _MappedArrayIterator<T> &iter) : _DeviceArrayIterator(iter) { }

		//! Returns the mapped array this iterator points to.
		/**
		 * Returns 0 if this is an invalid const-iterator.
		 */
		MappedArray<T> *getMappedArray() { return (MappedArray<T> *)getDeviceArray(); }

		//! Assignment operator.
		_MappedArrayIterator<T> &operator=(const _MappedArrayIterator<T> &iter) {
			_DeviceArrayIterator<T>::operator=(iter);
			return *this;
		}

		//! Returns a reference to the element in the mapped array this iterator points to.
		reference operator*() const {
			return (*getMappedArray())[getIndex()];
		}

	};


	template<class T> inline
	typename MappedArray<T>::iterator MappedArray<T>::begin() { return iterator(0,this); }

	template<class T> inline
	typename MappedArray<T>::const_iterator MappedArray<T>::begin() const { return const_iterator(0,this); }

	template<class T> inline
	typename MappedArray<T>::iterator MappedArray<T>::end() { return iterator(m_nElements,this); }

	template<class T> inline
	typename MappedArray<T>::const_iterator MappedArray<T>::end() const { return const_iterator(m_nElements,this); }

	template<class T> inline
	typename MappedArray<T>::iterator MappedArray<T>::rbegin() { return iterator(m_nElements-1,this); }

	template<class T> inline
	typename MappedArray<T>::const_iterator MappedArray<T>::rbegin() const { return const_iterator(m_nElements-1,this); }

	template<class T> inline
	typename MappedArray<T>::iterator MappedArray<T>::rend() { return iterator(-1,this); }

	template<class T> inline
	typename MappedArray<T>::const_iterator MappedArray<T>::rend() const { return const_iterator(-1,this); }

	template<class T> inline
	typename MappedArray<T>::iterator MappedArray<T>::at(index_t i) { return iterator(i,this); }

	template<class T> inline
	typename MappedArray<T>::const_iterator MappedArray<T>::at(index_t i) const { return const_iterator(i,this); }
}

#endif
