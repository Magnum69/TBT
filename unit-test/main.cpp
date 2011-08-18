
#include <iostream>
#include "DeviceArrayTest.h"
#include "DeviceStructTest.h"
#include "MappedStructTest.h"
#include <tbt/Global.h>


using namespace std;


int main()
{
	bool ok = true;

	//tbt::globalConfig.setCacheProgramBinaries(false);

	tbt::createContext(CL_DEVICE_TYPE_GPU);
	cout << "Platform:" << endl;
	tbt::displayPlatformInfo(cout) << endl;


	DeviceArrayTest devArrayTest;
	cout << "Testing unit " << devArrayTest.name() << "..." << endl;
	ok = ok && devArrayTest.runTests();

	DeviceStructTest devStructTest;
	cout << "Testing unit " << devStructTest.name() << "..." << endl;
	ok = ok && devStructTest.runTests();

	MappedStructTest mappedStructTest;
	cout << "Testing unit " << mappedStructTest.name() << "..." << endl;
	ok = ok && mappedStructTest.runTests();


	if(ok)
		cout << "no errors occured." << endl;
	else
		cout << "there were " << devArrayTest.numberOfErrors() << " errors!" << endl;

	return 0;
}
