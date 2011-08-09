
#ifndef _TBT_DEVICE_ARRAY_H
#define _TBT_DEVICE_ARRAY_H

#include <tbt/DeviceController.h>
#include <tbt/HostArray.h>


namespace tbt
{

	template<class T> class _DeviceArrayConstIterator;
	template<class T> class _DeviceArrayIterator;
	template<class T> class MappedArray;


	//! Array stored on an OpenCL device.
	/**
	 * \ingroup memobjects
	 */
	template<class T>
	class DeviceArray
	{
	protected:
		cl::Buffer m_buffer;         //!< the allocated OpenCL buffer object.
		index_t m_nElements;         //!< the number of elements in the array.
		DeviceController *m_devCon;  //!< the associated device controller.

	public:
		//! The type for iterator differences.
		typedef index_t difference_type;

		//! Const-iterator for device arrays.
		typedef _DeviceArrayConstIterator<T> const_iterator;

		//! Iterator for device arrays.
		typedef _DeviceArrayIterator<T> iterator;


		/** @name Constructors
		 */
		//@{

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

		//@}


		/** @name General Information
		 * These methods provide access to the associated device controller and the created OpenCL buffer object,
		 * as well as the size of the array.
		 */
		//@{

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

		//@}


		/** @name Iterators
		 * These methods return iterators to specific positions in the device array. Please note that these
		 * iterators do not provide access to the elements they point to, since the array is actually
		 * stored on an OpenCL device and must first be transfered to host memory for accessing the data.
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


		/** @name Transfering Data from Host to Device Memory
		 *  These methods load data from host memory onto the device. They enqueue read- and write-buffer
		 *  commands to the command queue for the associated device. The methods ending with <em>Blocking</em>
		 *  only return once the memory transfer is complete; the other methods just enqueue a command
		 *  for transfering the data (an event object can be used to wait for completion).
		 */
		//@{

		//! Loads data from C-array \a ptr onto the device.
		/**
		 * This method enqueues a blocking write-buffer command. Once this method returns, the memory
		 * transfer to the device is completed.
		 *
		 * @param[in] ptr  must point to an allocated region of memory that is large enough to store the whole array.
		 */
		void loadBlocking(const T *ptr) {
			m_devCon->getCommandQueue().enqueueWriteBuffer(m_buffer, CL_TRUE, 0, m_nElements*sizeof(T), ptr);
		}

		//! Loads data from host array \a ha onto the device.
		/**
		 * This method enqueues a blocking write-buffer command. Once this method returns, the memory
		 * transfer to the device is completed.
		 *
		 * @param[in] ha  must be a host array that is large enough to store the whole array.
		 */
		void loadBlocking(const HostArray<T> &ha) {
			m_devCon->getCommandQueue().enqueueWriteBuffer(m_buffer, CL_TRUE, 0, m_nElements*sizeof(T), &ha[0]);
		}

		//! Loads data from mapped array \a ma onto the device.
		/**
		 * This method enqueues a blocking write-buffer command. Once this method returns, the memory
		 * transfer to the device is completed.
		 *
		 * @param[in] ma  must be a mapped array that is large enough to store the whole array.
		 */
		void loadBlocking(const MappedArray<T> &ma);

		//! Loads the subarray [\a first, \a last) from host memory to the subarray starting at \a firstDev onto the device.
		/**
		 * This method enqueues a blocking write-buffer command. Once this method returns, the memory
		 * transfer to the device is completed. The method copies the elements in [\a first, \a last) to
		 * [\a firstDev, \a firstDev+(\a last-\a first)) in the device array,
		 *
		 * \pre \a \a first and \a last must be valid iterators pointing to the same array, and
		 *      \a firstDev must be a valid iterator pointing to this device array.
		 *
		 * @tparam    _ITER    must be a random access iterator for an array (i.e., the elements must be stored consecutively).
		 *                     Possible iterator types are, e.g., const-iterators for host or mapped arrays, or pointers to C-arrays.
		 * @param[in] first    is an iterator pointing to the first element in the subarray to be loaded.
		 * @param[in] last     is an iterator pointing to the one-past-last element in the subarray to be loaded.
		 * @param[in] firstDev is an iterator pointing to the starting position, where the subarray is stored in this device array.
		 */
		template<class _ITER>
		void loadBlocking(iterator first, iterator last, _ITER firstSrc) {
			m_devCon->getCommandQueue().enqueueWriteBuffer(m_buffer, CL_TRUE, first.m_index * sizeof(T), (last-first) * sizeof(T), &*firstSrc);
		}

