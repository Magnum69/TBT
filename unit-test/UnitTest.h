
#ifndef _UNIT_TEST_H
#define _UNIT_TEST_H

#include <string>


#define UTASSERT(expr) (void)( (!!(expr)) || (utAssert(expr, #expr, __FILE__, __LINE__), 0) )

class UnitTest
{
	std::string  m_name;
	bool         m_silent;
	unsigned int m_nErrors;

public:
	UnitTest(const std::string &name, bool silent = false) : m_name(name), m_silent(silent), m_nErrors(0) { }

	virtual ~UnitTest() { }


	virtual bool runTests() = 0;

	const std::string &name() const { return m_name; }

	unsigned int numberOfErrors() const { return m_nErrors; }

protected:
	void utAssert(bool expr, const char *strExpr, const char *strFile, unsigned int line);
};


#endif
