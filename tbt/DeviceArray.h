
#ifndef _TBT_DEVICE_ARRAY_H
#define _TBT_DEVICE_ARRAY_H

#include <tbt/DeviceController.h>
#include <tbt/HostArray.h>


namespace tbt
{

	template<class T> class DeviceArray;
	template<class T> class _DeviceArrayIterator;

	//! Const-iterator for device arrays.
	template<class T>
	class _DeviceArrayConstIterator
	{
		friend class DeviceArray<T>;

		index_t               m_index;     //!< the index in the device array.
		const DeviceArray<T> *m_devArray;  //!< the device array this iterator points to.

		//! Constructs an const-iterator pointing to \a index in \a devArray.
		/**
		 * This constructor can only be called from class DeviceArray.
		 */
		_DeviceArrayConstIterator(index_t index, const DeviceArray<T> *devArray) : m_index(index), m_devArray(devArray) { }

	public:
		//! The type for iterator differences.
		typedef index_t difference_type;

		//! Constructs an invalid const-iterator.
		_DeviceArrayConstIterator() : m_index(0), m_devArray(0) { }

		//! Copy constructor. Constructs a const-iterator pointing to the same position as \a iter.
		_DeviceArrayConstIterator(const _DeviceArrayConstIterator<T> &iter) : m_index(iter.m_index), m_devArray(iter.m_devArray) { }

		//! Constructs a const-iterator pointing to the same position as iterator \a iter.
		_DeviceArrayConstIterator(const _DeviceArrayIterator<T> &iter) : m_index(iter.m_index), m_devArray(iter.m_devArray) { }

		//! Returns the device array this iterator points to.
		/**
		 * Returns 0 if this is an invalid const-iterator.
		 */
		const DeviceArray<T> *getDeviceArray() const { return m_devArray; }

		//! Assignment operator.
		_DeviceArrayConstIterator<T> &operator=(const _DeviceArrayConstIterator<T> &iter) {
			m_index    = iter.m_index;
			m_devArray = iter.m_devArray;
			return *this;
		}

		//! Moves iterator one position forward (pre-increment).
		/**
		 * \pre This const-iterator must be valid.
		 */
		_DeviceArrayConstIterator<T> &operator++() {
			++m_index;
			return *this;
		}

		//! Moves iterator one position forward (post-increment).
		/**
		 * \pre This const-iterator must be valid.
		 */
		_DeviceArrayConstIterator<T> operator++(int) {
			_DeviceArrayConstIterator<T> iter(*this);
			++m_index;
			return iter;
		}

		//! Moves iterator one position backward (pre-increment).
		/**
		 * \pre This const-iterator must be valid.
		 */
		_DeviceArrayConstIterator<T> &operator--() {
			--m_index;
			return *this;
		}

		//! Moves iterator one position backward (post-increment).
		/**
		 * \pre This const-iterator must be valid.
		 */
		_DeviceArrayConstIterator<T> operator--(int) {
			_DeviceArrayConstIterator<T> iter(*this);
			--m_index;
			return iter;
		}

		//! Moves iterator \a offset positions forward.
		/**
		 * \pre This const-iterator must be valid.
		 */
		_DeviceArrayConstIterator<T> &operator+=(difference_type offset) {
			m_index += offset;
			return *this;
		}

		//! Returns an iterator pointing to a position \a offset positions forward of this iterator.
		/**
		 * \pre This const-iterator must be valid.
		 */
		_DeviceArrayConstIterator<T> operator+(difference_type offset) const {
			_DeviceArrayConstIterator<T> iter(*this);
			return ( iter += offset);
		}

		//! Moves iterator \a offset positions backward.
		/**
		 * \pre This const-iterator must be valid.
		 */
		_DeviceArrayConstIterator<T> &operator-=(difference_type offset) {
			m_index -= offset;
			return *this;
		}

		//! Returns an iterator pointing to a position \a offset positions backward of this iterator.
		/**
		 * \pre This const-iterator must be valid.
		 */
		_DeviceArrayConstIterator<T> operator-(difference_type offset) const {
			_DeviceArrayConstIterator<T> iter(*this);
			return ( iter -= offset);
		}

