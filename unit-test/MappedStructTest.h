
#ifndef _MAPPED_STRUCT_TEST
#define _MAPPED_STRUCT_TEST

#include "UnitTest.h"


class MappedStructTest : public UnitTest
{
public:
	MappedStructTest(bool silent = false) : UnitTest("MappedStruct", silent) { }

	bool runTests();

	void testMapMemory();
};


#endif
