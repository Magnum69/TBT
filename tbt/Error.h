
#ifndef _TBT_EXCEPTION_H
#define _TBT_EXCEPTION_H


#include <exception>


namespace tbt
{
	//! Errors that are thrown as exceptions by tbt.
	class Error
	{
	public:
		//! The code of an error.
		enum ErrorCode {
			ecUnknown,                 //!< unknown error.
			ecKernelFileNotFound,      //!< a kernel file could not be found.
			ecKernelCompileError,      //!< error while compiling a kernel program.
			ecFileNotFound,            //!< a file could not be found.
			ecOutOfMemory,             //!< not enough memory available.
			ecProgramCacheError,       //!< an error occurred while trying to cache a kernel binary.
			ecNoOpenCLPlatformFound,   //!< no suitable OpenCL platform could be found.
			ecDataTypeNotSupported,    //!< data type of a device array not supported.
			ecExtensionNotSupported    //!< an OpenCL extension is not supported by the device.
		};

		//! Constructs an unknown error.
		Error() : m_code(ecUnknown) { }

		//! Constructs an error with message \a msg and error code \a code.
		explicit Error(const std::string &msg, ErrorCode code = ecUnknown) : m_what(msg), m_code(code) { }

		//! Constructs an error with message \a msg and error code \a code.
		explicit Error(const char *msg, ErrorCode code = ecUnknown) : m_what(msg), m_code(code) { }

		//! Returns the error code.
		ErrorCode code() const { return m_code; }

		//! Returns the error message.
		std::string what() const { return m_what; }

	private:
		ErrorCode   m_code; //!< the error code.
		std::string m_what; //!< the error message.
	};
}


#endif
