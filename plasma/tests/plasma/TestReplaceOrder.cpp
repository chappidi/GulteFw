#include "TestSuite.h"

///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Replace Request	(Y,X)
//	Replace Reject	(Y,X)
// https://www.onixs.biz/fix-dictionary/4.4/app_dC.1.a.html
TEST_F(TestSuite, nos_rpl_rjt) {
	auto idX = new_order(10000);

	//replace request
	auto idY = idX.replace_order(11000);
	idY.reject(reason);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Replace Request	(Y,X)
//  Pending Ack		(X)
//	Replace Reject	(Y,X)
// https://www.onixs.biz/fix-dictionary/4.4/app_dC.1.a.html
TEST_F(TestSuite, nos_pnd_rpl_rjt) {
	auto idX = new_order(10000);

	//replace request
	auto idY = idX.replace_order(11000);
	idX.pending();
	idY.reject(reason);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Ack				(X)
//  Replace Request	(Y,X)
//	Replace Reject	(Y,X)
// https://www.onixs.biz/fix-dictionary/4.4/app_dC.1.a.html
TEST_F(TestSuite, nos_rpl_ack_rjt) {
	auto idX = new_order(10000);

	//replace request
	auto idY = idX.replace_order(11000);
	idX.accept();
	idY.reject(reason);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Ack				(X)
//  Replace Request	(Y,X)
//	Replace Reject	(Y,X)
// https://www.onixs.biz/fix-dictionary/4.4/app_dC.1.a.html
TEST_F(TestSuite, nos_ack_rpl_rjt) {
	auto idX = new_order(10000);
	idX.accept();

	//replace request
	auto idY = idX.replace_order(11000);
	idY.reject(reason);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Ack				(X)
//	Partial Fill	(X)
//  Replace Request	(Y,X)
//	Replace Reject	(Y,X)
// https://www.onixs.biz/fix-dictionary/4.4/app_dC.1.a.html
TEST_F(TestSuite, nos_ack_fill_rpl_rjt) {
	auto idX = new_order(10000);
	idX.accept();
	idX.fill(2000);

	//replace request
	auto idY = idX.replace_order(11000);
	idY.reject(reason);
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Ack				(X)
//  Replace Request	(Y,X)
//  Pending Replace (Y,X)
//	Replace Reject	(Y,X)
// https://www.onixs.biz/fix-dictionary/4.4/app_dC.1.a.html
TEST_F(TestSuite, nos_ack_rpl_pnd_rjt) {
	auto idX = new_order(10000);
	idX.accept();

	//replace request
	auto idY = idX.replace_order(11000);
	idY.pending();
	idY.reject(reason);
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
	auto idX = new_order(10000);
	idX.accept();
	idX.fill(2000);

	//replace request
	auto idY = idX.replace_order(11000);
	idY.pending();
	idY.reject(reason);
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
	auto idX = new_order(10000);
	idX.accept();
	idX.fill(2000);

	//replace request
	auto idY = idX.replace_order(11000);
	idX.fill(3000);
	idY.pending();
	idX.fill(5000);
	idY.reject(reason);
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
	auto idX = new_order(10000);
	idX.accept();

	//replace request
	auto idY = idX.replace_order(11000);
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
	auto idX = new_order(10000);
	idX.accept();
	idX.fill(2000);

	//replace request
	auto idY = idX.replace_order(11000);
	idY.pending();
	idX.fill(3000);
	idY.accept();
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
//  Replace Request	(Z,Y)
//	Pending Replace	(Z,Y)
//  Partial Fill	(Y)
//	Replaced		(Z,Y)
//  Partial Fill	(Z)
// https://www.onixs.biz/fix-dictionary/4.4/app_dD.1.a.html
TEST_F(TestSuite, nos_rpl_rpld_rpl_rpld) {
	auto idX = new_order(10000);
	idX.accept();
	idX.fill(2000);

	//replace request
	auto idY = idX.replace_order(8000);
	idY.pending();
	idX.fill(1000);
	idY.accept();
	idY.fill(1500);

	//replace request
	auto idZ = idY.replace_order(7000);
	idZ.pending();
	idY.fill(1000);
	idZ.accept();
	idZ.fill(1500);

	//cancel request
	auto idQ = idZ.cancel_order();
	idQ.pending();
	idZ.fill(500);
	idQ.accept();
}
