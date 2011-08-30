
#include <iostream>
#include <tbt/RadixSort.h>
#include <tbt/MappedArray.h>

using namespace std;


int main()
{
	const size_t C = 4*64; //512;
	const size_t m = 256 * 128;
	const size_t n = C*m;
	const int numIter = 1;

	try {

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
		tbt::HostArray<cl_uint> prescan(C);

		cl_uint s = 0;
		for(size_t i = 0; i < n; i++) {
			if(i % m == 0) {
				prescan[i / m] = s;
				//s = 0;
			}
			a[i] = (rand() % 50);
			s += a[i];
		}

		a.mapHostToDeviceBlocking();

		tbt::MappedArray<cl_uint> sum(devCon, C);
		radixSort.testKernelTester(a, sum, n, C);

		double tRed = 0, tLoc = 0, tBot = 0;
		for(int i = 0; i < numIter; i++) {
			double t1 = radixSort.testKernelPrescanReduce(a, sum, n, C);
			double t2 = radixSort.testKernelPrescanLocal(sum, C);
			//double t3 = radixSort.testKernelPrescanBottom(a, sum, n, C);
			tRed += t1;
			tLoc += t2;
			//tBot += t3;
			//cout << t1 << ", " << t2 << ", " << t3 << endl;
		}
		tRed = tRed / numIter;
		tLoc = tLoc / numIter;
		tBot = tBot / numIter;

		sum.mapDeviceToHostBlocking();
		a.mapDeviceToHostBlocking();

		double gb = (n+C)*sizeof(cl_uint) / (1024.0*1024.0*1024.0);
		double bw = 1000.0 * gb / tRed;

		cout << "Reduce:" << endl;
		cout << "kernel time: " << tRed << " ms" << endl;
		cout << "data:        " << gb << " GB" << endl;
		cout << "bandwidth:   " << bw << " GB/s" << endl;

		gb = 2*C*sizeof(cl_uint) / (1024.0*1024.0*1024.0);
		bw = 1000.0 * gb / tLoc;

		cout << "\nPrescanLocal:" << endl;
		cout << "kernel time: " << tLoc << " ms" << endl;
		cout << "data:        " << gb << " GB" << endl;
		cout << "bandwidth:   " << bw << " GB/s" << endl;

		//gb = (2*n+C)*sizeof(cl_uint) / (1024.0*1024.0*1024.0);
		//bw = 1000.0 * gb / tBot;

		//cout << "\nPrescanBottom:" << endl;
		//cout << "kernel time: " << tBot << " ms" << endl;
		//cout << "data:        " << gb << " GB" << endl;
		//cout << "bandwidth:   " << bw << " GB/s" << endl;

		//gb = (3*n+4*C)*sizeof(cl_uint) / (1024.0*1024.0*1024.0);
		//bw = 1000.0 * gb / (tRed+tLoc+tBot);

		//cout << "\ntotal:" << endl;
		//cout << "kernel time: " << (tRed+tLoc+tBot) << " ms" << endl;
		//cout << "data:        " << gb << " GB" << endl;
		//cout << "bandwidth:   " << bw << " GB/s" << endl;

		cout << "\nchecking..." << flush;

		//int numErrors = 0;
		//s = 0;
		//for(size_t i = 0; i < n; i++) {
		//	cl_uint x = a[i];
		//	if(a[i] != s)
		//		numErrors++;
		//	s += a[i];
		//}

		int numErrors = 0;
		for(size_t i = 0; i < C; i++) {
			cl_uint x = sum[i];
			cl_uint p = prescan[i];
			if(prescan[i] != sum[i]) {
				numErrors++;
				//cout << i << ": ERROR: " << prescan[i] << " != " << sum[i] << endl;
			}
		}

		if(numErrors == 0)
			cout << "ok." << endl;
		else
			cout << " " << numErrors << " ERRORS!" << endl;

	} catch(tbt::Error err) {
		cerr << "TBT ERROR: " << err.code() << endl;
		cerr << err.what() << endl;
	} catch(cl::Error err) {
		cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << endl;
	}

	return 0;
}
