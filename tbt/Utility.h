
#ifndef _TBT_UTILITY_H
#define _TBT_UTILITY_H


#include <tbt/tbthc.h>


namespace tbt
{
	class DeviceController;


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
		 * @param[in] fileName   is the file name of the info file to be created.
		 * @param[in] devCon     is the device controller for which the info file shall be created.
		 * @param[in] extensions specifies the extensions enabled when compiling the program.
		 * @return               true if file creation was successful, otherwise false.
		 */
		static bool writeProgramInfoFile(const char *fileName, const DeviceController *devCon, cl_uint extensions = 0);

		//! Checks the progam info file \a fileName if the cached binary is up-to-date.
		/**
		 * @param[in] fileName   is the file name of the info file.
		 * @param[in] devCon     is the device controller corresponding to the info file.
		 * @param[in] extensions specifies the extensions that should be enabled when compiling the program.
		 * @return               true if the info file could be read successfully and indicates
		 *                       that the cached binaray is up-to-date; otherwise false is returned.
		 */
		static bool checkProgramInfoFile(const char *fileName, const DeviceController *devCon, cl_uint extensions = 0);

	public:
		//! Builds OpenCL program \a progName in the global context.
		/**
		 * @param[in] progName     is the file name of the OpenCL program, relative to the path of the executable.
		 * @param[in] requiredExt  is a bitvector specifying the OpenCL extensions required to build \a progName.
		 * @param[in] optionalExt  is a bitvector specifying optional OpenCL extensions; these extensions are not
		 *                         required to build \a progName, but may be used by conditional compilation.
		 * @return                 the build program.
		 */
		static cl::Program buildProgram(const char *progName, cl_uint requiredExt = 0, cl_uint optionalExt = 0);

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

		static int firstBit(cl_uint bits);
	};

}


#endif
