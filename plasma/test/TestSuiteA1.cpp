#include "TestSuite.h"
struct TestSuiteA1 : public TestSuite 
{
	TestSuiteA1() {
		//empty
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle (X)
	//  Reject
	//	https://www.onixs.biz/fix-dictionary/4.4/app_dA.1.a.html
	void nos_rjt() {
		// New Order(X)	
		auto nos = clt.get_nos(epa.id(), 10000);
		plasma.OnMsg(nos);
		auto idX = epa.ClOrdId;
		std::cout << "[" << ClientId(nos.clOrdId()) << "-->" << idX << "]" << std::endl;
		// reject
		plasma.OnMsg(epa.get_rjt(idX));
		VALIDATE_REJECT(nos, idX, clt.nfr);
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//  Pending Ack		(X)
	//	Reject			(X)
	//	https://www.onixs.biz/fix-dictionary/4.4/app_dA.1.a.html
	void nos_pnd_new_rjt() {
		// New Order(X)	
		auto nos = clt.get_nos(epa.id(), 10000);
		plasma.OnMsg(nos);
		auto idX = epa.ClOrdId;
		std::cout << "[" << ClientId(nos.clOrdId()) << "-->" << idX << "]" << std::endl;
		// pending ack order
		plasma.OnMsg(epa.get_pnd_new(idX));
		VALIDATE_PENDING_NEW(nos, idX, clt.nfr);
		// reject
		plasma.OnMsg(epa.get_rjt(idX));
		VALIDATE_REJECT(nos, idX, clt.nfr);
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle (X)
	//  Pending Ack & Ack & Reject
	//	https://www.onixs.biz/fix-dictionary/4.4/app_dA.1.a.html
	void nos_pnd_new_new_rjt() {
		// New Order(X)	
		auto nos = clt.get_nos(epa.id(), 10000);
		plasma.OnMsg(nos);
		auto idX = epa.ClOrdId;
		std::cout << "[" << ClientId(nos.clOrdId()) << "-->" << idX << "]" << std::endl;
		// pending ack order
		plasma.OnMsg(epa.get_pnd_new(idX));
		VALIDATE_PENDING_NEW(nos, idX, clt.nfr);
		// ack order
		plasma.OnMsg(epa.get_new(idX));
		VALIDATE_NEW(nos, idX, clt.nfr);
		// reject
		plasma.OnMsg(epa.get_rjt(idX));
		VALIDATE_REJECT(nos, idX, clt.nfr);
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle (X)
	//  Ack & Fill
	//	https://www.onixs.biz/fix-dictionary/4.4/app_dA.1.a.html
	void nos_new_fill() {
		// New Order(X)	
		auto nos = clt.get_nos(epa.id(), 10000);
		plasma.OnMsg(nos);
		auto idX = epa.ClOrdId;
		std::cout << "[" << ClientId(nos.clOrdId()) << "-->" << idX << "]" << std::endl;
		// ack order
		plasma.OnMsg(epa.get_new(idX));
		VALIDATE_NEW(nos, idX, clt.nfr);
		// fill 2000
		plasma.OnMsg(epa.get_fill(idX, 2000, 99.98));
		VALIDATE_FILL(nos, idX, 2000, 99.98, clt.fill);
		EXPECT_TRUE(clt.fill.leavesQty() == 8000 && clt.fill.cumQty() == 2000 && clt.fill.avgPx() == 99.98);
		// fill 1000
		plasma.OnMsg(epa.get_fill(idX, 1000, 99.98));
		VALIDATE_FILL(nos, idX, 1000, 99.98, clt.fill);
		EXPECT_TRUE(clt.fill.leavesQty() == 7000 && clt.fill.cumQty() == 3000 && clt.fill.avgPx() == 99.98);
		//  fill 7000
		plasma.OnMsg(epa.get_fill(idX, 7000, 99.98));
		VALIDATE_FILLED(nos, idX, 7000, 99.98, clt.fill);
		EXPECT_TRUE(clt.fill.leavesQty() == 0 && clt.fill.cumQty() == 10000 && clt.fill.avgPx() == 99.98);
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle (X)
	//  Ack & Partial Fill & Done
	//	https://www.onixs.biz/fix-dictionary/4.4/app_dA.1.b.html
	void nos_new_fill_done() {
		auto nos = clt.get_nos(epa.id(), 10000);
		plasma.OnMsg(nos);
		auto idX = epa.ClOrdId;
		std::cout << "[" << ClientId(nos.clOrdId()) << "-->" << idX << "]" << std::endl;
		// ack order
		plasma.OnMsg(epa.get_new(idX));
		VALIDATE_NEW(nos, idX, clt.nfr);
		// fill 2000
		plasma.OnMsg(epa.get_fill(idX, 2000, 99.98));
		VALIDATE_FILL(nos, idX, 2000, 99.98, clt.fill);
		EXPECT_TRUE(clt.fill.leavesQty() == 8000 && clt.fill.cumQty() == 2000 && clt.fill.avgPx() == 99.98);
		// fill 1000
		plasma.OnMsg(epa.get_fill(idX, 1000, 99.98));
		VALIDATE_FILL(nos, idX, 1000, 99.98, clt.fill);
		EXPECT_TRUE(clt.fill.leavesQty() == 7000 && clt.fill.cumQty() == 3000 && clt.fill.avgPx() == 99.98);
		// done for day
		plasma.OnMsg(epa.get_done(idX));
		EXPECT_TRUE(clt.nfr.status() == OrdStatus::Done_For_Day);
		EXPECT_TRUE(clt.nfr.origClOrdId() == 0 && clt.nfr.clOrdId() == nos.clOrdId() && clt.nfr.orderId() == idX);
		EXPECT_TRUE(clt.nfr.leavesQty() == 0 && clt.nfr.cumQty() == 3000 && clt.nfr.avgPx() == 99.98);
	}
};

TEST_F(TestSuiteA1, nos_rjt) {
	nos_rjt();
}
TEST_F(TestSuiteA1, test_nos_pnd_new_rjt) {
	nos_pnd_new_rjt();
}
TEST_F(TestSuiteA1, nos_pnd_new_new_rjt) {
	nos_pnd_new_new_rjt();
}
TEST_F(TestSuiteA1, nos_new_fill) {
	nos_new_fill();
}
TEST_F(TestSuiteA1, nos_new_fill_done) {
	nos_new_fill_done();
}
