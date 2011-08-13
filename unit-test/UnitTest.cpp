
#include "UnitTest.h"
#include <iostream>

using namespace std;


void UnitTest::utAssert(bool expr, const char *strExpr, const char *strFile, unsigned int line)
{
	if(!m_silent) {
		cout << "In file " << strFile << ", line " << line << ":\n";
		cout << "  assertion " << strExpr << " failed!" << endl;
	}

	m_nErrors++;
}
