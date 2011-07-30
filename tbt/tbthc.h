
#ifndef _TBT_BASE_H
#define _TBT_BASE_H

/*! \mainpage The Basic Toolbox for heterogeneous programming
 *
 * TBT is a C++-library that makes it easy to use OpenCL for heterogeneous
 * programming. It provides an easy-to-use interface to OpenCL and
 * implements basic primitives that can also be used with user OpenCL
 * programs.
 *
 * \section Introduction
 *
 */


#define __NO_STD_VECTOR             // Use cl::vector instead of STL version
#define __CL_ENABLE_EXCEPTIONS      // use exceptions
#pragma warning( disable : 4290 )   // avoid useless warnings with Visual C++ and OpenCL exceptions


#include <CL/cl.hpp>
#include <iostream>

#include <tbt/Error.h>


namespace tbt
{

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