		//! Enqueues a command for loading data from a C-array \a ptr onto the device.
		/**
		 * This method enqueues a non-blocking write-buffer command and returns an event object associated with
		 * this write command that can be queried (or waited for).
		 *
		 * @param[in]     ptr         must point to an allocated region of memory that is large enough to store the whole array.
		 * @param[in,out] eventLoad   if not 0, returns an event object that identifies the write command.
		 */
		void load(const T *ptr, cl::Event *eventLoad = 0) {
			m_devCon->getCommandQueue().enqueueWriteBuffer(m_buffer, CL_FALSE, 0, m_nElements*sizeof(T), ptr, 0, eventLoad);
		}

		//! Enqueues a command for loading data from host array \a ha onto the device.
		/**
		 * This method enqueues a non-blocking write-buffer command and returns an event object associated with
		 * this write command that can be queried (or waited for).
		 *
		 * @param[in]     ha          must be a host array that is large enough to store the whole array.
		 * @param[in,out] eventLoad   if not 0, returns an event object that identifies the write command.
		 */
		void load(const HostArray<T> &ha, cl::Event *eventLoad = 0) {
			m_devCon->getCommandQueue().enqueueWriteBuffer(m_buffer, CL_FALSE, 0, m_nElements*sizeof(T), &ha[0], 0, eventLoad);
		}

		//! Enqueues a command for loading data from mapped array \a ma onto the device.
		/**
		 * This method enqueues a non-blocking write-buffer command and returns an event object associated with
		 * this write command that can be queried (or waited for).
		 *
		 * @param[in]     ma          must be a mapped array that is large enough to store the whole array.
		 * @param[in,out] eventLoad   if not 0, returns an event object that identifies the write command.
		 */
		void load(const MappedArray<T> &ma, cl::Event *eventLoad = 0);

		//! Enqueues a command for loading the subarray [\a first, \a last) from host memory at \a firstDev onto the device.
		/**
		 * This method enqueues a non-blocking write-buffer command and returns an event object associated with
		 * this write command that can be queried (or waited for). The command copies \ m := \a last-\a first elements in
		 * [\a firstSrc, \a firstSrc+\ m) to [\a first, \a last) in the device array,
		 *
		 * \pre \a \a first and \a last must be valid iterators pointing to this device array, and
		 *      \a firstSrc must be a valid iterator pointing to an array.
		 *
		 * @tparam        _ITER      must be an array iterator (i.e., the elements must be stored consecutively).
		 *                           Possible iterator types are, e.g., const-iterators for host or mapped arrays, or pointers to C-arrays.
		 * @param[in]     first      is an iterator pointing to the first element in the subarray to be loaded.
		 * @param[in]     last       is an iterator pointing to the one-past-last element in the subarray to be loaded.
		 * @param[in]     firstDev   is an iterator pointing to the starting position, where the subarray is stored in this device array.
		 * @param[in,out] eventLoad  if not 0, returns an event object that identifies the write command.
		 */
		template<class _ITER>
		void load(iterator first, iterator last, _ITER firstSrc, cl::Event *eventLoad = 0)
		{
			m_devCon->getCommandQueue().enqueueWriteBuffer(m_buffer, CL_FALSE,
				first.m_index * sizeof(T), (last-first) * sizeof(T), &*firstSrc, 0, eventLoad);
		}

		//@}

		/** @name Transfering Data from Device to Host Memory
		 *  These methods store data on the device in host memory. They enqueue read- and write-buffer
		 *  commands to the command queue for the associated device. The methods ending with <em>Blocking</em>
		 *  only return once the memory transfer is complete; the other methods just enqueue a command
		 *  for transfering the data (an event object can be used to wait for completion).
		 */
		//@{

		//! Stores the data on the device in C-array \a ptr.
		/**
		 * This method enqueues a blocking read-buffer command. Once this method returns, the memory
		 * transfer to the host is completed.
		 *
		 * @param[in,out] ptr  must point to an allocated region of memory that is large enough to hold the whole array.
		 */
		void storeBlocking(T *ptr) {
			m_devCon->getCommandQueue().enqueueReadBuffer(m_buffer, CL_TRUE, 0, m_nElements*sizeof(T), ptr);
		}

