#include "TestSuite.h"

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