#include "TestSuite.h"
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Child Order		(X)
//  Child Order		(X)
TEST_F(TestSuiteV2, nos_chld) {
	auto ptX = new_order(mls, 10000);
	ptX.pending();
	ptX.accept();

	auto chX = ptX.slice_order(epa, 1000);
	chX.accept();
	chX.fill(200);
	chX.fill(100);
	chX.done();

	auto chY = ptX.slice_order(epa, 1000);
	chY.accept();
	chY.fill(200);
	chY.fill(100);
	chY.canceled();
}