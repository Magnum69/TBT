
#ifndef _TBT_BASE_H
#define _TBT_BASE_H

#define __NO_STD_VECTOR             // Use cl::vector instead of STL version
#define __CL_ENABLE_EXCEPTIONS      // use exceptions
#pragma warning( disable : 4290 )   // avoid useless warnings with Visual C++ and OpenCL exceptions


#include <CL/cl.hpp>
#include <iostream>

#include <tbt/Error.h>


namespace tbt
{
	//! Maintenance of global platform and context, as well as some options.
	class Global
	{
		friend cl::Platform getPlatform();
		friend cl::Context getContext();
		friend void createContext(cl_device_type deviceType, const cl::Platform &platform);
		friend void createContext(cl_device_type deviceType);

		static cl::Platform s_platform;  //!< the global OpenCL platform we use.
		static cl::Context s_context;    //!< the global OpenCL context we use.

		static bool m_cacheProgramBinaries;           //!< shall we cache program (kernel) binaries at all?
		static bool m_recompileProgramsIfNewerDriver; //!< shall we check driver version and recompile programs if newer?

	public:
		//! Returns current setting of option cacheProgramBinaries.
		static bool getCacheProgramBinaries() { return m_cacheProgramBinaries; }

		//! Sets option cacheProgramBinaries to \a b.
		static void setCacheProgramBinaries(bool b) { m_cacheProgramBinaries = b; }

		//! Returns current setting of option recompileProgramsIfNewerDriver.
		static bool getRecompileProgramsIfNewerDriver() { return m_recompileProgramsIfNewerDriver; }

		//! Sets option recompileProgramsIfNewerDriver to \a b.
		static void setRecompileProgramsIfNewerDriver(bool b) { m_recompileProgramsIfNewerDriver = b; }
	};


	//! Create global context from given platform and device type.
	void createContext(cl_device_type deviceType, const cl::Platform &platform);

	//! Create global context from given device type.
	void createContext(cl_device_type deviceType);

	//! Returns global OpenCL platform.
	inline cl::Platform getPlatform() { return Global::s_platform; }

	//! Returns global OpenCL context.
	inline cl::Context getContext() { return Global::s_context; }

	//! Display information about global OpenCL platform.
	std::ostream &displayPlatformInfo(std::ostream &os = std::cout);

	//! Returns device type of given device.
	cl_device_type getType(const cl::Device &device);

	//! Returns maximal number of compute units of given device.
	cl_uint getMaxComputeUnits(const cl::Device &device);

	//! Returns maximal work group size of given device.
	size_t getMaxWorkGroupSize(const cl::Device &device);

	//! Returns size of local memory (in bytes) of given device.
	cl_ulong getLocalMemSize(const cl::Device &device);



	//! Collection of some static utility functions for file system and kernel caching.
	class Utility
	{
	private:
		//! Adjust string \a str such that in does not contain characters not allowed in file names.
		static std::string simplify(const std::string &str);

		//! Writes the program info file \a fileName for caching binaries of \a device.
		static bool writeProgramInfoFile(const char *fileName, const cl::Device &device);

		//! Checks the progam info file \a fileName if the cached binary is up-to-date.
		static bool checkProgramInfoFile(const char *fileName, const cl::Device &device);

	public:
		//!Builds OpenCL program \a progName in the global context.
		static cl::Program buildProgram(const char *progName);

		//! Returns the string representation of \a i.
		static std::string toString(cl_uint i);

		//! Returns the file path separator of the current system.
		static char getPathSeparator() {
#ifdef _WIN32
			return '\\';
#else
			return '/';
#endif
		}

		//! Returns the length (in bytes) of the file given by file handle \a fh.
		static size_t getFileLength(int fh);

		//! Returns the length (in bytes) of the file given by file descriptor \a pFile.
		static size_t getFileLength(FILE *pFile) { return getFileLength(_fileno(pFile)); }

		//! Returns the length (in bytes) of the file \a fileName.
		static size_t getFileLength(const char *fileName);

		//! Returns the time of last modification for file given by file handle \a fh.
		static __time64_t getFileModificationTime(int fh);

		//! Returns the time of last modification for file given by file descriptor \a pFile.
		static __time64_t getFileModificationTime(FILE *pFile) { return getFileModificationTime(_fileno(pFile)); }

		//! Returns the time of last modification for file \a fileName.
		static __time64_t getFileModificationTime(const char *fileName);

		//! Returns the full path to the current executable.
		static std::string getExePath();
	};
}


#endif
