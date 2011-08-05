
#include <tbt/algorithm.h>
#include <tbt/RadixSort.h>


namespace tbt {

	template<>
	void radixSort(DeviceArray<cl_uint> &devArray)
	{
		RadixSort rs;
		rs.run(devArray);
	}

}
