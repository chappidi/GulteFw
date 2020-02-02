#include "TestSuite.h"

///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Cancel Request	(Y,X)
//	Cancel Reject	(Y,X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.a.html
TEST_F(TestSuite, nos_cxl_rjt) {
	auto idX = new_order();

	//cancel request
	auto idY = cxl_order(idX);
	cxl_rjt(idY, idX, reason);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Cancel Request	(Y,X)
//  Pending Ack		(X)
//	Cancel Reject	(Y,X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.a.html
TEST_F(TestSuite, nos_pnd_cxl_rjt) {
	auto idX = new_order();

	//cancel request
	auto idY = cxl_order(idX);
	pending_ack(idX);
	cxl_rjt(idY, idX, reason);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Ack				(X)
//  Cancel Request	(Y,X)
//	Cancel Reject	(Y,X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.a.html
TEST_F(TestSuite, nos_ack_cxl_rjt) {
	auto idX = new_order();
	ack(idX);

	//cancel request
	auto idY = cxl_order(idX);
	cxl_rjt(idY, idX, reason);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Ack				(X)
//  Cancel Request	(Y,X)
//  Pending Cancel	(Y,X)
//	Cancel Reject	(Y,X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.a.html
TEST_F(TestSuite, nos_ack_cxl_pnd_rjt) {
	auto idX = new_order();
	ack(idX);

	//cancel request
	auto idY = cxl_order(idX);
	pending_cancel(idY, idX);
	cxl_rjt(idY, idX, reason);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Ack				(X)
//	Fill			(X)
//  Cancel Request	(Y,X)
//	Cancel Reject	(Y,X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.a.html
TEST_F(TestSuite, nos_ack_fill_cxl_rjt) {
	auto idX = new_order();
	ack(idX);
	fill(idX, 2000);

	//cancel request
	auto idY = cxl_order(idX);
	cxl_rjt(idY, idX, reason);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Ack				(X)
//	Fill			(X)
//  Cancel Request	(Y,X)
//	Pending Cxl		(Y,X)
//	Cancel Reject	(Y,X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.a.html
TEST_F(TestSuite, nos_ack_fill_cxl_pnd_rjt) {
	auto idX = new_order();
	ack(idX);
	fill(idX, 2000);

	//cancel request
	auto idY = cxl_order(idX);
	pending_cancel(idY, idX);
	cxl_rjt(idY, idX, reason);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Ack				(X)
//  Partial Fill	(X)
//  Cancel Request	(Y,X)
//  Partial Fill	(X)
//	Pending Cxl		(Y,X)
//  Filled			(X)
//	Cancel Reject	(Y,X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.c.html
TEST_F(TestSuite, nos_ack_fill_cxl_fill_pnd_fill_rjt) {
	auto idX = new_order();
	ack(idX);
	fill(idX, 2000);

	//cancel request
	auto idY = cxl_order(idX);
	fill(idX, 3000);
	pending_cancel(idY, idX);
	fill(idX, 5000);
	cxl_rjt(idY, idX, reason);
}






///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Replace Request	(Y,X)
//	Replace Reject	(Y,X)
// https://www.onixs.biz/fix-dictionary/4.4/app_dC.1.a.html
TEST_F(TestSuite, nos_rpl_rjt) {
	auto idX = new_order();
	auto idY = rpl_order(idX, 11000);
	rpl_rjt(idY, idX, reason);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Replace Request	(Y,X)
//  Pending Ack		(X)
//	Replace Reject	(Y,X)
// https://www.onixs.biz/fix-dictionary/4.4/app_dC.1.a.html
TEST_F(TestSuite, nos_pnd_rpl_rjt) {
	auto idX = new_order();
	auto idY = rpl_order(idX, 11000);
	pending_ack(idX);
	rpl_rjt(idY, idX, reason);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Ack				(X)
//  Replace Request	(Y,X)
//	Replace Reject	(Y,X)
// https://www.onixs.biz/fix-dictionary/4.4/app_dC.1.a.html
TEST_F(TestSuite, nos_ack_rpl_rjt) {
	auto idX = new_order();
	ack(idX);
	auto idY = rpl_order(idX, 11000);
	rpl_rjt(idY, idX, reason);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Ack				(X)
//  Replace Request	(Y,X)
//  Pending Replace (Y,X)
//	Replace Reject	(Y,X)
// https://www.onixs.biz/fix-dictionary/4.4/app_dC.1.a.html
TEST_F(TestSuite, nos_ack_rpl_pnd_rjt) {
	auto idX = new_order();
	ack(idX);
	auto idY = rpl_order(idX, 11000);
	pending_replace(idY, idX);
	rpl_rjt(idY, idX, reason);
}