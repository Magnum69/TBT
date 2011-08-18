
#ifndef _MAPPED_STRUCT_TEST
#define _MAPPED_STRUCT_TEST

#include "UnitTest.h"
#include <tbt/DeviceController.h>


class MappedStructTest : public UnitTest
{
public:
	MappedStructTest(bool silent = false) : UnitTest("MappedStruct", silent) { }

	bool runTests();

	void testMapMemory();

private:
	template<class FLOAT>
	void doTestMapMemory(tbt::DeviceController *devCon);

};


#endif
