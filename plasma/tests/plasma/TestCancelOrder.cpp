#include "TestSuite.h"

///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Cancel Request	(Y,X)
//	Cancel Reject	(Y,X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.a.html
TEST_F(TestSuite, nos_cxl_rjt) {
	auto idX = new_order(10000);

	//cancel request
	auto idY = idX.cancel_order();
	idY.reject(reason);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Cancel Request	(Y,X)
//  Pending Ack		(X)
//	Cancel Reject	(Y,X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.d.html
TEST_F(TestSuite, nos_pnd_cxl_rjt) {
	auto idX = new_order(10000);

	//cancel request
	auto idY = idX.cancel_order();
	idX.pending();
	idY.reject(reason);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Cancel Request	(Y,X)
//  Ack				(X)
//	Cancel Reject	(Y,X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.d.html
TEST_F(TestSuite, nos_cxl_ack_rjt) {
	auto idX = new_order(10000);

	//cancel request
	auto idY = idX.cancel_order();
	idX.accept();
	idY.reject(reason);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Ack				(X)
//  Cancel Request	(Y,X)
//	Cancel Reject	(Y,X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.a.html
TEST_F(TestSuite, nos_ack_cxl_rjt) {
	auto idX = new_order(10000);
	idX.accept();

	//cancel request
	auto idY = idX.cancel_order();
	idY.reject(reason);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Ack				(X)
//	Fill			(X)
//  Cancel Request	(Y,X)
//	Cancel Reject	(Y,X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.a.html
TEST_F(TestSuite, nos_ack_fill_cxl_rjt) {
	auto idX = new_order(10000);
	idX.accept();
	idX.fill(2000);

	//cancel request
	auto idY = idX.cancel_order();
	idY.reject(reason);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Ack				(X)
//  Cancel Request	(Y,X)
//  Pending Cancel	(Y,X)
//	Cancel Reject	(Y,X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.a.html
TEST_F(TestSuite, nos_ack_cxl_pnd_rjt) {
	auto idX = new_order(10000);
	idX.accept();

	//cancel request
	auto idY = idX.cancel_order();
	idY.pending();
	idY.reject(reason);
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
	auto idX = new_order(10000);
	idX.accept();
	idX.fill(2000);

	//cancel request
	auto idY = idX.cancel_order();
	idY.pending();
	idY.reject(reason);
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
	auto idX = new_order(10000);
	idX.accept();
	idX.fill(2000);

	//cancel request
	auto idY = idX.cancel_order();
	idX.fill(3000);
	idY.pending();
	idX.fill(2000);
	idY.reject(reason);
	idX.fill(3000);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Ack				(X)
//	Fill			(X)
//  Cancel Request	(Y,X)
//	Canceled		(Y,X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.a.html
TEST_F(TestSuite, nos_ack_cxl_cxld) {
	auto idX = new_order(10000);
	idX.accept();
	idX.fill(2000);

	//cancel request
	auto idY = idX.cancel_order();
	// accept cancel
	idY.accept();
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Ack				(X)
//  Cancel Request	(Y,X)
//	Pending Cxl		(Y,X)
//	Canceled		(Y,X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.a.html
TEST_F(TestSuite, nos_ack_cxl_pnd_cxld) {
	auto idX = new_order(10000);
	idX.accept();
	idX.fill(2000);

	//cancel request
	auto idY = idX.cancel_order();
	idY.pending();
	idY.accept();
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
	auto idX = new_order(10000);
	idX.accept();
	idX.fill(2000);

	//cancel request
	auto idY = idX.cancel_order();
	idX.fill(3000);
	idY.pending();
	idX.fill(1000);
	idY.accept();
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Cancel Request	(Y,X)
//  Pending	Cxl		(Y,X)
//  Ack				(X)
//	Canceled		(Y,X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.d.html
TEST_F(TestSuite, nos_cxl_pnd_ack_cxld) {
	auto idX = new_order(10000);

	//cancel request
	auto idY = idX.cancel_order();
	idY.pending();
	idX.accept();
	idY.accept();
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Cancel Request	(Y,X)
//  Pending	Cxl		(Y,X)
//	Canceled		(Y,X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.d.html
TEST_F(TestSuite, nos_cxl_pnd_cxld) {
	auto idX = new_order(10000);

	//cancel request
	auto idY = idX.cancel_order();
	idY.pending();
	idY.accept();
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Cancel Request	(Y,X)
//  Ack				(X)
//  Pending	Cxl		(Y,X)
//	Canceled		(Y,X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.e.html
TEST_F(TestSuite, nos_cxl_ack_pnd_cxld) {
	auto idX = new_order(10000);

	//cancel request
	auto idY = idX.cancel_order();
	idX.accept();
	idY.pending();
	idY.accept();
}
///////////////////////////////////////////////////////////////
//  Cancel Request	(Y,X)
//	Cancel Reject	(Y,X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.f.html
TEST_F(TestSuite, cxl_rjt_unknwn) {
	auto idX = 929;

	//cancel request on unsent order
	// plasma will reject it automatically
//	auto idY = idX.cancel_order();
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
	auto idX = new_order(10000);
	idX.accept();
	idX.fill(2000);

	//cancel request
	auto idY = idX.cancel_order();
	auto idZ = idX.cancel_order();
	idX.fill(3000);
	idY.pending();
	idZ.pending();
	idX.fill(1000);
	idY.reject(reason);
	idX.fill(1500);
	idZ.reject(reason);
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
	auto idX = new_order(10000);
	idX.accept();
	idX.fill(2000);

	//cancel request
	auto idY = idX.cancel_order();
	auto idZ = idX.cancel_order();
	idX.fill(3000);
	idY.pending();
	idZ.pending();
	idX.fill(1000);
	idY.accept();
	idZ.reject(reason);
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
	auto idX = new_order(10000);
	idX.accept();
	idX.fill(2000);
	//cancel request
	auto idY = idX.cancel_order();
	auto idZ = idX.cancel_order();
	auto idQ = idX.cancel_order();
	idX.fill(2000);
	idY.pending();
	idX.fill(1000);
	idZ.pending();
	idQ.pending();
	idZ.reject(reason);
	idQ.reject(reason);
	idX.fill(1500);
	idY.reject(reason);
}
