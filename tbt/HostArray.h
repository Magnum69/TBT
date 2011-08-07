
#ifndef _TBT_HOST_ARRAY_H
#define _TBT_HOST_ARRAY_H

#include <tbt/tbthc.h>


namespace tbt
{

	template<class T> class _HostArrayConstIterator;
	template<class T> class _HostArrayIterator;

	//! Array stored on the host.
	/**
	 * This class implements generic arrays with auomatic allocation and deallocation of memory,
	 * supporting random access to the elements using the array-operator []. It is also possible
	 * construct arrays for 0 elements; in this case no memory is allocated. The usual copy
	 * constructors and assignment operators are supported, which copy the elements to the
	 * newly created array.
	 */
	template<class T>
	class HostArray
	{
		T     *m_ptr;        //!< pointer to memory allocated for array.
		size_t m_nElements;  //!< number of elements in array.

	public:
		//! The type for iterator differences.
		typedef index_t difference_type;

		//! Const-iterator for device arrays.
		typedef _HostArrayConstIterator<T> const_iterator;

		//! Iterator for device arrays.
		typedef _HostArrayIterator<T> iterator;

		//! Const-reference type for mapped arrays.
		typedef const T &const_reference;

		//! Reference type for mapped arrays.
		typedef T &reference;


		//! Constructs an empty host array (for 0 elements).
		HostArray() : m_ptr(0), m_nElements(0) { }

		//! Copy constructor. Allocates a new array and copies all elements from \a b.
		HostArray(const HostArray<T> &b) : m_nElements(b.m_nElements) {
			m_ptr = (m_nElements > 0) ? new T [m_nElements] : 0;
			copy(b.m_ptr);
		}

		//! Constructs a host array for \a n elements of type \a T.
		HostArray(size_t n) : m_nElements(n) {
			m_ptr = (n > 0) ? new T [n] : 0;
		}

		//! Destructor. Releases allocated memory.
		~HostArray() {
			delete [] m_ptr;
		}

		//! Returns a reference to the <i>i</i>-th element in the array.
		const T &operator[](size_t i) const {
			return m_ptr[i];
		}

		//! Returns a reference to the <i>i</i>-th element in the array.
		T &operator[](size_t i) {
			return m_ptr[i];
		}

		//! Returns the number of elements in the array.
		size_t size() const {
			return m_nElements;
		}


		/** @name Iterators
		 * These methods return iterators to specific positions in the host array.
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


		//! Assignment operator. Copies the elements from b.
		HostArray<T> &operator=(const HostArray<T> &b) {
			if(b.m_nElements != m_nElements) {
				delete [] m_ptr;
				m_nElements = b.m_nElements;
				m_ptr = (m_nElements > 0) ? new T [m_nElements] : 0;
			}
			copy(b);
			return *this;
		}

		//! Assigns x to every element in the array.
		void fill(T x) {
			for(size_t i = 0; i < m_nElements; ++i)
				m_ptr[i] = x;
		}

		//! Resizes the array to an array for \a n elements.
		/**
		 * The array can be enlarged or shrunken; it is also possible to resize it to 0 elements.
		 * If the old array had \a n_old elements, then min(\a n_old, \a n) elements are copied from
		 * the old to the resized array.
		 */
		void resize(size_t n) {
			if(n != m_nElements) {
				if(n > 0) {
					T *ptr = new T [n];
					size_t n_min = min(n,m_nElements);
					for(size_t i = 0; i < n_min; i++)
						ptr[i] = m_ptr[i];

					delete m_ptr;
					m_ptr = ptr;

				} else {
					delete [] m_ptr;
					m_ptr = 0;
				}

				m_nElements = n;
			}
		}

	private:
		//! Auxiliary function for copying elements.
		void copy(T *b) {
			for(size_t i = 0; i < m_nElements; ++i)
				m_ptr[i] = b[i];
		}
	};


	//! Const-iterator for host arrays.
	template<class T>
	class _HostArrayConstIterator
	{
		friend class HostArray<T>;

		T *m_ptr;  //!< pointer to the element in the host array.

		//! Constructs a const-iterator pointing to the element \a ptr points to.
		/**
		 * This constructor can only be called from class HostArray.
		 */
		_HostArrayConstIterator(const T *ptr) : m_ptr(ptr) { }

	public:
		//! Shorthand for this iterator type.
		typedef _HostArrayConstIterator<T> _MyIter;

		//! The type for iterator differences.
		typedef typename HostArray<T>::difference_type difference_type;

