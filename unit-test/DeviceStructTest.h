
#ifndef _DEVICE_STRUCT_TEST
#define _DEVICE_STRUCT_TEST

#include "UnitTest.h"


class DeviceStructTest : public UnitTest
{
public:
	DeviceStructTest(bool silent = false) : UnitTest("DeviceStruct", silent) { }

	bool runTests();

	void testLoadStore();
};


#endif
