#include <plasma.h>
#include "GUI.h"
#include "EPA.h"
#include <gtest/gtest.h>

struct TestSuiteG1 : public testing::Test
{
	// declare variables
	plasma::OMS plasma;
	GUI clt;
	EPA epa;
	TestSuiteG1() {
		plasma.OnLogin(clt);
		plasma.OnLogin(epa);
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//	Ack				(X)
	//	Fill			(X)
	//	Status Request	(Y)
	//	https://www.onixs.biz/fix-dictionary/4.4/app_dG.1.a.html
	void nos_sts_unkown() {
		auto unknwn = clt.get_nos(epa.id(), 10000);
		// Send Order
		auto nos = clt.get_nos(epa.id(), 10000);
		plasma.OnMsg(nos);
		auto idX = epa.ClOrdId;
		std::cout << "[" << ClientId(nos.clOrdId()) << "-->" << idX << "]" << std::endl;
		// ack order
		plasma.OnMsg(epa.get_new(idX));
		// partial fill 2000
		plasma.OnMsg(epa.get_fill(idX, 2000, 99.98));

		// request status
		plasma.OnMsg(clt.get_sts(unknwn));
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == OrdStatus::Rejected);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == unknwn.clOrdId() && clt.exe.orderId() != idX);
		assert(clt.exe.leavesQty() == 0 && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//	Ack				(X)
	//	Status Request	(X)
	//	https://www.onixs.biz/fix-dictionary/4.4/app_dG.1.b.html
	void nos_new_sts() {
		// Send Order
		auto nos = clt.get_nos(epa.id(), 10000);
		plasma.OnMsg(nos);
		auto idX = epa.ClOrdId;
		std::cout << "[" << ClientId(nos.clOrdId()) << "-->" << idX << "]" << std::endl;
		// ack
		plasma.OnMsg(epa.get_new(idX));

		// request status
		plasma.OnMsg(clt.get_sts(nos));
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == OrdStatus::New);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == nos.qty() && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//	Status Request	(X)
	//	Ack				(X)
	//	Status Request	(X)
	//	Fill			(X)
	//	Status Request	(X)
	//	Fill All		(X)
	//	Status Request	(X)
	//	Replace Request	(Y,X)
	//	Pending Replace	(Y,X)
	//	Replaced		(Y,X)
	//	Status Request	(X)
	//	Status Request	(Y)
	//	https://www.onixs.biz/fix-dictionary/4.4/app_dG.1.c.html
	void nos_rpl_sts_multi() {
		// Send Order
		auto nos = clt.get_nos(epa.id(), 10000);
		plasma.OnMsg(nos);
		auto idX = epa.ClOrdId;
		std::cout << "[" << ClientId(nos.clOrdId()) << "-->" << idX << "]" << std::endl;
		auto osr = clt.get_sts(nos);
		// check status
		plasma.OnMsg(osr);
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == OrdStatus::NA);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 10000 && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		//ack order
		plasma.OnMsg(epa.get_new(idX));
		// status check
		plasma.OnMsg(osr);
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == OrdStatus::New);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 10000 && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// parital fill of 2000
		plasma.OnMsg(epa.get_fill(idX, 2000, 99.98));
		// status check
		plasma.OnMsg(osr);
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == OrdStatus::Partially_Filled);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 8000 && clt.exe.cumQty() == 2000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// fill all
		plasma.OnMsg(epa.get_fill(idX, 8000, 99.98));
		// status check
		plasma.OnMsg(osr);
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == OrdStatus::Filled);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 0 && clt.exe.cumQty() == 10000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);

		// replace request (qty = 12000)
		auto orr = clt.get_rpl(idX, 12000, nos);
		plasma.OnMsg(orr);
		auto idY = epa.ClOrdId;
		std::cout << "[" << ClientId(orr.clOrdId()) << "-->" << idY << "]" << std::endl;
		// pending rpl ack
		plasma.OnMsg(epa.get_pnd_rpl(idY, idX));
		assert(clt.exe.execType() == ExecType::Pending_Replace && clt.exe.ordStatus() == OrdStatus::Pending_Replace);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == orr.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 0 && clt.exe.cumQty() == 10000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// replaced
		plasma.OnMsg(epa.get_rpld(idY, idX));
		assert(clt.exe.execType() == ExecType::Replace && clt.exe.ordStatus() == OrdStatus::Partially_Filled);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == orr.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 2000 && clt.exe.cumQty() == 10000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);

		// status check (X) -> idX
		plasma.OnMsg(osr);
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == OrdStatus::Partially_Filled);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == orr.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 2000 && clt.exe.cumQty() == 10000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);

		// status check (Y) -> idY
		auto osrY = clt.get_sts(orr);
		plasma.OnMsg(osrY);
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == OrdStatus::Partially_Filled);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == orr.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 2000 && clt.exe.cumQty() == 10000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
	}
};

TEST_F(TestSuiteG1, nos_sts_unkown) {
	nos_sts_unkown();
}
TEST_F(TestSuiteG1, nos_new_sts) {
	nos_new_sts();
}
TEST_F(TestSuiteG1, nos_rpl_sts_multi) {
	nos_rpl_sts_multi();
}

