#include "TestSuite.h"
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Child Order		(X)
//  Child Order		(X)
TEST_F(TestSuiteV2, nos_slice) {
	auto ptX = new_order(mls, 10000);
	ptX.pending();
	ptX.accept();
	ptX.fill(500);

	auto ch1X = ptX.slice_order(epa, 1000);
	ch1X.accept();
	ch1X.fill(200);
	ch1X.fill(100);
	ch1X.done();
	ptX.fill(500, 99.98);

	auto ch2X = ptX.slice_order(epa, 1000);
	ch2X.accept();
	ch2X.fill(200);
	ch2X.fill(100);
	ch2X.canceled();
	ptX.fill(500, 99.98);
}

///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Child Order		(X)
//  Child Order		(X)
TEST_F(TestSuiteV2, nos_over_slice) {
	auto ptX = new_order(mls, 10000);
	ptX.pending();
	ptX.accept();
	ptX.fill(500, 99.98);

	auto ch1X = ptX.slice_order(epa, 6000);
	ch1X.accept();
	ch1X.fill(200);
	ch1X.fill(100);
	ptX.fill(1500, 99.98);
	// over slice
	auto ch2X = ptX.slice_order(epa, 4000, OrdStatus::Rejected);
	ch2X.status();
	ch1X.done();
	auto ch2X2 = ptX.slice_order(epa, 4000, OrdStatus::Rejected);
	ch2X2.status();
	ptX.fill(500, 99.98);
}

///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Child Order		(X1, X)
//  Replace			(Y1, X1)
//  Replace	Reject	(Y1, X1)
//  Child Order		(X2, X)
//  Replace			(Y2, X2)
//  Replaced		(Y2, X2)
TEST_F(TestSuiteV2, nos_chld_rpl) {
	auto ptX = new_order(mls, 10000);
	ptX.pending();
	ptX.accept();

	auto ch1X = ptX.slice_order(epa, 1000);
	ch1X.accept();
	ch1X.fill(200);
	ch1X.fill(100);
	auto ch1Y = ch1X.replace_order(2000);
	ch1Y.accept();
	ch1Y.fill(500);

	auto ch2X = ptX.slice_order(epa, 1000);
	ch2X.accept();
	ch2X.fill(200);
	ch2X.fill(100);
	auto ch2Y = ch2X.replace_order(3000);
	ch2Y.reject();
	ch2X.fill(100);
}
