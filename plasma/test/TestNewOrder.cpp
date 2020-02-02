#include "TestSuite.h"

///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Reject			(X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dA.1.a.html
TEST_F(TestSuite, nos_rjt) {
	auto idX = new_order();
	reject(idX);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Pending Ack		(X)
//	Reject			(X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dA.1.a.html
TEST_F(TestSuite, nos_pnd_rjt) {
	auto idX = new_order();
	pending_ack(idX);
	reject(idX);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Pending Ack		(X)
//  Ack				(X)
//	Reject			(X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dA.1.a.html
TEST_F(TestSuite, nos_pnd_ack_rjt) {
	auto idX = new_order();
	pending_ack(idX);
	ack(idX);
	reject(idX);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Ack				(X)
//	Reject			(X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dA.1.a.html
TEST_F(TestSuite, nos_ack_rjt) {
	auto idX = new_order();
	ack(idX);
	reject(idX);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Ack				(X)
//	Fill			(X)
//	Fill			(X)
//	Fill			(X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dA.1.a.html
TEST_F(TestSuite, nos_ack_fill) {
	auto idX = new_order();
	ack(idX);
	// parital fill 2000
	fill(idX, 2000);
	// parital fill 1000
	fill(idX, 1000);
	// complete fill 7000
	fill(idX, 7000);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Ack				(X)
//	Fill			(X)
//	Fill			(X)
//  DoneForDay		(X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dA.1.b.html
TEST_F(TestSuite, nos_ack_fill_done) {
	auto idX = new_order();
	ack(idX);
	// parital fill 2000
	fill(idX, 2000);
	// parital fill 1000
	fill(idX, 1000);
	// done for day
	done(idX);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Ack				(X)
//  Cancelled		(X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dE.1.b.html
TEST_F(TestSuite, nos_ack_cxld) {
	auto idX = new_order();
	ack(idX);
	// cancelled
	cxld(idX);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Ack				(X)
//	Fill			(X)
//  Cancelled		(X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dE.1.b.html
TEST_F(TestSuite, nos_ack_fill_cxld) {
	auto idX = new_order();
	ack(idX);
	// parital fill 1000
	fill(idX, 1000);
	cxld(idX);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//	Ack				(X)
//  NewOrderSingle	(X)
//	Fill			(X)
//  NewOrderSingle	(X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dF.1.a.html
TEST_F(TestSuite, nos_dup) {
	auto idX = new_order();
	// ack order
	ack(idX);
	// resend
	resend_nos(idX);
	// parital fill 2000
	fill(idX, 2000);
	// resend
	resend_nos(idX);
}