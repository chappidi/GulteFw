#include "TestSuite.h"
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
TEST_F(TestSuiteV2, child_cancel_rjt) {
	auto ptX = new_order(mls, 10000);
	ptX.pending();
	ptX.accept();
	// slice order
	auto c1X = ptX.slice_order(epa, 2000);
	c1X.accept();
	c1X.fill(100);
	// cancel slice qty
	auto c1Y = c1X.cancel_order();
	c1X.fill(120);
	c1Y.reject();
	c1X.fill(150);
	// cancel slice qty
	auto c1Z = c1X.cancel_order();
	c1X.fill(170);
	c1Z.pending();
	c1X.fill(200);
	c1Z.reject();

	c1X.fill(250);
	// unsolicited cxl
	c1X.canceled();
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
TEST_F(TestSuiteV2, child_cancel_cxld) {
	auto ptX = new_order(mls, 10000);
	ptX.pending();
	ptX.accept();
	// slice order 1
	auto c1X = ptX.slice_order(epa, 2000);
	c1X.accept();
	c1X.fill(100);
	// cancel slice qty
	auto c1Y = c1X.cancel_order();
	c1X.fill(120);
	c1Y.reject();
	c1X.fill(150);
	// cancel slice qty
	auto c1Z = c1X.cancel_order();
	c1X.fill(170);
	c1Z.pending();
	c1X.fill(200);
	c1Z.accept();

	// slice order 2
	auto c2X = ptX.slice_order(epa, 3000);
	c2X.accept();
	c2X.fill(100);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
TEST_F(TestSuiteV2, child_cancel_cxld_multi) {
	auto ptX = new_order(mls, 10000);
	ptX.pending();
	ptX.accept();
	// slice order 1 & 2
	auto c1X = ptX.slice_order(epa, 2000);
	auto c2X = ptX.slice_order(epa, 3000);
	c2X.accept();
	c1X.accept();

	c1X.fill(100);
	c2X.fill(100);

	// cancel slice 1
	auto c1Y = c1X.cancel_order();
	c1X.fill(120);
	c1Y.reject();
	c1X.fill(150);
	// cancel slice 2
	auto c2Y = c2X.cancel_order();
	c2Y.pending();
	// cancel slice 1
	auto c1Z = c1X.cancel_order();
	c1X.fill(170);
	c1Z.pending();
	c1X.fill(200);
	c1Z.accept();

	c2Y.accept();
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
TEST_F(TestSuiteV2, parent_cancel_cxld) {
	auto ptX = new_order(mls, 10000);
	ptX.pending();
	ptX.accept();
	// slice order 1 & 2
	auto c1X = ptX.slice_order(epa, 2000);
	auto c2X = ptX.slice_order(epa, 3000);
	c2X.accept();
	c1X.accept();

	c1X.fill(100);
	c2X.fill(100);

	// cancel parent
	auto ptY = ptX.cancel_order();
	ptY.pending();
	c1X.fill(100);
	c2X.fill(100);
	ptY.reject();

	// fill child 
	c1X.fill(100);
	c2X.fill(100);

	// cancel parent
	auto ptZ = ptX.cancel_order();
	ptZ.pending();
	c1X.fill(100);
	c2X.fill(100);
	// TODO if any open child. send pending_cancel only
	ptZ.accept();
	// TODO: check any child and send 
}