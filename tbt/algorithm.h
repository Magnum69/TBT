#ifndef _TBT_ALGORITHM_H
#define _TBT_ALGORITHM_H


#include <tbt/DeviceArray.h>


namespace tbt
{

	/**
	 * \defgroup algorithm Algorithms
	 * \brief These functions implement the algorithms available in TBT. They work on device arrays
	 *        which determine on what device an algorithm is run.
	 */


	//! Sorts a device array with radix-sort.
	/**
	 * @tparam T         is the data type to be sorted. Allowed types are (at the moment) only cl_uint.
	 * @param  devArray  is the device array to be sorted. Radix-sort will be run on the device
	 *                   associated with \a devArray.
	 * \ingroup algorithm
	 */
	template<class T>
	void radixSort(DeviceArray<T> &devArray) {
		throw Error("radixSort: data type of device array not supported", Error::ecDataTypeNotSupported);
	}


	// specializations

	template<>
	void radixSort(DeviceArray<cl_uint> &devArray);
}


#endif
