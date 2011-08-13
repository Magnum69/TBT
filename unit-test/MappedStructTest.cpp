
#include "MappedStructTest.h"
#include <tbt/MappedStruct.h>
#include <tbt/Global.h>
#include <tbt/Module.h>

using namespace std;


bool MappedStructTest::runTests()
{
	try {
		testMapMemory();

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


struct Data {
	cl_int    m_n;
	cl_float  m_x;
	cl_double m_y;
	cl_uint   m_d[3];
};

	
class MappedStructTestModule : public tbt::Module
{
	static cl::Kernel s_kernel;

public:
	void run(tbt::MappedStruct<Data> &ms)
	{
		if(!isProgramLoaded()) {
			buildProgramFromSourceRel("mapped-struct-test.cl");
			s_kernel = createKernel("mappedStructTest");
		}

		s_kernel.setArg<cl::Buffer>(0, ms);
		ms.getDeviceController()->enqueueTask(s_kernel);
	}
};

cl::Kernel MappedStructTestModule::s_kernel;


void MappedStructTest::testMapMemory()
{
	tbt::DeviceController *devCon = tbt::getDeviceController();

	tbt::MappedStruct<Data> ms(devCon);

	cl_int n = 17;
	float x = 2.32f;
	double y = -113.56;

	ms->m_n = n;
	ms->m_x = x;
	ms->m_y = y;
	ms->m_d[0] = 15;
	ms->m_d[1] = 25;
	ms->m_d[2] = 35;

	ms.mapHostToDeviceBlocking();

	// TBD: call some test kernel...
	MappedStructTestModule test;
	test.run(ms);
	devCon->finish();

	ms.mapDeviceToHostBlocking();

	// TBD: check what have been done by test kernel...
	UTASSERT(ms->m_n == 2 * n);
	UTASSERT(ms->m_x == x * x);
	UTASSERT(ms->m_y == 4 * y);
	UTASSERT(ms->m_d[0] == 15);
	UTASSERT(ms->m_d[1] == 25);
	UTASSERT(ms->m_d[2] == 15+25+35);
}
