

#include <tbt/RadixSort.h>
#include <tbt/MappedArray.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>

using namespace std;


template<class ARRAY>
void initRandom(ARRAY &a)
{
	size_t n = a.size();
#ifdef _WIN32
	HCRYPTPROV hCryptProv = NULL;
	CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
	CryptGenRandom(hCryptProv, (DWORD)n*sizeof(cl_uint), (BYTE*) &a[0]);
	CryptReleaseContext(hCryptProv,0);

#else
	for(size_t i = 0; i < n; ++i) {
		a[i] = rand();
	}
#endif
}


template<class ARRAY>
void outputArray(ARRAY &a)
{
	size_t n = a.size();
	for(size_t i = 0; i < n; ++i) {
		cout << "a[" << setw(4) << i << "] = " << setw(10) << a[i];
		cout << "   " << setw(3) << (a[i] & 0xffu)
			<< " | " << setw(3) << ((a[i] >> 8) & 0xffu)
			<< " | " << setw(3) << ((a[i] >> 16) & 0xffu)
			<< " | " << setw(3) << ((a[i] >> 24) & 0xffu) << endl;
	}

	cout << "\nHistogram for shift = 0:" << endl;

	cl_uint *count = new cl_uint[256];
	for(cl_uint i = 0; i < 256; ++i)
		count[i] = 0;

	for(cl_uint i = 0; i < n; ++i)
		count[a[i] & 0xffu]++;

	for(cl_uint i = 0; i < 256; ++i)
		cout << setw(3) << i << ": " << setw(4) << count[i] << endl;
}


int main(int argc, char *argv[])
{
	cl_device_type deviceType = CL_DEVICE_TYPE_CPU;
	cl_uint n = 1024;
	enum OutputMode { omQuiet, omNormal, omVerbose } outputMode = omNormal;
	
	// parse command line arguments
	for(int i = 1; i < argc; ++i) {
		string cmd = argv[i];

		if(cmd == "-h" || cmd == "--help") {
			cout << "Call with: radix-sort options-list" << endl;
			cout << "\navailable options:" << endl;
			cout << "\n-n #elements\n  specifies the number of elements to be sorted" << endl;
			cout << "\n-d, --device {CPU,GPU}\n  specifies the device used: CPU or GPU" << endl;
			cout << "\n--quiet\n  generate no output" << endl;
			cout << "\n--verbose\n  generate detailed output" << endl;
			cout << "\n-h, --help\n  display this help and exit" << endl;
			return 0;

		} else if (cmd == "-n") {
			if(++i >= argc) {
				cerr << "Missing argument for option " << cmd << endl;
				return 1;
			}
			n = atoi(argv[i]);

		} else if (cmd == "-d" || cmd == "--device") {
			if(++i >= argc) {
				cerr << "Missing argument for option " << cmd << endl;
				return 1;
			}
			string name = argv[i];
			if(name == "CPU" || name == "cpu")
				deviceType = CL_DEVICE_TYPE_CPU;
			else if (name == "GPU" || name == "gpu")
				deviceType = CL_DEVICE_TYPE_GPU;
			else {
				cerr << "Unknown device type " << name << endl;
				return 1;
			}

		} else if (cmd == "--quiet") {
			outputMode = omQuiet;

		} else if(cmd == "--verbose") {
			outputMode = omVerbose;

		} else {
			cerr << "Unknown option: " << cmd << endl;
			return 1;
		}
	}
	
	if(n % 1024)
		n += (1024 - (n % 1024));
	
	try {
		tbt::createContext(deviceType, CL_QUEUE_PROFILING_ENABLE);
	
		if(outputMode == omVerbose) {
			cout << "Selected platform: " << endl;;
			tbt::displayPlatformInfo();
		}

		tbt::DeviceController *devCon = tbt::getDeviceController();

		if(outputMode == omVerbose) {
			cout << "Selected device:   " << devCon->getName() << endl;
			cout << "    " << devCon->getMaxComputeUnits() << " compute units" << endl;
			cout << "    " << devCon->getMemBaseAddrAlign() << " bits address alignment" << endl;
		}

		tbt::RadixSort radixSort;

		if(outputMode == omVerbose) {
			cout << "Creating array with " << n << " random unsigned ints..." << flush;
		}
		//tbt::HostArray<cl_uint> a(n);
		tbt::MappedArray<cl_uint> a(devCon, n);
		initRandom(a);

		tbt::HostArray<cl_uint> c(n);
		for(size_t i = 0; i < n; ++i)
			c[i] = a[i];

		if(outputMode == omVerbose) {
			cout << "done." << endl;
			if(n <= 1024) outputArray(a);
		}

		//tbt::DeviceArray<cl_uint> devArray(devCon, a.size());
		//devArray.loadFrom(a);
		a.mapHostToDevice();
		//radixSort.run(devArray);
		radixSort.run(a);
		//devArray.storeToBlocking(a);
		a.mapDeviceToHostBlocking();

		cout << "kernel Counting:            " << radixSort.totalTimeKernelCounting() << " ms" << endl;
		cout << "kernel Prescan Sum:         " << radixSort.totalTimeKernelPrescanSum()  << " ms" << endl;
		cout << "kernel Prescan:             " << radixSort.totalTimeKernelPrescan()  << " ms" << endl;
		cout << "kernel Prescan with Offset: " << radixSort.totalTimeKernelPrescanWithOffset()  << " ms" << endl;
		cout << "kernel Permute:             " << radixSort.totalTimeKernelPermute()  << " ms" << endl;
		cout << "total kernel time:          " << radixSort.totalTimeKernels() << " ms" << endl;
		cout << endl;
		cout << "total time:                 " << radixSort.totalTime() << " ms" << endl;

		cout << "Checking results..." << flush;
		sort(&c[0],&c[n]);
			
		cl_uint nIncorrect = 0;
		for(cl_uint i = 0; i < n; ++i)
			if(a[i] != c[i]) {
				nIncorrect++;
				//cout << i << ": " << a[i] << " and " << c[i] << endl;
			}
		if(nIncorrect)
			cout << " " << nIncorrect << " elements incorrect!" << endl;
		else
			cout << "ok." << endl;

	} catch(cl::Error err) {
		cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << endl;
	}

	return 0;
}
