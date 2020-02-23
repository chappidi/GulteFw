#include "TestSuite.h"
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
TEST_F(TestSuiteV2, child_replace_rjt) {
	auto ptX = new_order(mls, 10000);
	ptX.pending();
	ptX.accept();
	// slice order
	auto c1X = ptX.slice_order(epa, 1000);
	c1X.accept();
	c1X.fill(200);
	// replace slice qty decrease
	auto c1Y = c1X.replace_order(800);
	c1Y.reject();
	c1X.fill(100);
	// replace slice increase decrease
	auto c1Z = c1X.replace_order(1500);
	c1Z.reject();
	c1X.fill(100);
}

///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
TEST_F(TestSuiteV2, child_replace_rpld) {

}