		//! The reference type for this iterator.
		typedef typename HostArray<T>::const_reference reference;

		/** @name Construction and Assignment
		 * These constructors just provide a default constructor (creating an invalid iterator)
		 * and copy constructors. To create an iterator pointing to a specific position in a
		 * host array, use the methods provided by HostArray.
		 */
		//@{

		//! Constructs an invalid const-iterator.
		_HostArrayConstIterator() : m_ptr(0) { }

		//! Copy constructor. Constructs a const-iterator pointing to the same position as \a iter.
		_HostArrayConstIterator(const _MyIter &iter) : m_ptr(iter.m_ptr) { }

		//! Constructs a const-iterator pointing to the same position as iterator \a iter.
		_HostArrayConstIterator(const _HostArrayIterator<T> &iter) : m_ptr(iter.m_ptr) { }

		//! Assignment operator.
		_MyIter &operator=(const _MyIter &iter) {
			m_ptr = iter.m_ptr;
			return *this;
		}

		//@}


		/** @name Access Methods
		 * These methods provide access to the element this iterator points to.
		 */
		//@{

		//! Returns a reference to the element in the host array this iterator points to.
		reference operator*() const {
			return *m_ptr;
		}

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
			++m_ptr;
			return *this;
		}

		//! Moves iterator one position forward (post-increment).
		/**
		 * \pre This const-iterator must be valid.
		 */
		_MyIter operator++(int) {
			_MyIter iter(*this);
			++m_ptr;
			return iter;
		}

		//! Moves iterator one position backward (pre-increment).
		/**
		 * \pre This const-iterator must be valid.
		 */
		_MyIter &operator--() {
			--m_ptr;
			return *this;
		}

		//! Moves iterator one position backward (post-increment).
		/**
		 * \pre This const-iterator must be valid.
		 */
		_MyIter operator--(int) {
			_DeviceArrayConstIterator<T> iter(*this);
			--m_ptr;
			return iter;
		}

		//! Moves iterator \a offset positions forward.
		/**
		 * \pre This const-iterator must be valid.
		 */
		_MyIter &operator+=(difference_type offset) {
			m_ptr += offset;
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
			m_ptr -= offset;
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
			return m_ptr - iter.m_ptr;
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
			return ( m_ptr == right.m_ptr );
		}

		//! Returns true if \a right points to a different position as this iterator; false otherwise.
		bool operator!=(const _MyIter &right) const {
			return ( m_ptr != right.m_ptr );
		}

		//! Returns true if this iterator points to a position before the position \a right points to.
		/**
		 * \pre \a both right and this iterator must be valid and point to the same array.
		 */
		bool operator<(const _MyIter &right) const {
			return ( m_ptr < right.m_ptr );
		}

		//! Returns true if this iterator points to the same position or a position before the position \a right points to.
		/**
		 * \pre \a both right and this iterator must be valid and point to the same array.
		 */
		bool operator<=(const _MyIter &right) const {
			return ( m_ptr <= right.m_ptr );
		}

		//! Returns true if this iterator points to a position after the position \a right points to.
		/**
		 * \pre \a both right and this iterator must be valid and point to the same array.
		 */
		bool operator>(const _MyIter &right) const {
			return ( m_ptr > right.m_ptr );
		}

		//! Returns true if this iterator points to the same position or a position after the position \a right points to.
		/**
		 * \pre \a both right and this iterator must be valid and point to the same array.
		 */
		bool operator>=(const _MyIter &right) const {
			return ( m_ptr >= right.m_ptr );
		}