		//! Returns the difference between two iterators.
		/**
		 * \pre This const-iterator and \a iter must both be valid and point to the same device array.
		 *
		 * @param iter must be a valid const-iterator pointing to the same device array as this const-iterator.
		 * @return the difference between the two iterators, i.e., the number of elements in the
		 *         interval [this iterator, \a iter); this difference will be 0 if both iterators
		 *         point to the same position, and negative if \a iter points to a position before the
		 *         position this iterator points to.
		 */
		difference_type operator-(const _DeviceArrayConstIterator<T> &iter) const {
			return m_index - iter.m_index;
		}

	};


	//! Iterator for device arrays.
	template<class T>
	class _DeviceArrayIterator
	{
		friend class DeviceArray<T>;
		friend class _DeviceArrayIterator<T>;

		index_t         m_index;     //!< the index in the device array.
		DeviceArray<T> *m_devArray;  //!< the device array this iterator points to.

		//! Constructs an iterator pointing to \a index in \a devArray.
		/**
		 * This constructor can only be called from class DeviceArray.
		 */
		_DeviceArrayIterator(index_t index, DeviceArray<T> *devArray) : m_index(index), m_devArray(devArray) { }

	public:
		//! The type for iterator differences.
		typedef index_t difference_type;

		//! Constructs an invalid iterator.
		_DeviceArrayIterator() : m_index(0), m_devArray(0) { }

		//! Copy constructor. Constructs an iterator pointing to the same position as \a iter.
		_DeviceArrayIterator(const _DeviceArrayIterator<T> &iter) : m_index(iter.m_index), m_devArray(iter.m_devArray) { }

		//! Returns the device array this iterator points to.
		/**
		 * Returns 0 if this is an invalid iterator.
		 */
		DeviceArray<T> *getDeviceArray() { return m_devArray; }

		//! Assignment operator.
		_DeviceArrayIterator<T> &operator=(const _DeviceArrayIterator<T> &iter) {
			m_index    = iter.m_index;
			m_devArray = iter.m_devArray;
			return *this;
		}

		//! Moves iterator one position forward (pre-increment).
		/**
		 * \pre This iterator must be valid.
		 */
		_DeviceArrayIterator<T> &operator++() {
			++m_index;
			return *this;
		}

		//! Moves iterator one position forward (post-increment).
		/**
		 * \pre This iterator must be valid.
		 */
		_DeviceArrayIterator<T> operator++(int) {
			_DeviceArrayIterator<T> iter(*this);
			++m_index;
			return iter;
		}

		//! Moves iterator one position backward (pre-increment).
		/**
		 * \pre This iterator must be valid.
		 */
		_DeviceArrayIterator<T> &operator--() {
			--m_index;
			return *this;
		}

		//! Moves iterator one position backward (post-increment).
		/**
		 * \pre This iterator must be valid.
		 */
		_DeviceArrayIterator<T> operator--(int) {
			_DeviceArrayIterator<T> iter(*this);
			--m_index;
			return iter;
		}

		//! Moves iterator \a offset positions forward.
		/**
		 * \pre This iterator must be valid.
		 */
		_DeviceArrayIterator<T> &operator+=(difference_type offset) {
			m_index += offset;
			return *this;
		}

		//! Returns an iterator pointing to a position \a offset positions forward of this iterator.
		/**
		 * \pre This iterator must be valid.
		 */
		_DeviceArrayIterator<T> operator+(difference_type offset) const {
			_DeviceArrayIterator<T> iter(*this);
			return ( iter += offset);
		}

		//! Moves iterator \a offset positions backward.
		/**
		 * \pre This iterator must be valid.
		 */
		_DeviceArrayIterator<T> &operator-=(difference_type offset) {
			m_index -= offset;
			return *this;
		}

		//! Returns an iterator pointing to a position \a offset positions backward of this iterator.
		/**
		 * \pre This iterator must be valid.
		 */
		_DeviceArrayIterator<T> operator-(difference_type offset) const {
			_DeviceArrayIterator<T> iter(*this);
			return ( iter -= offset);
		}

