
#ifndef _TBT_EXCEPTION_H
#define _TBT_EXCEPTION_H


#include <exception>


namespace tbt
{

	class Error : public std::exception
	{
	public:
		enum ErrorCode {
			errUnknown, errKernelFileNotFound, errKernelCompileError, errFileNotFound, errOutOfMemory, errProgramCacheError, errNoOpenCLPlatformFound
		};

		Error() : std::exception(), m_code(errUnknown) { }
		Error(const char *msg) : std::exception(msg), m_code(errUnknown) { }
		Error(const char *msg, ErrorCode code) : std::exception(msg), m_code(code) { }

		ErrorCode code() const { return m_code; }

	private:
		ErrorCode m_code;
	};
}


#endif
