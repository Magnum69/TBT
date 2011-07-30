
#ifndef _TBT_HOST_ARRAY_H
#define _TBT_HOST_ARRAY_H

#include <tbt/tbthc.h>


namespace tbt
{

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

}

#endif
