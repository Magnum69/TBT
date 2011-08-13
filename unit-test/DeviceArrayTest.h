
#ifndef _DEVICE_ARRAY_TEST
#define _DEVICE_ARRAY_TEST

#include "UnitTest.h"


class DeviceArrayTest : public UnitTest
{
public:
	DeviceArrayTest(bool silent = false) : UnitTest("DeviceArray", silent) { }

	bool runTests();

	void testLoadStore();
	void testIterators();
};


#endif
