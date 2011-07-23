
#include <tbt/tbthc.h>

#include <iostream>
#include <fstream>
#include <direct.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sstream>
#include <sys/stat.h>


using namespace std;


namespace tbt {

	cl::Platform Global::s_platform;
	cl::Context Global::s_context;

	bool Global::m_cacheProgramBinaries = true;
	bool Global::m_recompileProgramsIfNewerDriver = false;


	cl_device_type getType(const cl::Device &device)
	{
		cl_device_type deviceType;
		device.getInfo(CL_DEVICE_TYPE, &deviceType);
		return deviceType;
	}


	cl_uint getMaxComputeUnits(const cl::Device &device)
	{
		cl_uint nComputeUnits;
		device.getInfo(CL_DEVICE_MAX_COMPUTE_UNITS, &nComputeUnits);
		return nComputeUnits;
	}


	size_t getMaxWorkGroupSize(const cl::Device &device)
	{
		size_t maxWorkGroupSize;
		device.getInfo(CL_DEVICE_MAX_WORK_GROUP_SIZE, &maxWorkGroupSize);
		return maxWorkGroupSize;
	}


	cl_ulong getLocalMemSize(const cl::Device &device)
	{
		cl_ulong localMemSize;
		device.getInfo(CL_DEVICE_LOCAL_MEM_SIZE, &localMemSize);
		return localMemSize;
	}


	cl::Platform getStdPlatform(cl_device_type deviceType)
	{
		// Get a AMD/GPU and Intel/CPU platform and context.
		cl::vector< cl::Platform > platformList;
		cl::Platform::get(&platformList);

		int indexAMD = -1, indexIntel = -1, indexNvidia = -1;
		for(int i = 0; i < (int)platformList.size(); ++i) {
			string platformName;
			platformList[i].getInfo((cl_platform_info)CL_PLATFORM_NAME, &platformName);
			if(platformName == "AMD Accelerated Parallel Processing")
				indexAMD = i;
			else if(platformName == "Intel(R) OpenCL")
				indexIntel = i;
			else if(platformName == "NVIDIA CUDA")
				indexNvidia = i;
		}

		if(deviceType == CL_DEVICE_TYPE_GPU && indexAMD == -1 && indexNvidia == -1)
			throw Error("No OpenCL platform for GPUs found!", Error::ecNoOpenCLPlatformFound);
		
		if(deviceType == CL_DEVICE_TYPE_CPU && indexAMD == -1 && indexIntel == -1)
			throw Error("No OpenCL platform for CPUs found!", Error::ecNoOpenCLPlatformFound);

		if(deviceType == CL_DEVICE_TYPE_GPU)
			return platformList[(indexAMD != -1) ? indexAMD : indexNvidia];
		else
			return (indexIntel != -1) ? platformList[indexIntel] : platformList[indexAMD];
	}


	ostream &displayPlatformInfo(ostream &os)
	{
		cl::Platform platform = getPlatform();

		string platformName;
		platform.getInfo((cl_platform_info)CL_PLATFORM_NAME,    &platformName);
		os << "    name:       " << platformName << endl;

		string platformVendor;
		platform.getInfo((cl_platform_info)CL_PLATFORM_VENDOR,  &platformVendor);
		os << "    vendor:     " << platformVendor << endl;

		string platformVersion;
		platform.getInfo((cl_platform_info)CL_PLATFORM_VERSION, &platformVersion);
		os << "    version:    " << platformVersion << endl;

		string platformProfile;
		platform.getInfo((cl_platform_info)CL_PLATFORM_PROFILE, &platformProfile);
		os << "    profile:    " << platformProfile << endl;

		string platformExtensions;
		platform.getInfo((cl_platform_info)CL_PLATFORM_EXTENSIONS, &platformExtensions);
		os << "    extensions: " << platformExtensions << endl;

		return os;
	}


	void createContext(cl_device_type deviceType, const cl::Platform &platform)
	{
		Global::s_platform = platform;

		cl_context_properties cprops[3] = {
			CL_CONTEXT_PLATFORM,
			(cl_context_properties)(platform)(),
			0
		};

		Global::s_context = cl::Context(deviceType, cprops);
	}


	void createContext(cl_device_type deviceType)
	{
		Global::s_platform = getStdPlatform(deviceType);

		cl_context_properties cprops[3] = {
			CL_CONTEXT_PLATFORM,
			(cl_context_properties)(Global::s_platform)(),
			0
		};

		Global::s_context = cl::Context(deviceType, cprops);
	}

	
	string Utility::toString(cl_uint i)
	{
		stringstream ss;
		ss << i;
		return ss.str();
	}


