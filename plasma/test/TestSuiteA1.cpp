#include <gtest/gtest.h>
#include <plasma.h>
#include "GUI.h"
#include "EPA.h"
struct TestSuiteA1 : public testing::Test {

	// declare variables
	plasma::OMS plasma;
	GUI clt;
	EPA epa;
	TestSuiteA1() {
		plasma.OnLogin(clt);
		plasma.OnLogin(epa);
	}
};
//https://www.onixs.biz/fix-dictionary/4.4/app_dA.1.a.html

TEST_F(TestSuiteA1, test_nos_rjt) {
	// New Order(X)	
	auto nos = clt.get_nos(epa.id(), 10000);
	plasma.OnMsg(nos);
	auto idX = epa.ClOrdId;
	std::cout << "[" << ClientId(nos.clOrdId()) << "-->" << idX << "]" << std::endl;
	// reject
	plasma.OnMsg(epa.get_rjt(idX));
	EXPECT_TRUE(clt.nfr.status() == OrdStatus::Rejected);
	EXPECT_TRUE(clt.nfr.origClOrdId() == 0 && clt.nfr.clOrdId() == nos.clOrdId() && clt.nfr.orderId() == idX);
	EXPECT_TRUE(clt.nfr.leavesQty() == 0 && clt.nfr.cumQty() == 0 && clt.nfr.avgPx() == 0);
}

TEST_F(TestSuiteA1, test_nos_pnd_new_rjt) {
	// New Order(X)	
	auto nos = clt.get_nos(epa.id(), 10000);
	plasma.OnMsg(nos);
	auto idX = epa.ClOrdId;
	std::cout << "[" << ClientId(nos.clOrdId()) << "-->" << idX << "]" << std::endl;
	// pending ack order
	plasma.OnMsg(epa.get_pnd_new(idX));
	EXPECT_TRUE(clt.nfr.status() == OrdStatus::Pending_New);
	EXPECT_TRUE(clt.nfr.origClOrdId() == 0 && clt.nfr.clOrdId() == nos.clOrdId() && clt.nfr.orderId() == idX);
	EXPECT_TRUE(clt.nfr.leavesQty() == 10000 && clt.nfr.cumQty() == 0 && clt.nfr.avgPx() == 0);
	// reject
	plasma.OnMsg(epa.get_rjt(idX));
	EXPECT_TRUE(clt.nfr.status() == OrdStatus::Rejected);
	EXPECT_TRUE(clt.nfr.origClOrdId() == 0 && clt.nfr.clOrdId() == nos.clOrdId() && clt.nfr.orderId() == idX);
	EXPECT_TRUE(clt.nfr.leavesQty() == 0 && clt.nfr.cumQty() == 0 && clt.nfr.avgPx() == 0);
}
