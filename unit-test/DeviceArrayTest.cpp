
#include "DeviceArrayTest.h"
#include <tbt/DeviceArray.h>
#include <tbt/Global.h>

using namespace std;


bool DeviceArrayTest::runTests()
{
	try {
		testLoadStore();
		testIterators();

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


void DeviceArrayTest::testLoadStore()
{
	tbt::DeviceController *devCon = tbt::getDeviceController();

	//-------------------------------------------------------------------------
	// Test load -> store with nicely sized float-array
	//-------------------------------------------------------------------------

	int n = 10240;
	tbt::DeviceArray<float> da_f(devCon, n);
	UTASSERT( da_f.size() == n );

	float *ptrA = new float[n];
	for(int i = 0; i < n; ++i)
		ptrA[i] = (float)(3*i);

	da_f.loadBlocking(ptrA);

	float *ptrB = new float[n];
	da_f.storeBlocking(ptrB);

	for(int i = 0; i < n; ++i)
		UTASSERT( ptrA[i] == ptrB[i] );

	delete [] ptrA;
	delete [] ptrB;


	//-------------------------------------------------------------------------
	// Test load -> store with host array of type cl_ulong
	//-------------------------------------------------------------------------

	n = 56733;
	tbt::HostArray  <cl_ulong> ha_ul(n);
	tbt::DeviceArray<cl_ulong> da_ul(devCon, n);

	for(int i = 0; i < n; ++i) {
		cl_ulong li = (cl_ulong)i;
		ha_ul[i] = li + (li << 32);
	}

	da_ul.loadBlocking(ha_ul);

	tbt::HostArray<cl_ulong> haB(n);
	cl::Event eventStore;
	da_ul.store(haB, &eventStore);

	eventStore.wait();

	for(int i = 0; i < n; ++i)
		UTASSERT( ha_ul[i] == ha_ul[i] );


	//-------------------------------------------------------------------------
	// Test load -> store with subarray of type double
	//-------------------------------------------------------------------------

	n = 128000;
	int m = 33201;
	int offset = 2753;

	double *ptrdA = new double[n];

	for(int i = 0; i < n; ++i)
		ptrdA[i] = 17.45 * i;

	tbt::DeviceArray<double> da_d(devCon, n);
	da_d.load(ptrdA);
	devCon->finish();

	double *ptrdB = new double[m];
	da_d.storeBlocking(da_d.at(offset), da_d.at(offset+m), ptrdB);

	for(int i = 0; i < m; i++)
		UTASSERT(ptrdB[i] == ptrdA[offset+i]);

	delete [] ptrdA;
	delete [] ptrdB;

	//-------------------------------------------------------------------------
	// Test load -> store with subarray of type int
	//-------------------------------------------------------------------------

	n = 100000;
	m = 3765;
	offset = 17351;
	tbt::HostArray<cl_int> ha_i(n);

	for(int i = 0; i < n; i++)
		ha_i[i] = i;

	tbt::DeviceArray<cl_int> da_i(devCon,n);

	da_i.loadBlocking (da_i.at(offset), da_i.at(offset+m), ha_i.at(offset));
	da_i.storeBlocking(da_i.at(offset), da_i.at(offset+m), ha_i.at(offset+m));

	for(int i = 0; i < n; i++) {
		if(i < offset+m || i >= offset+2*m)
			UTASSERT( ha_i[i] == i );
		else
			UTASSERT( ha_i[i] == i-m );
	}
}


void DeviceArrayTest::testIterators()
{
	tbt::DeviceController *devCon = tbt::getDeviceController();

	const int n = 150;
	tbt::DeviceArray<cl_int> da(devCon, n);

	int *ptrA = new int[n];
	int *ptrB = new int[n];
	for(int i = 0; i < n; i++) {
		ptrA[i] = i;
		ptrB[i] = 0;
	}

	da.loadBlocking(ptrA);

	//-------------------------------------------------------------------------
	// Test begin(), end(), rbegin(), rend(), at() iterators
	//-------------------------------------------------------------------------

	tbt::DeviceArray<cl_int>::const_iterator itBegin = da.begin();
	tbt::DeviceArray<cl_int>::const_iterator itEnd   = da.end();

	da.storeBlocking(itBegin, itEnd, ptrB);

	for(int i = 0; i < n; i++)
		UTASSERT( ptrB[i] == i );

	int value;
	tbt::DeviceArray<cl_int>::iterator it = da.at(77);
	tbt::DeviceArray<cl_int>::iterator it2 = it+1;

	da.storeBlocking(it, it2, &value);
	UTASSERT( value == 77 );

	UTASSERT( itBegin == da.rend()   + 1 );
	UTASSERT( itEnd   == da.rbegin() + 1 );

	UTASSERT( it.getDeviceArray() == &da );


	//-------------------------------------------------------------------------
	// Test pointer arithmetic
	//-------------------------------------------------------------------------

	it2 = it;
	it2 += 11;
	UTASSERT( it2 - it == 11 );

	it2 -= 11;
	UTASSERT( it2 == it );

	it2 = it + 3;
	UTASSERT( it2 - it == 3 );

	it2 = it2 - 5;
	UTASSERT( it2 - it == -2 );

	it2 = it;
	UTASSERT( it2 - it == 0 );

	UTASSERT( ++it2 - it == 1 );
	it2--;
	UTASSERT( it2 == it );

	UTASSERT( --it2 - it == -1 );

	tbt::DeviceArray<cl_int>::const_iterator cit1 = da.at(20);
	tbt::DeviceArray<cl_int>::const_iterator cit2(cit1);

	UTASSERT( ++cit2 - cit1 == 1 );
	cit2--;
	UTASSERT( cit2 == cit1 );

	UTASSERT( --cit2 - cit1 == -1 );

	cit2 = cit1;
	cit2 += 31;
	UTASSERT( cit2 - cit1 == 31 );

	cit2 -= 31;
	UTASSERT( cit2 == cit1 );

	cit2 = cit1 + 3;
	UTASSERT( cit2 - cit1 == 3 );

	cit2 = cit2 - 5;
	UTASSERT( cit2 - cit1 == -2 );

	cit2 = cit1;
	UTASSERT( cit2 - cit1 == 0 );

	UTASSERT( ++cit2 - cit1 == 1 );
	cit2--;
	UTASSERT( cit2 == cit1 );

	UTASSERT( --cit2 - cit1 == -1 );


	//-------------------------------------------------------------------------
	// Test pointer comparison
	//-------------------------------------------------------------------------

	it2 = it + 7;
	UTASSERT( (it2 <  it) == false );
	UTASSERT( (it2 <= it) == false );
	UTASSERT( (it2 >  it) == true  );
	UTASSERT( (it2 >= it) == true  );
	UTASSERT( (it2 == it) == false );
	UTASSERT( (it2 != it) == true  );

	UTASSERT( (it == it) == true  );
	UTASSERT( (it != it) == false );

	cit1 = it;
	cit2 = it - 12;

	UTASSERT( (cit2 <  cit1) == true  );
	UTASSERT( (cit2 <= cit1) == true  );
	UTASSERT( (cit2 >  cit1) == false );
	UTASSERT( (cit2 >= cit1) == false );
	UTASSERT( (cit2 == cit1) == false );
	UTASSERT( (cit2 != cit1) == true  );

	UTASSERT( (cit1 == cit1) == true  );
	UTASSERT( (cit1 != cit1) == false );
}