		//! Returns the difference between two iterators.
		/**
		 * \pre This iterator and \a iter must both be valid and point to the same device array.
		 *
		 * @param iter must be a valid iterator pointing to the same device array as this iterator.
		 * @return the difference between the two iterators, i.e., the number of elements in the
		 *         interval [this iterator, \a iter); this difference will be 0 if both iterators
		 *         point to the same position, and negative if \a iter points to a position before the
		 *         position this iterator points to.
		 */
		difference_type operator-(const _DeviceArrayConstIterator<T> &iter) const {
			return m_index - iter.m_index;
		}

	};


	//! Array stored on an OpenCL device.
	template<class T>
	class DeviceArray
	{
	protected:
		cl::Buffer m_buffer;         //!< the allocated OpenCL buffer object.
		index_t m_nElements;         //!< the number of elements in the array.
		DeviceController *m_devCon;  //!< the associated device controller.

	public:
		//! Const-iterator for device arrays.
		typedef _DeviceArrayConstIterator<T> const_iterator;

		//! Iterator for device arrays.
		typedef _DeviceArrayIterator<T> iterator;

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
		 * @param[in] devCon  must be a valid device controller that will be associated with the device array.
		 * @param[in] n       is the number of elements (of type \a T) in the constructed device array.
		 * @param[in] flags   are the memory flags (restricting the access of kernels to the corresponding device memory)
		 *                    that will be used for creating the OpenCL buffer object; \a flags can be one of
		 *                    CL_MEM_READ_WRITE, CL_MEM_READ_ONLY and CL_MEM_WRITE_ONLY.
		 */
		DeviceArray(DeviceController *devCon, index_t n, cl_mem_flags flags = CL_MEM_READ_WRITE)
			: m_buffer( devCon->getContext(), flags & (CL_MEM_READ_WRITE | CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY), n*sizeof(T) ),
			  m_nElements(n),
			  m_devCon(devCon)
		{ }

		//! Returns the device controller associated with this device array.
		/**
		 * @return the device controller associated with this device array; will be 0 if the device array is invalid.
		 */
		DeviceController *getDeviceController() const {
			return m_devCon;
		}

		//! Returns the corresponding OpenCL buffer object.
		/**
		 * @return the OpenCL buffer object of this device array; will be invalid if the device array is invalid.
		 */
		cl::Buffer &getBuffer() { return m_buffer; }

		//! Converts a device array to an OpenCL buffer object.
		/**
		 * @return the OpenCL buffer object of this device array; will be invalid if the device array is invalid.
		 */
		operator cl::Buffer() { return m_buffer; }

		//! Returns the number of elements in the array.
		/**
		 * @return the number of elements in this device array; will be 0 if the device array is invalid.
		 */
		index_t size() const {
			return m_nElements;
		}

		//! Returns an iterator pointing to the first element in the array.
		iterator begin() { return iterator(0,this); }

		//! Returns a const-iterator pointing to the first element in the array.
		const_iterator begin() const { return const_iterator(0,this); }

		//! Returns an iterator pointing to one past the last element in the array.
		iterator end() { return iterator(m_nElements,this); }

		//! Returns a const-iterator pointing to one past the last element in the array.
		const_iterator end() const { return const_iterator(m_nElements,this); }

		//! Returns an iterator pointing to the first element in the reversed array.
		/**
		 * The returned iterator also points to the last element in the array.
		 */
		iterator rbegin() { return iterator(0,this); }

		//! Returns a const-iterator pointing to the first element in the reversed array.
		/**
		 * The returned iterator also points to the last element in the array.
		 */
		const_iterator rbegin() const { return const_iterator(0,this); }

		//! Returns an iterator pointing to one past the last element in the reversed array.
		/**
		 * The returned iterator also points to one before the first element in the array.
		 */
		iterator rend() { return iterator(m_nElements,this); }

		//! Returns a const-iterator pointing to one past the last element in the reversed array.
		/**
		 * The returned iterator also points to one before the first element in the array.
		 */
		const_iterator rend() const { return const_iterator(m_nElements,this); }


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
		 * @param[in] ptr  must point to an allocated region of memory that is large enough to store the whole array.
		 */
		void loadFromBlocking(const T *ptr) {
			m_devCon->getCommandQueue().enqueueWriteBuffer(m_buffer, CL_TRUE, 0, m_nElements*sizeof(T), ptr);
		}

