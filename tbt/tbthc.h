
#ifndef _TBT_BASE_H
#define _TBT_BASE_H

/*! \mainpage Introduction
 *
 * TBT&mdash;<em>The Basic Toolbox for heterogeneous programming</em>&mdash;is a C++-library
 * that makes it easy to use OpenCL for heterogeneous programming. It provides an easy-to-use
 * interface to OpenCL and implements basic primitives that can also be used with user OpenCL
 * programs.
 *
 * \section Overview
 * TBT provides the following functionality:
 *   - Automatic selection of the OpenCL platform and creation of a global OpenCL context and command queues for the OpenCL devices.
 *   - Automatic caching of program binaries.
 *   - Generic host and device arrays associating structured data with a specific OpenCL device and command queue.
 *   - Easy access to device-specific information and simplified enqueueing of commands.
 *   - Basic primitives for building OpenCL programs.
 *
 * \section hello-world TBT's hello world
 * This is a simple TBT-program that sorts an array on the GPU.
 *
 * \code
 * #include <tbt/algorithm.h>
 *
 * int main()
 * {
 *     // Create a global context for a GPU device (automatically selects an appropriate platform)
 *     tbt::createContext(CL_DEVICE_TYPE_GPU);
 *
 *     // Get a device controller in the global context.
 *     tbt::DeviceController *devCon = tbt::getDeviceController();
 *     
 *     const size_t n = 4096;
 *     tbt::MappedArray<cl_uint> mappedArray(devCon, n);  // Create an array on the host, mapped to a GPU device 
 *     initRandom(mappedArray);                           // initialize array with random data (on the host)
 *
 *     mappedArray.mapHostToDevice();                     // Transfer array from host to device
 *     radixSort(mappedArray);                            // Sort the array on the GPU device
 *     mappedArray.mapDeviceToHostBlocking();             // Transfer (sorted) array back to host
 * }
 * \endcode
 */


#define __NO_STD_VECTOR             // Use cl::vector instead of STL version
#define __CL_ENABLE_EXCEPTIONS      // use exceptions
#pragma warning( disable : 4290 )   // avoid useless warnings with Visual C++ and OpenCL exceptions


#include <CL/cl.hpp>
#include <iostream>

#include <tbt/Error.h>


namespace tbt
{

	//! The type for array indices.
	typedef ptrdiff_t index_t;


	//! Collection of some static utility functions for file system and kernel caching.
	class Utility
	{
	private:
		//! Adjust string \a str such that in does not contain characters not allowed in file names.
		/**
		 * @param[in] str  is the string to be simplified.
		 * @return         the simplified string.
		 */
		static std::string simplify(const std::string &str);

		//! Writes the program info file \a fileName for caching binaries of \a device.
		/**
		 * @param[in] fileName  is the file name of the info file to be created.
		 * @param[in] device    is the device for which the info file shall be created.
		 * @return              true if file creation was successful, otherwise false.
		 */
		static bool writeProgramInfoFile(const char *fileName, const cl::Device &device);

		//! Checks the progam info file \a fileName if the cached binary is up-to-date.
		/**
		 * @param[in] fileName  is the file name of the info file.
		 * @param[in] device    is the device corresponding to the info file.
		 * @return              true if the info file could be read successfully and indicates
		 *                      that the cached binaray is up-to-date; otherwise false is returned.
		 */
		static bool checkProgramInfoFile(const char *fileName, const cl::Device &device);

	public:
		//! Builds OpenCL program \a progName in the global context.
		/**
		 * @param[in] progName  is the file name of the OpenCL program, relative to the path of the executable.
		 * @return              the build program.
		 */
		static cl::Program buildProgram(const char *progName);

		//! Returns the string representation of \a i.
		/**
		 * @param[in] i  is the number to be converted to a string.
		 * @return       the string representation of \a i.
		 */
		static std::string toString(cl_uint i);

		//! Returns the file path separator of the current system.
		/**
		 * @return the file path separator: <tt>'/'</tt> on Linux/Mac systems, and <tt>'\\'</tt> on Windows.
		 */
		static char getPathSeparator() {
#ifdef _WIN32
			return '\\';
#else
			return '/';
#endif
		}

		//! Returns the length (in bytes) of the file given by file handle \a fh.
		/**
		 * @param[in] fh  must be a valid file handle.
		 * @return        the length of the file in bytes.
		 */
		static size_t getFileLength(int fh);

		//! Returns the length (in bytes) of the file given by file descriptor \a pFile.
		/**
		 * @param[in] pFile  must be a valid file descriptor.
		 * @return           the length of the file in bytes.
		 */
		static size_t getFileLength(FILE *pFile) { return getFileLength(_fileno(pFile)); }

		//! Returns the length (in bytes) of the file \a fileName.
		/**
		 * @param[in] fileName  must be the file name.
		 * @return              the length of the file in bytes.
		 */
		static size_t getFileLength(const char *fileName);

		//! Returns the time of last modification for file given by file handle \a fh.
		/**
		 * @param[in] fh  must be a valid file handle.
		 * @return        the last time of modification as __time64_t structure.
		 */
		static __time64_t getFileModificationTime(int fh);

		//! Returns the time of last modification for file given by file descriptor \a pFile.
		/**
		 * @param[in] pFile  must be a valid file descriptor.
		 * @return           the last time of modification as __time64_t structure.
		 */
		static __time64_t getFileModificationTime(FILE *pFile) { return getFileModificationTime(_fileno(pFile)); }

		//! Returns the time of last modification for file \a fileName.
		/**
		 * @param[in] fileName  must be the file name.
		 * @return              the last time of modification as __time64_t structure.
		 */
		static __time64_t getFileModificationTime(const char *fileName);

		//! Returns the full path to the current executable.
		/**
		 * @return  the full path name of the current executable.
		 */
		static std::string getExePath();

		//! Allocates \a size bytes of memory on a specified alignment boundary.
		/**
		 * @param[in] size       is the size of memory in bytes to be allocated.
		 * @param[in] alignment  specifies the alignment boundary in bytes; must be a power of 2.
		 * @return               the base address of the allocated memory; this address is a multiple of \a alignment.
		 */
		static void *alignedMalloc(size_t size, size_t alignment);

		//! Frees memory allocated with alignedMalloc().
		/**
		 * @param[in] ptr  is the base address of the memory block to be freed; may be 0, in which case no action is performed.
		 */
		static void alignedFree(void *ptr);
	};
}


#endif
