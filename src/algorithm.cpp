
#include <tbt/algorithm.h>
#include <tbt/RadixSort.h>


namespace tbt {

	template<>
	void radixSort<cl_uint>(DeviceArray<cl_uint> &devArray)
	{
		RadixSort rs;
		rs.run(devArray);
	}

	template<>
	void radixSort<cl_uint>(typename DeviceArray<cl_uint>::iterator first, typename DeviceArray<cl_uint>::iterator last)
	{
		RadixSort rs;
		rs.run(first,last);
	}

}
