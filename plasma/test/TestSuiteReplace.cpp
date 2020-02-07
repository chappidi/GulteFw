#include "TestSuiteN.h"

///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Replace Request	(Y,X)
//	Replace Reject	(Y,X)
// https://www.onixs.biz/fix-dictionary/4.4/app_dC.1.a.html
TEST_F(TestSuite, nos_rpl_rjt) {
	NewOrder idX(*this);

	//replace request
	ReplaceOrder idY(idX, 11000);
	idY.reject(reason);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Replace Request	(Y,X)
//  Pending Ack		(X)
//	Replace Reject	(Y,X)
// https://www.onixs.biz/fix-dictionary/4.4/app_dC.1.a.html
TEST_F(TestSuite, nos_pnd_rpl_rjt) {
	auto idX = new_order();

	//replace request
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
TEST_F(TestSuite, nos_rpl_ack_rjt) {
	auto idX = new_order();

	//replace request
	auto idY = rpl_order(idX, 11000);
	ack(idX);
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

	//replace request
	auto idY = rpl_order(idX, 11000);
	rpl_rjt(idY, idX, reason);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Ack				(X)
//	Partial Fill	(X)
//  Replace Request	(Y,X)
//	Replace Reject	(Y,X)
// https://www.onixs.biz/fix-dictionary/4.4/app_dC.1.a.html
TEST_F(TestSuite, nos_ack_fill_rpl_rjt) {
	auto idX = new_order();
	ack(idX);
	fill(idX, 2000);

	//replace request
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

	//replace request
	auto idY = rpl_order(idX, 11000);
	pending_replace(idY, idX);
	rpl_rjt(idY, idX, reason);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Ack				(X)
//	Partial Fill	(X)
//  Replace Request	(Y,X)
//  Pending Replace (Y,X)
//	Replace Reject	(Y,X)
// https://www.onixs.biz/fix-dictionary/4.4/app_dC.1.a.html
TEST_F(TestSuite, nos_ack_fill_rpl_pnd_rjt) {
	auto idX = new_order();
	ack(idX);
	fill(idX, 2000);

	//replace request
	auto idY = rpl_order(idX, 11000);
	pending_replace(idY, idX);
	rpl_rjt(idY, idX, reason);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Ack				(X)
//	Partial Fill	(X)
//  Replace Request	(Y,X)
//	Partial Fill	(X)
//  Pending Replace (Y,X)
//	Filled			(X)
//	Replace Reject	(Y,X)
// https://www.onixs.biz/fix-dictionary/4.4/app_dC.1.b.html
TEST_F(TestSuite, nos_ack_fill_rpl_fill_pnd_fill_rjt) {
	auto idX = new_order();
	ack(idX);
	fill(idX, 2000);

	//replace request
	auto idY = rpl_order(idX, 11000);
	fill(idX, 3000);
	pending_replace(idY, idX);
	fill(idX, 5000);
	rpl_rjt(idY, idX, reason);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Ack				(X)
//  Replace Request	(Y,X)
//	Pending Replace	(Y,X)
//	Replaced		(Y,X)
//  Partial Fill	(Y)
//  Partial Fill	(Y)
// https://www.onixs.biz/fix-dictionary/4.4/app_dC.1.a.html
TEST_F(TestSuite, nos_ack_rpl_pnd_rpld_fill) {
	NewOrder idX(*this);
	idX.accept();

	//replace request
	ReplaceOrder idY(idX, 11000);
	idY.pending();
	idY.accept();
	idY.fill(1000);
	idY.fill(5000);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Ack				(X)
//  Partial Fill	(X)
//  Replace Request	(Y,X)
//	Pending Replace	(Y,X)
//  Partial Fill	(X)
//	Replaced		(Y,X)
//  Partial Fill	(Y)
//  Partial Fill	(Y)
// https://www.onixs.biz/fix-dictionary/4.4/app_dC.1.b.html
TEST_F(TestSuite, nos_ack_fill_rpl_fill_rpld_fill_fill) {
	NewOrder idX(*this);
	idX.accept();
	idX.fill(2000);

	//replace request
	ReplaceOrder idY(idX, 11000);
	idY.pending();
	idX.fill(3000);
	idY.accept();
	idY.fill(5000);
}