		//@}
	};


	//! Const-iterator for host arrays.
	template<class T>
	class _HostArrayIterator
	{
		friend class HostArray<T>;
		friend class _HostArrayConstIterator<T>;

		T *m_ptr;  //!< pointer to the element in the host array.

		//! Constructs a const-iterator pointing to the element \a ptr points to.
		/**
		 * This constructor can only be called from class HostArray.
		 */
		_HostArrayIterator(T *ptr) : m_ptr(ptr) { }

	public:
		//! Shorthand for this iterator type.
		typedef _HostArrayIterator<T> _MyIter;

		//! The type for iterator differences.
		typedef typename HostArray<T>::difference_type difference_type;

		//! The reference type for this iterator.
		typedef typename HostArray<T>::reference reference;

		/** @name Construction and Assignment
		 * These constructors just provide a default constructor (creating an invalid iterator)
		 * and copy constructors. To create an iterator pointing to a specific position in a
		 * device array, use the methods provided by DeviceArray.
		 */
		//@{

		//! Constructs an invalid const-iterator.
		_HostArrayIterator() : m_ptr(0) { }

		//! Copy constructor. Constructs a const-iterator pointing to the same position as \a iter.
		_HostArrayIterator(const _MyIter &iter) : m_ptr(iter.m_ptr) { }

		//! Assignment operator.
		_MyIter &operator=(const _MyIter &iter) {
			m_ptr = iter.m_ptr;
			return *this;
		}

		//@}


		/** @name Access Methods
		 * These methods provide access to the element this iterator points to.
		 */
		//@{

		//! Returns a reference to the element in the host array this iterator points to.
		reference operator*() const {
			return *m_ptr;
		}

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
			++m_ptr;
			return *this;
		}

		//! Moves iterator one position forward (post-increment).
		/**
		 * \pre This const-iterator must be valid.
		 */
		_MyIter operator++(int) {
			_MyIter iter(*this);
			++m_ptr;
			return iter;
		}

		//! Moves iterator one position backward (pre-increment).
		/**
		 * \pre This const-iterator must be valid.
		 */
		_MyIter &operator--() {
			--m_ptr;
			return *this;
		}

		//! Moves iterator one position backward (post-increment).
		/**
		 * \pre This const-iterator must be valid.
		 */
		_MyIter operator--(int) {
			_DeviceArrayConstIterator<T> iter(*this);
			--m_ptr;
			return iter;
		}

		//! Moves iterator \a offset positions forward.
		/**
		 * \pre This const-iterator must be valid.
		 */
		_MyIter &operator+=(difference_type offset) {
			m_ptr += offset;
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
			m_ptr -= offset;
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
			return m_ptr - iter.m_ptr;
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
			return ( m_ptr == right.m_ptr );
		}

		//! Returns true if \a right points to a different position as this iterator; false otherwise.
		bool operator!=(const _MyIter &right) const {
			return ( m_ptr != right.m_ptr );
		}

		//! Returns true if this iterator points to a position before the position \a right points to.
		/**
		 * \pre \a both right and this iterator must be valid and point to the same array.
		 */
		bool operator<(const _MyIter &right) const {
			return ( m_ptr < right.m_ptr );
		}

		//! Returns true if this iterator points to the same position or a position before the position \a right points to.
		/**
		 * \pre \a both right and this iterator must be valid and point to the same array.
		 */
		bool operator<=(const _MyIter &right) const {
			return ( m_ptr <= right.m_ptr );
		}

		//! Returns true if this iterator points to a position after the position \a right points to.
		/**
		 * \pre \a both right and this iterator must be valid and point to the same array.
		 */
		bool operator>(const _MyIter &right) const {
			return ( m_ptr > right.m_ptr );
		}

		//! Returns true if this iterator points to the same position or a position after the position \a right points to.
		/**
		 * \pre \a both right and this iterator must be valid and point to the same array.
		 */
		bool operator>=(const _MyIter &right) const {
			return ( m_ptr >= right.m_ptr );
		}

		//@}
	};


	template<class T> inline
	typename HostArray<T>::iterator HostArray<T>::begin() { return iterator(m_ptr); }

	template<class T> inline
	typename HostArray<T>::const_iterator HostArray<T>::begin() const { return const_iterator(m_ptr); }

	template<class T> inline
	typename HostArray<T>::iterator HostArray<T>::end() { return iterator(m_ptr+m_nElements); }

	template<class T> inline
	typename HostArray<T>::const_iterator HostArray<T>::end() const { return const_iterator(m_ptr+m_nElements); }

	template<class T> inline
	typename HostArray<T>::iterator HostArray<T>::rbegin() { return iterator(m_ptr+m_nElements-1); }

	template<class T> inline
	typename HostArray<T>::const_iterator HostArray<T>::rbegin() const { return const_iterator(m_ptr+m_nElements-1); }

	template<class T> inline
	typename HostArray<T>::iterator HostArray<T>::rend() { return iterator(m_ptr-1); }

	template<class T> inline
	typename HostArray<T>::const_iterator HostArray<T>::rend() const { return const_iterator(m_ptr-1); }

	template<class T> inline
	typename HostArray<T>::iterator HostArray<T>::at(index_t i) { return iterator(m_ptr+i); }

	template<class T> inline
	typename HostArray<T>::const_iterator HostArray<T>::at(index_t i) const { return const_iterator(m_ptr+i); }

}

#endif