	bool Utility::writeProgramInfoFile(const char *fileName, const cl::Device &device)
	{
		ofstream osInfo(fileName);
		if(!osInfo) return false;

		string str;
		device.getInfo(CL_DEVICE_NAME, &str);
		osInfo << "CL_DEVICE_NAME\t"    << str  << "\n";

		device.getInfo(CL_DEVICE_VENDOR, &str);
		osInfo << "CL_DEVICE_VENDOR\t"  << str  << "\n";

		device.getInfo(CL_DEVICE_VERSION, &str);
		osInfo << "CL_DEVICE_VERSION\t" << str << "\n";

		device.getInfo(CL_DRIVER_VERSION, &str);
		osInfo << "CL_DRIVER_VERSION\t" << str << "\n";

		return true;
	}

	bool Utility::checkProgramInfoFile(const char *fileName, const cl::Device &device)
	{
		ifstream isInfo(fileName);
		if(!isInfo) return false;

		char buffer[256];
		while(!isInfo.eof())
		{
			isInfo.getline(buffer, 255);

			int spPos = 0;
			while(buffer[spPos] != '\0' && !isspace(buffer[spPos]))
				spPos++;

			if(buffer[spPos] == '\0') continue;

			std::string name(buffer, buffer+spPos);

			spPos++;
			while(buffer[spPos] != '\0' && isspace(buffer[spPos]))
				spPos++;

			std::string value(buffer+spPos);

			string str;
			if(name == "CL_DEVICE_NAME") {
				device.getInfo(CL_DEVICE_NAME, &str);
				if(str != value) return false;

			} else if (name == "CL_DEVICE_VENDOR") {
				device.getInfo(CL_DEVICE_VENDOR, &str);
				if(str != value) return false;

			} else if (name == "CL_DEVICE_VERSION") {
				device.getInfo(CL_DEVICE_VERSION, &str);
				if(str != value) return false;

			} else if (name == "CL_DRIVER_VERSION") {
				device.getInfo(CL_DRIVER_VERSION, &str);
				if(str != value) return false;
			}
		}

		return true;
	}


	string Utility::simplify(const string &str)
	{
		char c = '\0';
		string result;

		for(int i = 0; i < str.length(); i++) {
			char cc = str[i];
			if(cc == '\t' || cc == '\n' || cc == '\r' || cc == '\b' || cc == '\\' || cc == '/' || cc == ':' || cc == '<' || cc == '>' || cc == '\"' || cc == '|' || cc == '?' || cc == '*')
				cc = ' ';
			if(c == ' ' && cc == ' ')
				continue;
			result += cc;
			c = cc;
		}

		return result;
	}

	
	string Utility::getExePath()
	{
#ifdef _WIN32
		char buffer[MAX_PATH];
		if( !GetModuleFileName(NULL, buffer, sizeof(buffer)) )
			throw std::string("GetModuleFileName() failed!");

		std::string str(buffer);
		int last = (int)str.find_last_of('\\');
#else
		char buffer[PATH_MAX + 1];
		ssize_t len;
		if( (len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1)) == -1 )
			throw std::string("readlink() failed!");

		buffer[len] = '\0';
		std::string str(buffer);
		int last = (int)str.find_last_of('/');
#endif