		//! Loads data from host array \a a onto the device.
		/**
		 * This methods enqueues a blocking write-buffer command. Once this method returns, the memory
		 * transfer to the device is completed.
		 *
		 * @param[in] ha  must be a host array that is large enough to store the whole array.
		 */
		void loadFromBlocking(const HostArray<T> &ha) {
			m_devCon->getCommandQueue().enqueueWriteBuffer(m_buffer, CL_TRUE, 0, m_nElements*sizeof(T), &ha[0]);
		}

		//! Enqueues a command for loading data from a C-array \a ptr onto the device.
		/**
		 * This methods enqueues a non-blocking write-buffer command and returns an event object associated with
		 * this write command that can be queried (or waited for).
		 *
		 * @param[in]     ptr         must point to an allocated region of memory that is large enough to store the whole array.
		 * @param[in,out] eventLoad   if not 0, returns an event object that identifies the write command.
		 */
		void loadFrom(const T *ptr, cl::Event *eventLoad = 0) {
			m_devCon->getCommandQueue().enqueueWriteBuffer(m_buffer, CL_FALSE, 0, m_nElements*sizeof(T), ptr, 0, eventLoad);
		}

		//! Enqueues a command for loading data from host array \a a onto the device.
		/**
		 * This methods enqueues a non-blocking write-buffer command and returns an event object associated with
		 * this write command that can be queried (or waited for).
		 *
		 * @param[in]     ha          must be a host array that is large enough to store the whole array.
		 * @param[in,out] eventLoad   if not 0, returns an event object that identifies the write command.
		 */
		void loadFrom(const HostArray<T> &ha, cl::Event *eventLoad = 0) {
			m_devCon->getCommandQueue().enqueueWriteBuffer(m_buffer, CL_FALSE, 0, m_nElements*sizeof(T), &ha[0], 0, eventLoad);
		}

		//! Stores the data on the device in C-array \a ptr.
		/**
		 * This methods enqueues a blocking read-buffer command. Once this method returns, the memory
		 * transfer to the host is completed.
		 *
		 * @param[in,out] ptr  must point to an allocated region of memory that is large enough to hold the whole array.
		 */
		void storeToBlocking(T *ptr) {
			m_devCon->getCommandQueue().enqueueReadBuffer(m_buffer, CL_TRUE, 0, m_nElements*sizeof(T), ptr);
		}

		//! Stores the data on the device in host array \a a.
		/**
		 * This methods enqueues a blocking read-buffer command. Once this method returns, the memory
		 * transfer to the host is completed.
		 *
		 * @param[in,out] ha  must be a host array that is large enough to hold the whole array.
		 */
		void storeToBlocking(HostArray<T> &ha) {
			m_devCon->getCommandQueue().enqueueReadBuffer(m_buffer, CL_TRUE, 0, m_nElements*sizeof(T), &ha[0]);
		}

		//! Enqueues a command for storing the data on the device in C-array \a ptr.
		/**
		 * This methods enqueues a non-blocking read-buffer command and returns an event object associated with
		 * this read command that can be queried (or waited for).
		 *
		 * @param[in,out] ptr          must point to an allocated region of memory that is large enough to hold the whole array.
		 * @param[in,out] eventStore   if not 0, returns an event object that identifies the read command.
		 */
		void storeTo(T *ptr, cl::Event *eventStore = 0) {
			m_devCon->getCommandQueue().enqueueReadBuffer(m_buffer, CL_FALSE, 0, m_nElements*sizeof(T), ptr, 0, eventStore);
		}

		//! Enqueues a command for storing the data on the device in host array \a a.
		/**
		 * This methods enqueues a non-blocking read-buffer command and returns an event object associated with
		 * this read command that can be queried (or waited for).
		 *
		 * @param[in,out] ha           must be a host array that is large enough to hold the whole array.
		 * @param[in,out] eventStore   if not 0, returns an event object that identifies the read command.
		 */
		void storeTo(HostArray<T> &ha, cl::Event *eventStore = 0) {
			m_devCon->getCommandQueue().enqueueReadBuffer(m_buffer, CL_FALSE, 0, m_nElements*sizeof(T), &ha[0], 0, eventStore);
		}

		//@}
	};

}

#endif
