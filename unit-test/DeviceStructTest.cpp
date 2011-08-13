
#include "DeviceStructTest.h"
#include <tbt/DeviceStruct.h>
#include <tbt/Global.h>

using namespace std;


bool DeviceStructTest::runTests()
{
	try {
		testLoadStore();

	} catch(cl::Error error) {
		cout << "OpenCL exception occurred:" << endl;
		cout << "error code: " << error.err() << endl;
		cout << "message:    " << error.what() << endl;

		return false;

	} catch(tbt::Error error) {
		cout << "TBT exception occurred:" << endl;
		cout << "error code: " << error.code() << endl;
		cout << "message:    " << error.what() << endl;

		return false;
	}

	return ( numberOfErrors() == 0 );
}


void DeviceStructTest::testLoadStore()
{
	tbt::DeviceController *devCon = tbt::getDeviceController();

	struct Data {
		cl_int    m_n;
		cl_float  m_x;
		cl_double m_y;
		cl_uint   m_d[3];
	} v1, v2, v3;

	v1.m_n = 2500;
	v1.m_x = 24.2f;
	v1.m_y = 5760.35;
	v1.m_d[0] = 10;
	v1.m_d[1] = 20;
	v1.m_d[2] = 30;

	tbt::DeviceStruct<Data> ds(devCon);

	ds.loadBlocking (v1);
	ds.storeBlocking(v2);

	UTASSERT( v2.m_n    == 2500    );
	UTASSERT( v2.m_x    == 24.2f   );
	UTASSERT( v2.m_y    == 5760.35 );
	UTASSERT( v2.m_d[0] == 10      );
	UTASSERT( v2.m_d[1] == 20      );
	UTASSERT( v2.m_d[2] == 30      );

	v3.m_n = -2500;
	v3.m_x = -24.2f;
	v3.m_y = -5760.35;
	v3.m_d[0] = 0;
	v3.m_d[1] = 2;
	v3.m_d[2] = 4;

	ds.load(v3);
	devCon->finish();
	ds.store(v1);
	devCon->finish();

	UTASSERT( v1.m_n    == -2500    );
	UTASSERT( v1.m_x    == -24.2f   );
	UTASSERT( v1.m_y    == -5760.35 );
	UTASSERT( v1.m_d[0] == 0        );
	UTASSERT( v1.m_d[1] == 2        );
	UTASSERT( v1.m_d[2] == 4        );
}
