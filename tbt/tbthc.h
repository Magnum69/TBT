
#ifndef _TBT_BASE_H
#define _TBT_BASE_H

#define __NO_STD_VECTOR // Use cl::vector instead of STL version
#define __CL_ENABLE_EXCEPTIONS // needed for exceptions
#pragma warning( disable : 4290 )

#include <CL/cl.hpp>
#include <iostream>
#include "Error.h"


namespace tbt
{
	class Global
	{
		friend cl::Platform getPlatform();
		friend cl::Context getContext();
		friend void createContext(cl_device_type deviceType, const cl::Platform &platform);
		friend void createContext(cl_device_type deviceType);

		static cl::Platform s_platform;
		static cl::Context s_context;

		static bool m_cacheProgramBinaries;           // shall we cache program (kernel) binaries at all?
		static bool m_recompileProgramsIfNewerDriver; // shall we check driver version and recompile programs if newer?

	public:
		static bool getCacheProgramBinaries() { return m_cacheProgramBinaries; }
		static void setCacheProgramBinaries(bool b) { m_cacheProgramBinaries = b; }

		static bool getRecompileProgramsIfNewerDriver() { return m_recompileProgramsIfNewerDriver; }
		static void setRecompileProgramsIfNewerDriver(bool b) { m_recompileProgramsIfNewerDriver = b; }
	};


	void createContext(cl_device_type deviceType, const cl::Platform &platform);
	void createContext(cl_device_type deviceType);

	inline cl::Platform getPlatform() { return Global::s_platform; }
	inline cl::Context getContext() { return Global::s_context; }

	std::ostream &displayPlatformInfo(std::ostream &os = std::cout);


	cl::Platform getStdPlatform(cl_device_type deviceType);


	cl_device_type getType(const cl::Device &device);

	cl_uint getMaxComputeUnits(const cl::Device &device);

	size_t getMaxWorkGroupSize(const cl::Device &device);

	cl_ulong getLocalMemSize(const cl::Device &device);


	class Utility
	{
		static std::string simplify(const std::string &str);
		static bool writeProgramInfoFile(const char *fileName, const cl::Device &device);
		static bool checkProgramInfoFile(const char *fileName, const cl::Device &device);

	public:
		static cl::Program buildProgram(const char *progName);

		static std::string toString(cl_uint i);

		static char getPathSeparator() {
#ifdef _WIN32
			return '\\';
#else
			return '/';
#endif
		}

		static size_t getFileLength(int fh);
		static size_t getFileLength(FILE *pFile) { return getFileLength(_fileno(pFile)); }
		static size_t getFileLength(const char *fileName);

		static __time64_t getFileModificationTime(int fh);
		static __time64_t getFileModificationTime(FILE *pFile) { return getFileModificationTime(_fileno(pFile)); }
		static __time64_t getFileModificationTime(const char *fileName);

		static std::string getExePath();
	};
}


#endif