		//! Stores the data on the device in host array \a ha.
		/**
		 * This method enqueues a blocking read-buffer command. Once this method returns, the memory
		 * transfer to the host is completed.
		 *
		 * @param[in,out] ha  must be a host array that is large enough to hold the whole array.
		 */
		void storeBlocking(HostArray<T> &ha) {
			m_devCon->getCommandQueue().enqueueReadBuffer(m_buffer, CL_TRUE, 0, m_nElements*sizeof(T), &ha[0]);
		}

		//! Stores the data on the device in mapped array \a ma.
		/**
		 * This method enqueues a blocking read-buffer command. Once this method returns, the memory
		 * transfer to the host is completed.
		 *
		 * @param[in,out] ma  must be a mapped array that is large enough to hold the whole array.
		 */
		void storeBlocking(MappedArray<T> &ma);

		//! Enqueues a command for storing the data on the device in C-array \a ptr.
		/**
		 * This method enqueues a non-blocking read-buffer command and returns an event object associated with
		 * this read command that can be queried (or waited for).
		 *
		 * @param[in,out] ptr          must point to an allocated region of memory that is large enough to hold the whole array.
		 * @param[in,out] eventStore   if not 0, returns an event object that identifies the read command.
		 */
		void store(T *ptr, cl::Event *eventStore = 0) {
			m_devCon->getCommandQueue().enqueueReadBuffer(m_buffer, CL_FALSE, 0, m_nElements*sizeof(T), ptr, 0, eventStore);
		}

		//! Enqueues a command for storing the data on the device in host array \a ha.
		/**
		 * This method enqueues a non-blocking read-buffer command and returns an event object associated with
		 * this read command that can be queried (or waited for).
		 *
		 * @param[in,out] ha           must be a host array that is large enough to hold the whole array.
		 * @param[in,out] eventStore   if not 0, returns an event object that identifies the read command.
		 */
		void store(HostArray<T> &ha, cl::Event *eventStore = 0) {
			m_devCon->getCommandQueue().enqueueReadBuffer(m_buffer, CL_FALSE, 0, m_nElements*sizeof(T), &ha[0], 0, eventStore);
		}

		//! Enqueues a command for storing the data on the device in mapped array \a ma.
		/**
		 * This method enqueues a non-blocking read-buffer command and returns an event object associated with
		 * this read command that can be queried (or waited for).
		 *
		 * @param[in,out] ma           must be a mapped array that is large enough to hold the whole array.
		 * @param[in,out] eventStore   if not 0, returns an event object that identifies the read command.
		 */
		void store(MappedArray<T> &ma, cl::Event *eventStore = 0);

		//@}
	};


	//! Const-iterator for device arrays.
	/**
	 * \ingroup iterators
	 */
	template<class T>
	class _DeviceArrayConstIterator
	{
		friend class DeviceArray<T>;

		index_t               m_index;     //!< the index in the device array.
		const DeviceArray<T> *m_devArray;  //!< the device array this iterator points to.

	protected:

		//! Constructs a const-iterator pointing to \a index in \a devArray.
		/**
		 * This constructor can only be called from class DeviceArray.
		 */
		_DeviceArrayConstIterator(index_t index, const DeviceArray<T> *devArray) : m_index(index), m_devArray(devArray) { }

		//! Returns the index of the position in the device array this iterator points to.
		index_t getIndex() const { return m_index; }

	public:
		//! Shorthand for this iterator type.
		typedef _DeviceArrayConstIterator<T> _MyIter;

		//! The type for iterator differences.
		typedef typename DeviceArray<T>::difference_type difference_type;


		/** @name Construction and Assignment
		 * These constructors just provide a default constructor (creating an invalid iterator)
		 * and copy constructors. To create an iterator pointing to a specific position in a
		 * device array, use the methods provided by DeviceArray.
		 */
		//@{

		//! Constructs an invalid const-iterator.
		_DeviceArrayConstIterator() : m_index(0), m_devArray(0) { }

		//! Copy constructor. Constructs a const-iterator pointing to the same position as \a iter.
		_DeviceArrayConstIterator(const _MyIter &iter) : m_index(iter.m_index), m_devArray(iter.m_devArray) { }

