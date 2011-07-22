

#include <tbt/RadixSort.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>

using namespace std;


void initRandom(cl_uint *a, int n)
{
#ifdef _WIN32
	HCRYPTPROV hCryptProv = NULL;
	CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
	CryptGenRandom(hCryptProv, n*sizeof(unsigned int), (BYTE*)a);
	CryptReleaseContext(hCryptProv,0);

#else
	for(int i = 0; i < n; ++i) {
		a[i] = rand();
	}
#endif
}


//bool getPlatform(cl_device_type deviceType, cl::Platform &platform)
//{
//	try {
//		// Get a AMD/GPU and Intel/CPU platform and context.
//		cl::vector< cl::Platform > platformList;
//		cl::Platform::get(&platformList);
//
//		int indexAMD = -1, indexIntel = -1, indexNvidia = -1;
//		for(int i = 0; i < (int)platformList.size(); ++i) {
//			string platformName;
//			platformList[i].getInfo((cl_platform_info)CL_PLATFORM_NAME, &platformName);
//			if(platformName == "AMD Accelerated Parallel Processing")
//				indexAMD = i;
//			else if(platformName == "Intel(R) OpenCL")
//				indexIntel = i;
//			else if(platformName == "NVIDIA CUDA")
//				indexNvidia = i;
//		}
//
//		if(deviceType == CL_DEVICE_TYPE_GPU && indexAMD == -1 && indexNvidia == -1) {
//			cerr << "Could neither find AMD nor Nvidia platform!" << endl;
//			return false;
//		}
//		if(deviceType == CL_DEVICE_TYPE_CPU && indexAMD == -1 && indexIntel == -1) {
//			cerr << "Could neither find AMD nor Intel platform!" << endl;
//			return false;
//		}
//
//		if(deviceType == CL_DEVICE_TYPE_GPU)
//			platform = platformList[(indexAMD != -1) ? indexAMD : indexNvidia];
//		else
//			platform = (indexIntel != -1) ? platformList[indexIntel] : platformList[indexAMD];
//		return true;
//
//	} catch(cl::Error err) {
//		cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << endl;
//		return false;
//	}
//}


void outputArray(cl_uint *a, cl_uint n)
{
	for(cl_uint i = 0; i < n; ++i) {
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

		if(cmd == "-h" || cmd == "-help") {
			cout << "Call with: radix-sort options-list" << endl;
			cout << "\navailable options:" << endl;
			cout << "\n-n #elements\n  specifies the number of elements to be sorted" << endl;
			cout << "\n-d device\n  specifies the device used: CPU, GPU" << endl;
			cout << "\n-quiet\n  generate no output" << endl;
			cout << "\n-verbose\n  generate detailed output" << endl;
			cout << "\n-h, -help\n  display this help" << endl;
			return 0;

		} else if (cmd == "-n") {
			if(++i >= argc) {
				cerr << "Missing argument for option " << cmd << endl;
				return 1;
			}
			n = atoi(argv[i]);

		} else if (cmd == "-d") {
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

		} else if (cmd == "-quiet") {
			outputMode = omQuiet;

		} else if(cmd == "-verbose") {
			outputMode = omVerbose;
		}
	}
	
	if(n % 1024)
		n += (1024 - (n % 1024));
	
	try {
		tbt::createContext(deviceType);
	
		if(outputMode == omVerbose) {
			cout << "Selected platform: " << endl;;
			tbt::displayPlatformInfo();
		}

		cl::Context context = tbt::getContext();
		cl::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
		cl::Device device = devices[0];

		if(outputMode == omVerbose) {
			char cBuffer[1024];
			device.getInfo(CL_DEVICE_NAME, &cBuffer);
			cout << "Selected device:   " << cBuffer << endl;

			cl_uint computeUnits;
			device.getInfo(CL_DEVICE_MAX_COMPUTE_UNITS, &computeUnits);
			cout << "    " << computeUnits << " compute units" << endl;
		}

		cl::CommandQueue queue(context, device, CL_QUEUE_PROFILING_ENABLE);

		RadixSort radixSort;

		if(outputMode == omVerbose) {
			cout << "Creating array with " << n << " random unsigned ints..." << flush;
		}
		cl_uint *a = new cl_uint[n];
		cl_uint *c = new cl_uint[n];

		initRandom(a,n);
		for(cl_uint i = 0; i < n; ++i)
			c[i] = a[i];

		if(outputMode == omVerbose) {
			cout << "done." << endl;
			if(n <= 1024) outputArray(a,n);
		}

		bool ok = radixSort.run(queue, a,n);

		cout << "kernel Counting:            " << radixSort.totalTimeKernelCounting() << " ms" << endl;
		cout << "kernel Prescan Sum:         " << radixSort.totalTimeKernelPrescanSum()  << " ms" << endl;
		cout << "kernel Prescan:             " << radixSort.totalTimeKernelPrescan()  << " ms" << endl;
		cout << "kernel Prescan with Offset: " << radixSort.totalTimeKernelPrescanWithOffset()  << " ms" << endl;
		cout << "kernel Permute:             " << radixSort.totalTimeKernelPermute()  << " ms" << endl;
		cout << "total kernel time:          " << radixSort.totalTimeKernels() << " ms" << endl;

		if(ok) {
			cout << "Checking results..." << flush;
			sort(c,c+n);
			
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
		}

		delete[] a;
		delete [] c;


	} catch(cl::Error err) {
		cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << endl;
	}

	return 0;
}
