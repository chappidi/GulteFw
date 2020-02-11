#include "TestSuite.h"
#include <OrderV2.h>
///////////////////////////////////////////////////////////////
//
TEST_F(TestSuite, SIZE) {
	std::cout << "sizeof(Order) " << sizeof(Order) << std::endl << std::endl;
	std::cout << "sizeof(OrderV2) " << sizeof(OrderV2) << std::endl << std::endl;
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Reject			(X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dA.1.a.html
TEST_F(TestSuite, nos_rjt) {
	auto idX = new_order(10000);
	idX.reject();
	idX.status();
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Pending Ack		(X)
//	Reject			(X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dA.1.a.html
TEST_F(TestSuite, nos_pnd_rjt) {
	auto idX = new_order(10000);
	idX.pending();
	idX.reject();
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Pending Ack		(X)
//  Ack				(X)
//	Reject			(X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dA.1.a.html
TEST_F(TestSuite, nos_pnd_ack_rjt) {
	auto idX = new_order(10000);
	idX.pending();
	idX.accept();
	idX.reject();
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Ack				(X)
//	Reject			(X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dA.1.a.html
TEST_F(TestSuite, nos_ack_rjt) {
	auto idX = new_order(10000);
	idX.accept();
	idX.reject();
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Ack				(X)
//	Fill			(X)
//	Fill			(X)
//	Fill			(X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dA.1.a.html
TEST_F(TestSuite, nos_ack_fill) {
	auto idX = new_order(10000);
	idX.accept();
	// parital fill 2000
	idX.fill(2000);
	// parital fill 1000
	idX.fill(1000);
	// complete fill 7000
	idX.fill(7000);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Ack				(X)
//	Fill			(X)
//	Fill			(X)
//  DoneForDay		(X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dA.1.b.html
TEST_F(TestSuite, nos_ack_fill_done) {
	auto idX = new_order(10000);
	idX.accept();
	// parital fill 2000
	idX.fill(2000);
	// parital fill 1000
	idX.fill(1000);
	// done for day
	idX.done();
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Ack				(X)
//  Canceled		(X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dE.1.b.html
TEST_F(TestSuite, nos_ack_cxld) {
	auto idX = new_order(10000);
	idX.accept();
	// canceled
	idX.canceled();
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Ack				(X)
//	Fill			(X)
//  Canceled		(X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dE.1.b.html
TEST_F(TestSuite, nos_ack_fill_cxld) {
	auto idX = new_order(10000);
	idX.accept();
	// parital fill 1000
	idX.fill(1000);
	idX.canceled();
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//	Ack				(X)
//  NewOrderSingle	(X)
//	Fill			(X)
//  NewOrderSingle	(X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dF.1.a.html
TEST_F(TestSuite, nos_dup) {
	auto idX = new_order(10000);
	// ack order
	idX.accept();
	// resend
	idX.resend();
	// parital fill 2000
	idX.fill(2000);
	// resend
	idX.resend();
}