		//! Constructs a const-iterator pointing to the same position as iterator \a iter.
		_DeviceArrayConstIterator(const _DeviceArrayIterator<T> &iter) : m_index(iter.m_index), m_devArray(iter.m_devArray) { }

		//! Assignment operator.
		_MyIter &operator=(const _MyIter &iter) {
			m_index    = iter.m_index;
			m_devArray = iter.m_devArray;
			return *this;
		}

		//@}


		/** @name Information Methods
		 */
		//@{

		//! Returns the device array this iterator points to.
		/**
		 * Returns 0 if this is an invalid const-iterator.
		 */
		const DeviceArray<T> *getDeviceArray() const { return m_devArray; }

		//@}


		/** @name Operators for Moving Iterators
		 * These methods move an iterator forward or backward, or create a new iterator that points to a position
		 * which is a specific number of elements before or after.
		 */
		//@{

		//! Moves iterator one position forward (pre-increment).
		/**
		 * \pre This const-iterator must be valid.
		 */
		_MyIter &operator++() {
			++m_index;
			return *this;
		}

		//! Moves iterator one position forward (post-increment).
		/**
		 * \pre This const-iterator must be valid.
		 */
		_MyIter operator++(int) {
			_MyIter iter(*this);
			++m_index;
			return iter;
		}

		//! Moves iterator one position backward (pre-increment).
		/**
		 * \pre This const-iterator must be valid.
		 */
		_MyIter &operator--() {
			--m_index;
			return *this;
		}

		//! Moves iterator one position backward (post-increment).
		/**
		 * \pre This const-iterator must be valid.
		 */
		_MyIter operator--(int) {
			_DeviceArrayConstIterator<T> iter(*this);
			--m_index;
			return iter;
		}

		//! Moves iterator \a offset positions forward.
		/**
		 * \pre This const-iterator must be valid.
		 */
		_MyIter &operator+=(difference_type offset) {
			m_index += offset;
			return *this;
		}

		//! Returns an iterator pointing to a position \a offset positions forward of this iterator.
		/**
		 * \pre This const-iterator must be valid.
		 */
		_MyIter operator+(difference_type offset) const {
			_DeviceArrayConstIterator<T> iter(*this);
			return ( iter += offset);
		}

		//! Moves iterator \a offset positions backward.
		/**
		 * \pre This const-iterator must be valid.
		 */
		_MyIter &operator-=(difference_type offset) {
			m_index -= offset;
			return *this;
		}

		//! Returns an iterator pointing to a position \a offset positions backward of this iterator.
		/**
		 * \pre This const-iterator must be valid.
		 */
		_MyIter operator-(difference_type offset) const {
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
		difference_type operator-(const _MyIter &iter) const {
			return m_index - iter.m_index;
		}

		//@}


		/** @name Operators for Comparing Iterators
		 * These operators compare iterators for equality / inequality, or for the relative position
		 * of the elements they point to. The latter compare operators can only be called if the
		 * two iterators to be compared are both valid and point to the same device array.
		 */
		//@{

		//! Returns true if \a right points to the same position as this iterator; false otherwise.
		bool operator==(const _MyIter &right) const {
			return ( m_index == right.m_index && m_devArray == right.m_devArray );
		}

		//! Returns true if \a right points to a different position as this iterator; false otherwise.
		bool operator!=(const _MyIter &right) const {
			return ( m_index != right.m_index || m_devArray != right.m_devArray );
		}

		//! Returns true if this iterator points to a position before the position \a right points to.
		/**
		 * \pre \a both right and this iterator must be valid and point to the same array.
		 */
		bool operator<(const _MyIter &right) const {
			return ( m_index < right.m_index );
		}

		//! Returns true if this iterator points to the same position or a position before the position \a right points to.
		/**
		 * \pre \a both right and this iterator must be valid and point to the same array.
		 */
		bool operator<=(const _MyIter &right) const {
			return ( m_index <= right.m_index );
		}

		//! Returns true if this iterator points to a position after the position \a right points to.
		/**
		 * \pre \a both right and this iterator must be valid and point to the same array.
		 */
		bool operator>(const _MyIter &right) const {
			return ( m_index > right.m_index );
		}

		//! Returns true if this iterator points to the same position or a position after the position \a right points to.
		/**
		 * \pre \a both right and this iterator must be valid and point to the same array.
		 */
		bool operator>=(const _MyIter &right) const {
			return ( m_index >= right.m_index );
		}

		//@}
	};