		return str.substr(0, last + 1);
	}


	size_t Utility::getFileLength(int fh)
	{
		struct _stat64 stat;
		int retVal = _fstat64(fh, &stat);
		if (retVal == -1)
			throw Error("OclBase::getFileLength: Could not access file-status information!");

		return (size_t)stat.st_size;
	}


	size_t Utility::getFileLength(const char *fileName)
	{
		struct _stat64 stat;
		int retVal = _stat64(fileName, &stat);
		if (retVal == -1)
			throw Error("OclBase::getFileLength: Could not access file-status information!");

		return (size_t)stat.st_size;
	}


	__time64_t Utility::getFileModificationTime(int fh)
	{
		struct _stat64 stat;
		int retVal = _fstat64(fh, &stat);
		if (retVal == -1)
			throw Error("OclBase::getFileModificationTime: File not found!", Error::ecFileNotFound);

		return stat.st_mtime;
	}


	__time64_t Utility::getFileModificationTime(const char *fileName)
	{
		struct _stat64 stat;
		int retVal = _stat64(fileName, &stat);
		if (retVal == -1)
			throw Error("OclBase::getFileModificationTime: File not found!", Error::ecFileNotFound);

		return stat.st_mtime;
	}


	cl::Program Utility::buildProgram(const char *progName)
	{
		cl::Context context = getContext();
		cl::vector<cl::Device> devices;
		context.getInfo(CL_CONTEXT_DEVICES, &devices);

		if(devices.size() != 1)
			throw Error("OclBase::buildProgram: Currently only one device supported!");

		cl::Device device = devices[0];
		
		bool cacheBinary = Global::getCacheProgramBinaries();

		string sourceName = getExePath() + progName;
		string binaryName, infoName, deviceName;

		// try reading cached binary file?
		if(cacheBinary)
		{
			// assemble directory and file names for cache
			device.getInfo(CL_DEVICE_NAME, &deviceName);
			deviceName = simplify(deviceName);

			cl_uint deviceVendorID;
			device.getInfo(CL_DEVICE_VENDOR_ID, &deviceVendorID);

			string dirNameCache = getExePath() + "cache";
			string dirNameCacheDevice = dirNameCache + getPathSeparator() + toString(deviceVendorID) + "_" + simplify(deviceName);
			binaryName = dirNameCacheDevice + getPathSeparator() + progName + ".bin";
			infoName   = dirNameCacheDevice + getPathSeparator() + progName + ".info";

			// create cache directories (if not yet present)
			_mkdir(dirNameCache.c_str());
			int retVal = _mkdir(dirNameCacheDevice.c_str());
			cacheBinary = (retVal == 0 || errno == EEXIST);

			// check if we have a cache directory, and if the cached file is up-to-date
			if( retVal == 0 || (errno == EEXIST && (!Global::getRecompileProgramsIfNewerDriver() || checkProgramInfoFile(infoName.c_str(), device))) )
			{
				//read cached file
				FILE *pFile;
				errno_t err = fopen_s(&pFile, binaryName.c_str(), "rb");
				if(err == 0) {
					if(getFileModificationTime(sourceName.c_str()) <= getFileModificationTime(pFile)) {
						size_t size = getFileLength(pFile);
						if(size > 0)
						{
							//cout << "Cached program exists." << endl;
	
							char *buffer = new char[size];
							if(buffer == 0)
								throw Error("OclBase::buildProgram: Could not allocate buffer for cached binary file!", Error::ecOutOfMemory);

							size_t bytesRead = fread(buffer, sizeof(char), size, pFile);
							if(bytesRead < size)
								cerr << "Read only " << bytesRead << " bytes; expected " << size << endl;
							fclose(pFile);

					
							// create program from binary file
							cl::Program::Binaries binaries(1, make_pair(buffer, size));
							cl::vector<cl_int> binaryStatus(1);
							try {
								cl::Program program(context, devices, binaries, &binaryStatus);
								//cout << "status: " << binaryStatus[0] << endl;
								program.build(devices);

								delete [] buffer;
								return program;

							// in case of error when loading binary, just go on and create new binary (below)
							} catch(cl::Error error) {
								cerr << "Error while creating / building program!" << endl;
								delete [] buffer;
							}
						}

					} else
						fclose(pFile);
				}
			}
		}

		// read source file
		ifstream sourceFile(sourceName);
		if(!sourceFile) {
			std::string msg("OclBase::buildProgram: Could not read kernel file ");
			msg.append(sourceName);
			throw Error(msg.c_str(), Error::ecKernelFileNotFound);
		}

		string progstr(istreambuf_iterator<char>(sourceFile), (istreambuf_iterator<char>()));
		cl::Program::Sources source( 1, make_pair(progstr.c_str(), progstr.length()));

		// build program
		cl::Program program = cl::Program(context, source);
		try {
			program.build(devices);
		} catch(cl::Error err) {
			cerr << "Compiler error: " << err.what() << "(" << err.err() << ")" << endl;
			string str =  program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
			throw Error("Could not compile kernels", Error::ecKernelCompileError);
		}

		// cache binary file
		if(cacheBinary) {
			cl_uint numdev;
			program.getInfo(CL_PROGRAM_NUM_DEVICES, &numdev);

			cl::vector<size_t> binarySizes(1);
			program.getInfo(CL_PROGRAM_BINARY_SIZES, &binarySizes);
			size_t size = binarySizes[0];

			char *buffer = new char[size];
			cl::vector<char *> buffers(1, buffer);
			program.getInfo(CL_PROGRAM_BINARIES, &buffers);

			FILE *pFile;
			errno_t err = fopen_s(&pFile, binaryName.c_str(), "wb");
			if(err == 0) {
				size_t bytesWritten = fwrite(buffer, 1, size, pFile);
				fclose(pFile);

				if(bytesWritten < size)
					throw Error("OclBase::buildProgram: Could not write binary file to cache!", Error::ecProgramCacheError);

				if(writeProgramInfoFile(infoName.c_str(), device) == false)
					throw Error("OclBase::buildProgram: Could not write info file to cache!", Error::ecProgramCacheError);
				
			} else {
				throw Error("OclBase::buildProgram: Could not cache binary file!", Error::ecProgramCacheError);
			}

			delete [] buffer;
		}

		return program;
	}
}
