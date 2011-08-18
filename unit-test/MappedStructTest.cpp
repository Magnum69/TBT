
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


template<class FLOAT>
struct Data {
	cl_int    m_n;
	cl_float  m_x;
	FLOAT     m_y;
	cl_uint   m_d[3];
};

	
template<class FLOAT>
class MappedStructTestModule : public tbt::Module
{
	static cl::Kernel s_kernel;

public:
	void run(tbt::MappedStruct<Data<FLOAT> > &ms)
	{
		if(!isProgramLoaded()) {
			buildProgramFromSourceRel("mapped-struct-test.cl", 0, TBT_EXT_FP64);
			s_kernel = createKernel("mappedStructTest");
		}

		s_kernel.setArg<cl::Buffer>(0, ms);
		ms.getDeviceController()->enqueueTask(s_kernel);
	}
};

template<class FLOAT>
cl::Kernel MappedStructTestModule<FLOAT>::s_kernel;


template<class FLOAT>
void MappedStructTest::doTestMapMemory(tbt::DeviceController *devCon)
{
	tbt::MappedStruct<Data<FLOAT> > ms(devCon);

	cl_int    n = 17;
	cl_float  x = 2.32f;
	FLOAT     y = (FLOAT)-113.56;

	ms->m_n = n;
	ms->m_x = x;
	ms->m_y = y;
	ms->m_d[0] = 15;
	ms->m_d[1] = 25;
	ms->m_d[2] = 35;

	ms.mapHostToDeviceBlocking();

	MappedStructTestModule<FLOAT> test;
	test.run(ms);
	devCon->finish();

	ms.mapDeviceToHostBlocking();

	float xRes = x * x;
	FLOAT yRes = y;
	yRes *= 4;

	UTASSERT(ms->m_n == 2 * n);
	UTASSERT(ms->m_x == xRes);
	UTASSERT(ms->m_y == yRes);
	UTASSERT(ms->m_d[0] == 15);
	UTASSERT(ms->m_d[1] == 25);
	UTASSERT(ms->m_d[2] == 15+25+35);
}


void MappedStructTest::testMapMemory()
{
	tbt::DeviceController *devCon = tbt::getDeviceController();

	if(devCon->getExtensions() & TBT_EXT_FP64)
		doTestMapMemory<cl_double>(devCon);
	else
		doTestMapMemory<cl_float>(devCon);
}