	//! Iterator for device arrays.
	/**
	 * \ingroup iterators
	 */
	template<class T>
	class _DeviceArrayIterator
	{
		friend class DeviceArray<T>;
		friend class _DeviceArrayConstIterator<T>;

		index_t         m_index;     //!< the index in the device array.
		DeviceArray<T> *m_devArray;  //!< the device array this iterator points to.

	protected:

		//! Constructs an iterator pointing to \a index in \a devArray.
		/**
		 * This constructor can only be called from class DeviceArray.
		 */
		_DeviceArrayIterator(index_t index, DeviceArray<T> *devArray) : m_index(index), m_devArray(devArray) { }

	public:
		//! Shorthand for this iterator type.
		typedef _DeviceArrayIterator<T> _MyIter;

		//! The type for iterator differences.
		typedef typename DeviceArray<T>::difference_type difference_type;


		/** @name Construction and Assignment
		 * These constructors just provide a default constructor (creating an invalid iterator)
		 * and copy constructors. To create an iterator pointing to a specific position in a
		 * device array, use the methods provided by DeviceArray.
		 */
		//@{

		//! Constructs an invalid iterator.
		_DeviceArrayIterator() : m_index(0), m_devArray(0) { }

		//! Copy constructor. Constructs an iterator pointing to the same position as \a iter.
		_DeviceArrayIterator(const _MyIter &iter) : m_index(iter.m_index), m_devArray(iter.m_devArray) { }

		//! Assignment operator.
		_MyIter &operator=(const _MyIter &iter) {
			m_index    = iter.m_index;
			m_devArray = iter.m_devArray;
			return *this;
		}

		//@}

		/** @name Information Methods
		 */
		//@{

		//! Returns the device array this iterator points to.
		/**
		 * Returns 0 if this is an invalid iterator.
		 */
		DeviceArray<T> *getDeviceArray() { return m_devArray; }

		//@}


		/** @name Operators for Moving Iterators
		 * These methods move an iterator forward or backward, or create a new iterator that points to a position
		 * which is a specific number of elements before or after.
		 */
		//@{

		//! Moves iterator one position forward (pre-increment).
		/**
		 * \pre This iterator must be valid.
		 */
		_MyIter &operator++() {
			++m_index;
			return *this;
		}

		//! Moves iterator one position forward (post-increment).
		/**
		 * \pre This iterator must be valid.
		 */
		_MyIter operator++(int) {
			_MyIter iter(*this);
			++m_index;
			return iter;
		}

		//! Moves iterator one position backward (pre-increment).
		/**
		 * \pre This iterator must be valid.
		 */
		_MyIter &operator--() {
			--m_index;
			return *this;
		}

		//! Moves iterator one position backward (post-increment).
		/**
		 * \pre This iterator must be valid.
		 */
		_MyIter operator--(int) {
			_MyIter iter(*this);
			--m_index;
			return iter;
		}

		//! Moves iterator \a offset positions forward.
		/**
		 * \pre This iterator must be valid.
		 */
		_MyIter &operator+=(difference_type offset) {
			m_index += offset;
			return *this;
		}

		//! Returns an iterator pointing to a position \a offset positions forward of this iterator.
		/**
		 * \pre This iterator must be valid.
		 */
		_MyIter operator+(difference_type offset) const {
			_MyIter iter(*this);
			return ( iter += offset );
		}

		//! Moves iterator \a offset positions backward.
		/**
		 * \pre This iterator must be valid.
		 */
		_MyIter &operator-=(difference_type offset) {
			m_index -= offset;
			return *this;
		}

