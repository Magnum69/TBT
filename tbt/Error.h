
#ifndef _TBT_EXCEPTION_H
#define _TBT_EXCEPTION_H


#include <exception>


namespace tbt
{
	//! Errors that are thrown as exceptions by tbt.
	class Error : public std::exception
	{
	public:
		//! The code of an error.
		enum ErrorCode {
			errUnknown,                 //!< unknown error.
			errKernelFileNotFound,      //!< a kernel file could not be found.
			errKernelCompileError,      //!< error while compiling a kernel program.
			errFileNotFound,            //!< a file could not be found.
			errOutOfMemory,             //!< not enough memory available.
			errProgramCacheError,       //!< an error occurred while trying to cache a kernel binary.
			errNoOpenCLPlatformFound    //!< no suitable OpenCL platform could be found.
		};

		//! Constructs an unknown error.
		Error() : std::exception(), m_code(errUnknown) { }

		//! Constructs an errors with message \a msg and unknown error code.
		Error(const char *msg) : std::exception(msg), m_code(errUnknown) { }

		//! Constructs an error with message \a msg and error code \a code.
		Error(const char *msg, ErrorCode code) : std::exception(msg), m_code(code) { }

		//! Returns the error code.
		ErrorCode code() const { return m_code; }

	private:
		ErrorCode m_code; //!< the error code.
	};
}


#endif
