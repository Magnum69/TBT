
#include <tbt/Utility.h>
#include <tbt/Global.h>

#include <fstream>
#include <direct.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sstream>
#include <sys/stat.h>


using namespace std;


namespace tbt {
	
	string Utility::toString(cl_uint i)
	{
		stringstream ss;
		ss << i;
		return ss.str();
	}


	bool Utility::writeProgramInfoFile(const char *fileName, const DeviceController *devCon, cl_uint extensions)
	{
		ofstream osInfo(fileName);
		if(!osInfo) return false;

		osInfo << "CL_DEVICE_NAME\t"        << devCon->getName()          << "\n";
		osInfo << "CL_DEVICE_VENDOR\t"      << devCon->getVendor()        << "\n";
		osInfo << "CL_DEVICE_VERSION\t"     << devCon->getVersion()       << "\n";
		osInfo << "CL_DRIVER_VERSION\t"     << devCon->getDriverVersion() << "\n";
		osInfo << "TBT_DEVICE_EXTENSIONS\t" << extensions                 << "\n";

		return true;
	}

	bool Utility::checkProgramInfoFile(const char *fileName, const DeviceController *devCon, cl_uint extensions)
	{
		ifstream isInfo(fileName);
		if(!isInfo) return false;

		bool checkedName       = false;
		bool checkedVendor     = false;
		bool checkedVersion    = false;
		bool checkedDriver     = false;
		bool checkedExtensions = false;

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
				if(devCon->getName() != value) return false;
				checkedName = true;

			} else if (name == "CL_DEVICE_VENDOR") {
				if(devCon->getVendor() != value) return false;
				checkedVendor = true;

			} else if (name == "CL_DEVICE_VERSION") {
				if(devCon->getVersion() != value) return false;
				checkedVersion = true;

			} else if (name == "CL_DRIVER_VERSION") {
				if(devCon->getDriverVersion() != value) return false;
				checkedDriver = true;

			} else if (name == "TBT_DEVICE_EXTENSIONS") {
				if(extensions != atoi(value.c_str())) return false;
				checkedExtensions = true;
			}
		}

		return checkedName && checkedVendor && checkedVersion && checkedDriver && checkedExtensions;
	}


	string Utility::simplify(const string &str)
	{
		char c = '\0';
		string result;

		for(size_t i = 0; i < str.length(); i++) {
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


	cl::Program Utility::buildProgram(const char *progName, cl_uint requiredExt, cl_uint optionalExt)
	{
		cl::Context context = getContext();
		DeviceController *devCon = getDeviceController();

		cl::vector<cl::Device> devices;
		devices.push_back(devCon->getDevice());
		//context.getInfo(CL_CONTEXT_DEVICES, &devices);

		//if(devices.size() != 1)
		//	throw Error("OclBase::buildProgram: Currently only one device supported!");

		cl_uint extensions = (requiredExt | optionalExt) & devCon->getExtensions();
		bool cacheBinary = globalConfig.getCacheProgramBinaries();

		string sourceName = getExePath() + progName;
		string binaryName, infoName, deviceName;

		// try reading cached binary file?
		if(cacheBinary)
		{
			// assemble directory and file names for cache
			deviceName = simplify(devCon->getName());

			cl_uint deviceVendorID = devCon->getVendorID();

			string dirNameCache = getExePath() + "cache";
			string dirNameCacheDevice = dirNameCache + getPathSeparator() + toString(deviceVendorID) + "_" + simplify(deviceName);
			binaryName = dirNameCacheDevice + getPathSeparator() + progName + ".bin";
			infoName   = dirNameCacheDevice + getPathSeparator() + progName + ".info";

			// create cache directories (if not yet present)
			_mkdir(dirNameCache.c_str());
			int retVal = _mkdir(dirNameCacheDevice.c_str());
			cacheBinary = (retVal == 0 || errno == EEXIST);

			// check if we have a cache directory, and if the cached file is up-to-date
			if( retVal == 0 || (errno == EEXIST && (!globalConfig.getRecompileProgramsIfNewerDriver() || checkProgramInfoFile(infoName.c_str(), devCon, extensions))) )
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
		const string &header = devCon->getOpenCLHeader(requiredExt, optionalExt);

		cl::Program::Sources source;
		source.push_back(make_pair(header .c_str(), header. length()));
		source.push_back(make_pair(progstr.c_str(), progstr.length()));

		// build program
		cl::Program program = cl::Program(context, source);
		try {
			program.build(devices);
		} catch(cl::Error err) {
			if(err.err() == CL_BUILD_PROGRAM_FAILURE) {
				string msg = "Could not compile kernels.\nBuild-Log:\n";
				msg += program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devCon->getDevice());
				throw Error(msg.c_str(), Error::ecKernelCompileError);

			} else
				throw;
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

				if(writeProgramInfoFile(infoName.c_str(), devCon, extensions) == false)
					throw Error("OclBase::buildProgram: Could not write info file to cache!", Error::ecProgramCacheError);
				
			} else {
				throw Error("OclBase::buildProgram: Could not cache binary file!", Error::ecProgramCacheError);
			}

			delete [] buffer;
		}

		return program;
	}


	void *Utility::alignedMalloc(size_t size, size_t alignment)
	{
		return _aligned_malloc(size, alignment);
	}


	void Utility::alignedFree(void *ptr)
	{
		_aligned_free(ptr);
	}


	int Utility::firstBit(cl_uint bits) {
		int num = 0;
		while((bits & 0x01) == 0 && num < 32) {
			bits >>= 1;
			++num;
		}

		return num;
	}

}
