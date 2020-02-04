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
//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.d.html
TEST_F(TestSuite, nos_pnd_cxl_rjt) {
	auto idX = new_order();

	//cancel request
	auto idY = cxl_order(idX);
	pending_ack(idX);
	cxl_rjt(idY, idX, reason);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Cancel Request	(Y,X)
//  Ack				(X)
//	Cancel Reject	(Y,X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.d.html
TEST_F(TestSuite, nos_cxl_ack_rjt) {
	auto idX = new_order();

	//cancel request
	auto idY = cxl_order(idX);
	ack(idX);
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
//  Ack				(X)
//	Fill			(X)
//  Cancel Request	(Y,X)
//	Canceled		(Y,X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.a.html
TEST_F(TestSuite, nos_ack_cxl_cxld) {
	auto idX = new_order();
	ack(idX);
	fill(idX, 2000);

	//cancel request
	auto idY = cxl_order(idX);
	// accept cancel
	cxld(idY, idX);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Ack				(X)
//  Cancel Request	(Y,X)
//	Pending Cxl		(Y,X)
//	Canceled		(Y,X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.a.html
TEST_F(TestSuite, nos_ack_cxl_pnd_cxld) {
	auto idX = new_order();
	ack(idX);
	fill(idX, 2000);

	//cancel request
	auto idY = cxl_order(idX);
	pending_cancel(idY, idX);
	cxld(idY, idX);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Ack				(X)
//  Partial Fill	(X)
//  Cancel Request	(Y,X)
//  Partial Fill	(X)
//	Pending Cxl		(Y,X)
//  Partial Fill	(X)
//	Canceled		(Y,X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.b.html
TEST_F(TestSuite, nos_fill_cxl_fill_pnd_fill_cxld) {
	auto idX = new_order();
	ack(idX);
	fill(idX, 2000);

	//cancel request
	auto idY = cxl_order(idX);
	fill(idX, 3000);
	pending_cancel(idY, idX);
	fill(idX, 1000);
	cxld(idY, idX);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Cancel Request	(Y,X)
//  Pending	Cxl		(Y,X)
//  Ack				(X)
//	Canceled		(Y,X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.d.html
TEST_F(TestSuite, nos_cxl_pnd_ack_cxld) {
	auto idX = new_order();

	//cancel request
	auto idY = cxl_order(idX);
	pending_cancel(idY, idX);
	ack(idX);
	cxld(idY, idX);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Cancel Request	(Y,X)
//  Pending	Cxl		(Y,X)
//	Canceled		(Y,X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.d.html
TEST_F(TestSuite, nos_cxl_pnd_cxld) {
	auto idX = new_order();

	//cancel request
	auto idY = cxl_order(idX);
	pending_cancel(idY, idX);
	cxld(idY, idX);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Cancel Request	(Y,X)
//  Ack				(X)
//  Pending	Cxl		(Y,X)
//	Canceled		(Y,X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.e.html
TEST_F(TestSuite, nos_cxl_ack_pnd_cxld) {
	auto idX = new_order();

	//cancel request
	auto idY = cxl_order(idX);
	ack(idX);
	pending_cancel(idY, idX);
	cxld(idY, idX);
}
///////////////////////////////////////////////////////////////
//  Cancel Request	(Y,X)
//	Cancel Reject	(Y,X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.f.html
TEST_F(TestSuite, cxl_rjt_unknwn) {
	auto idX = 929;

	//cancel request on unsent order
	// plasma will reject it automatically
	auto idY = cxl_order(idX);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Ack				(X)
//  Partial Fill	(X)
//  Cancel Request	(Y,X)
//  Cancel Request	(Z,X)
//  Partial Fill	(X)
//  Pending	Cxl		(Y,X)
//  Pending	Cxl		(Z,X)
//  Partial Fill	(X)
//	Cancel Reject	(Y,X)
//  Partial Fill	(X)
//	Cancel Reject	(Z,X)
TEST_F(TestSuite, nos_fill_cxl_cxl_fill_rjt_fill_rjt) {
	auto idX = new_order();
	ack(idX);
	fill(idX, 2000);

	//cancel request
	auto idY = cxl_order(idX);
	auto idZ = cxl_order(idX);
	fill(idX, 3000);
	pending_cancel(idY, idX);
	pending_cancel(idZ, idX);
	fill(idX, 1000);
	cxl_rjt(idY, idX, reason);
	fill(idX, 1500);
	cxl_rjt(idZ, idX, reason);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Ack				(X)
//  Partial Fill	(X)
//  Cancel Request	(Y,X)
//  Cancel Request	(Z,X)
//  Partial Fill	(X)
//  Pending	Cxl		(Y,X)
//  Pending	Cxl		(Z,X)
//  Partial Fill	(X)
//	Canceled		(Y,X)
//	Cancel Reject	(Z,X)
TEST_F(TestSuite, nos_fill_cxl_cxl_fill_cxld_rjt) {
	auto idX = new_order();
	ack(idX);
	fill(idX, 2000);

	//cancel request
	auto idY = cxl_order(idX);
	auto idZ = cxl_order(idX);
	fill(idX, 3000);
	pending_cancel(idY, idX);
	pending_cancel(idZ, idX);
	fill(idX, 1000);
	cxld(idY, idX);
	cxl_rjt(idZ, idX, reason);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Ack				(X)
//  Partial Fill	(X)
//  Cancel Request	(Y,X)
//  Cancel Request	(Z,X)
//  Cancel Request	(Q,X)
//  Partial Fill	(X)
//  Pending	Cxl		(Y,X)
//  Partial Fill	(X)
//  Pending	Cxl		(Z,X)
//  Pending	Cxl		(Q,X)
//	Cancel Reject	(Z,X)
//	Cancel Reject	(Q,X)
//	Partial Fill	(X)
//	Cancel Reject	(Y,X)
TEST_F(TestSuite, nos_cxl_cxl_cxl_fill_rjt_rjt_fill_rjt) {
	auto idX = new_order();
	ack(idX);
	fill(idX, 2000);
	//cancel request
	auto idY = cxl_order(idX);
	auto idZ = cxl_order(idX);
	auto idQ = cxl_order(idX);
	fill(idX, 2000);
	pending_cancel(idY, idX);
	fill(idX, 1000);
	pending_cancel(idZ, idX);
	pending_cancel(idQ, idX);
	cxl_rjt(idZ, idX, reason);
	cxl_rjt(idQ, idX, reason);
	fill(idX, 1500);
	cxl_rjt(idY, idX, reason);
}
