#include "TestSuite.h"
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Child Order		(X)
//  Child Order		(X)
TEST_F(TestSuiteV2, nos_chld) {
	OrderReq ptX(oms, gui, mls, 10000);
	ptX.pending();
	ptX.accept();

	ChildOrderReq chX(ptX, epa, 1000);
	chX.accept();
	chX.fill(200);
	chX.fill(100);
	chX.done();

	ChildOrderReq chY(ptX, epa, 1000);
	chY.accept();
	chY.fill(200);
	chY.fill(100);
	chY.canceled();
}