
#include <iostream>
#include <tbt/RadixSort.h>
#include <tbt/MappedArray.h>

using namespace std;


int main()
{
	const size_t C = 512;
	const size_t m = 256 * 4;
	const size_t n = C*m;

	tbt::createContext(CL_DEVICE_TYPE_CPU, CL_QUEUE_PROFILING_ENABLE);
	cout << "Platform:" << endl;
	tbt::displayPlatformInfo(cout) << endl;

	cout << "Device:" << endl;
	tbt::DeviceController *devCon = tbt::getDeviceController();
	devCon->displayInfo() << endl;

	cout << "n = " << n << endl;
	cout << "C = " << C << endl;
	cout << "m = " << m << endl;

	tbt::RadixSort radixSort;
	tbt::MappedArray<cl_uint> a(devCon, n);

	for(size_t i = 0; i < n; i++)
		a[i] = 1; //(rand() % 500);
	a.mapHostToDevice();

	tbt::MappedArray<cl_uint> sum(devCon, C);

	double t = radixSort.testKernelPrescanReduce(a, sum, n, C);
	sum.mapDeviceToHost();

	cout << "kernel time: " << t << " ms" << endl;

	for(size_t i = 0; i < C; i++) {
		cl_uint s = 0;
		for(size_t j = 0; j < m; j++) {
			s += a[m*i+j];
		}
		if(s != sum[i])
			cout << i << ": ERROR: " << s << " != " << sum[i] << endl;
	}

	return 0;
}