		//! Returns an iterator pointing to a position \a offset positions backward of this iterator.
		/**
		 * \pre This iterator must be valid.
		 */
		_MyIter operator-(difference_type offset) const {
			_MyIter iter(*this);
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
		difference_type operator-(const _MyIter &iter) const {
			return m_index - iter.m_index;
		}

		//@}

		/** @name Operators for Comparing Iterators
		 * These operators compare iterators for equality / inequality, or for the relative position
		 * of the elements they point to. The latter compare operators can only be called if the
		 * two iterators to be compared are both valid and point to the same device array.
		 */
		//@{

		//! Returns true if \a right points to the same position as this iterator; false otherwise.
		bool operator==(const _MyIter &right) const {
			return ( m_index == right.m_index && m_devArray == right.m_devArray );
		}

		//! Returns true if \a right points to a different position as this iterator; false otherwise.
		bool operator!=(const _MyIter &right) const {
			return ( m_index != right.m_index || m_devArray != right.m_devArray );
		}

		//! Returns true if this iterator points to a position before the position \a right points to.
		/**
		 * \pre \a both right and this iterator must be valid and point to the same array.
		 */
		bool operator<(const _MyIter &right) const {
			return ( m_index < right.m_index );
		}

		//! Returns true if this iterator points to the same position or a position before the position \a right points to.
		/**
		 * \pre \a both right and this iterator must be valid and point to the same array.
		 */
		bool operator<=(const _MyIter &right) const {
			return ( m_index <= right.m_index );
		}

		//! Returns true if this iterator points to a position after the position \a right points to.
		/**
		 * \pre \a both right and this iterator must be valid and point to the same array.
		 */
		bool operator>(const _MyIter &right) const {
			return ( m_index > right.m_index );
		}

		//! Returns true if this iterator points to the same position or a position after the position \a right points to.
		/**
		 * \pre \a both right and this iterator must be valid and point to the same array.
		 */
		bool operator>=(const _MyIter &right) const {
			return ( m_index >= right.m_index );
		}

		//@}

	};


	template<class T> inline
	typename DeviceArray<T>::iterator DeviceArray<T>::begin() { return iterator(0,this); }

	template<class T> inline
	typename DeviceArray<T>::const_iterator DeviceArray<T>::begin() const { return const_iterator(0,this); }

	template<class T> inline
	typename DeviceArray<T>::iterator DeviceArray<T>::end() { return iterator(m_nElements,this); }

	template<class T> inline
	typename DeviceArray<T>::const_iterator DeviceArray<T>::end() const { return const_iterator(m_nElements,this); }

	template<class T> inline
	typename DeviceArray<T>::iterator DeviceArray<T>::rbegin() { return iterator(m_nElements-1,this); }

	template<class T> inline
	typename DeviceArray<T>::const_iterator DeviceArray<T>::rbegin() const { return const_iterator(m_nElements-1,this); }

	template<class T> inline
	typename DeviceArray<T>::iterator DeviceArray<T>::rend() { return iterator(-1,this); }

	template<class T> inline
	typename DeviceArray<T>::const_iterator DeviceArray<T>::rend() const { return const_iterator(-1,this); }

	template<class T> inline
	typename DeviceArray<T>::iterator DeviceArray<T>::at(index_t i) { return iterator(i,this); }

	template<class T> inline
	typename DeviceArray<T>::const_iterator DeviceArray<T>::at(index_t i) const { return const_iterator(i,this); }

}

#include <tbt/MappedArray.h>

namespace tbt {

	template<class T> inline
	void DeviceArray<T>::loadBlocking(const MappedArray<T> &ma)
	{
		m_devCon->getCommandQueue().enqueueWriteBuffer(m_buffer, CL_TRUE, 0, m_nElements*sizeof(T), &ma[0]);
	}

	template<class T> inline
	void DeviceArray<T>::load(const MappedArray<T> &ma, cl::Event *eventLoad)
	{
		m_devCon->getCommandQueue().enqueueWriteBuffer(m_buffer, CL_FALSE, 0, m_nElements*sizeof(T), &ma[0], 0, eventLoad);
	}

	template<class T> inline
	void DeviceArray<T>::storeBlocking(MappedArray<T> &ma)
	{
		m_devCon->getCommandQueue().enqueueReadBuffer(m_buffer, CL_TRUE, 0, m_nElements*sizeof(T), &ma[0]);
	}

	template<class T> inline
	void DeviceArray<T>::store(MappedArray<T> &ma, cl::Event *eventStore = 0)
	{
		m_devCon->getCommandQueue().enqueueReadBuffer(m_buffer, CL_FALSE, 0, m_nElements*sizeof(T), &ma[0], 0, eventStore);
	}

}

#endif
