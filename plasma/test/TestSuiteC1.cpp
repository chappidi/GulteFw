#include <plasma.h>
#include "GUI.h"
#include "EPA.h"
#include <gtest/gtest.h>

struct TestSuiteC1 : public testing::Test
{
	// declare variables
	plasma::OMS plasma;
	GUI clt;
	EPA epa;
	TestSuiteC1() {
		plasma.OnLogin(clt);
		plasma.OnLogin(epa);
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//  Ack				(X)
	//  Replace Request	(Y,X)
	//	Pending Replace	(Y,X)
	//	Replace Reject	(Y,X)
	// https://www.onixs.biz/fix-dictionary/4.4/app_dC.1.a.html
	void nos_new_rpl_pnd_rjt() {
		// Send Order
		auto nos = clt.get_nos(epa.id(), 10000);
		plasma.OnMsg(nos);
		auto idX = epa.ClOrdId;
		std::cout << "[" << ClientId(nos.clOrdId()) << "-->" << idX << "]" << std::endl;
		// ack order
		plasma.OnMsg(epa.get_new(idX));
		assert(clt.exe.execType() == ExecType::New && clt.exe.ordStatus() == OrdStatus::New);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == nos.qty() && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);

		//replace request
		auto orr = clt.get_rpl(idX, 11000, nos);
		plasma.OnMsg(orr);
		auto idY = epa.ClOrdId;
		std::cout << "[" << ClientId(orr.clOrdId()) << "-->" << idY << "]" << std::endl;
		// pending rpl ack
		plasma.OnMsg(epa.get_pnd_rpl(idY, idX));
		assert(clt.exe.execType() == ExecType::Pending_Replace && clt.exe.ordStatus() == OrdStatus::Pending_Replace);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == orr.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == nos.qty() && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// reject
		plasma.OnMsg(epa.get_rjt(idY, idX, ""));
		assert(clt.rjt.status() == OrdStatus::New);
		assert(clt.rjt.origClOrdId() == nos.clOrdId() && clt.rjt.clOrdId() == orr.clOrdId() && clt.rjt.orderId() == idX);
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
	void nos_new_rpl_pnd_rpld_fill() {
		// Send Order
		auto nos = clt.get_nos(epa.id(), 10000);
		plasma.OnMsg(nos);
		auto idX = epa.ClOrdId;
		std::cout << "[" << ClientId(nos.clOrdId()) << "-->" << idX << "]" << std::endl;
		// ack order
		plasma.OnMsg(epa.get_new(idX));
		assert(clt.exe.execType() == ExecType::New && clt.exe.ordStatus() == OrdStatus::New);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == nos.qty() && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);

		//replace request
		auto orr = clt.get_rpl(idX, 11000, nos);
		plasma.OnMsg(orr);
		auto idY = epa.ClOrdId;
		std::cout << "[" << ClientId(orr.clOrdId()) << "-->" << idY << "]" << std::endl;
		// pending rpl ack
		plasma.OnMsg(epa.get_pnd_rpl(idY, idX));
		assert(clt.exe.execType() == ExecType::Pending_Replace && clt.exe.ordStatus() == OrdStatus::Pending_Replace);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == orr.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == nos.qty() && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// replaced
		plasma.OnMsg(epa.get_rpld(idY, idX));
		assert(clt.exe.execType() == ExecType::Replace && clt.exe.ordStatus() == OrdStatus::New);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == orr.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == orr.qty() && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// fill 1000
		plasma.OnMsg(epa.get_fill(idY, 1000, 99.98));
		assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == OrdStatus::Partially_Filled);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == orr.clOrdId() && clt.exe.orderId() == idY);
		assert(clt.exe.leavesQty() == 10000 && clt.exe.cumQty() == 1000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 1000 && clt.exe.lastPx() == 99.98);
		// fill 2000
		plasma.OnMsg(epa.get_fill(idY, 2000, 99.98));
		assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == OrdStatus::Partially_Filled);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == orr.clOrdId() && clt.exe.orderId() == idY);
		assert(clt.exe.leavesQty() == 8000 && clt.exe.cumQty() == 3000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 2000 && clt.exe.lastPx() == 99.98);
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//  Ack				(X)
	//  Partial Fill	(X)
	//  Replace Request	(Y,X)
	//	Pending Replace	(Y,X)
	//  Partial Fill	(Y)
	//	Replace Reject	(Y,X)
	// https://www.onixs.biz/fix-dictionary/4.4/app_dC.1.b.html
	void nos_new_fill_rpl_fill_rjt() {
		// Send Order
		auto nos = clt.get_nos(epa.id(), 10000);
		plasma.OnMsg(nos);
		auto idX = epa.ClOrdId;
		std::cout << "[" << ClientId(nos.clOrdId()) << "-->" << idX << "]" << std::endl;
		// ack order
		plasma.OnMsg(epa.get_new(idX));
		assert(clt.exe.execType() == ExecType::New && clt.exe.ordStatus() == OrdStatus::New);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == nos.qty() && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// fill 1000
		plasma.OnMsg(epa.get_fill(idX, 1000, 99.98));
		assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == OrdStatus::Partially_Filled);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 9000 && clt.exe.cumQty() == 1000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 1000 && clt.exe.lastPx() == 99.98);

		//replace request
		auto orr = clt.get_rpl(idX, 12000, nos);
		plasma.OnMsg(orr);
		auto idY = epa.ClOrdId;
		std::cout << "[" << ClientId(orr.clOrdId()) << "-->" << idY << "]" << std::endl;
		// pending rpl ack
		plasma.OnMsg(epa.get_pnd_rpl(idY, idX));
		assert(clt.exe.execType() == ExecType::Pending_Replace && clt.exe.ordStatus() == OrdStatus::Pending_Replace);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == orr.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 9000 && clt.exe.cumQty() == 1000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// fill 100
		plasma.OnMsg(epa.get_fill(idX, 100, 99.98));
		assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == OrdStatus::Partially_Filled);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 8900 && clt.exe.cumQty() == 1100 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 100 && clt.exe.lastPx() == 99.98);
		// reject
		plasma.OnMsg(epa.get_rjt(idY, idX, ""));
		assert(clt.rjt.status() == OrdStatus::Partially_Filled);
		assert(clt.rjt.origClOrdId() == nos.clOrdId() && clt.rjt.clOrdId() == orr.clOrdId() && clt.rjt.orderId() == idX);
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
	void nos_new_fill_rpl_fill_rpld_fill_fill() {
		// Send Order
		auto nos = clt.get_nos(epa.id(), 10000);
		plasma.OnMsg(nos);
		auto idX = epa.ClOrdId;
		std::cout << "[" << ClientId(nos.clOrdId()) << "-->" << idX << "]" << std::endl;
		// ack order
		plasma.OnMsg(epa.get_new(idX));
		assert(clt.exe.execType() == ExecType::New && clt.exe.ordStatus() == OrdStatus::New);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == nos.qty() && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// fill 1000
		plasma.OnMsg(epa.get_fill(idX, 1000, 99.98));
		assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == OrdStatus::Partially_Filled);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 9000 && clt.exe.cumQty() == 1000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 1000 && clt.exe.lastPx() == 99.98);

		// replace request (qty = 12000)
		auto orr = clt.get_rpl(idX, 12000, nos);
		plasma.OnMsg(orr);
		auto idY = epa.ClOrdId;
		std::cout << "[" << ClientId(orr.clOrdId()) << "-->" << idY << "]" << std::endl;
		// pending rpl ack
		plasma.OnMsg(epa.get_pnd_rpl(idY, idX));
		assert(clt.exe.execType() == ExecType::Pending_Replace && clt.exe.ordStatus() == OrdStatus::Pending_Replace);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == orr.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 9000 && clt.exe.cumQty() == 1000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// fill 100
		plasma.OnMsg(epa.get_fill(idX, 100, 99.98));
		assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == OrdStatus::Partially_Filled);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 8900 && clt.exe.cumQty() == 1100 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 100 && clt.exe.lastPx() == 99.98);
		// replaced
		plasma.OnMsg(epa.get_rpld(idY, idX));
		assert(clt.exe.execType() == ExecType::Replace && clt.exe.ordStatus() == OrdStatus::Partially_Filled);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == orr.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 10900 && clt.exe.cumQty() == 1100 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// fill 1000
		plasma.OnMsg(epa.get_fill(idY, 1000, 99.98));
		assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == OrdStatus::Partially_Filled);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == orr.clOrdId() && clt.exe.orderId() == idY);
		assert(clt.exe.leavesQty() == 9900 && clt.exe.cumQty() == 2100 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 1000 && clt.exe.lastPx() == 99.98);
		// fill 2000
		plasma.OnMsg(epa.get_fill(idY, 2000, 99.98));
		assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == OrdStatus::Partially_Filled);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == orr.clOrdId() && clt.exe.orderId() == idY);
		assert(clt.exe.leavesQty() == 7900 && clt.exe.cumQty() == 4100 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 2000 && clt.exe.lastPx() == 99.98);
	}
};
TEST_F(TestSuiteC1, nos_new_rpl_pnd_rjt) {
	nos_new_rpl_pnd_rjt();
}
TEST_F(TestSuiteC1, nos_new_rpl_pnd_rpld_fill) {
	nos_new_rpl_pnd_rpld_fill();
}
TEST_F(TestSuiteC1, nos_new_fill_rpl_fill_rjt) {
	nos_new_fill_rpl_fill_rjt();
}
TEST_F(TestSuiteC1, nos_new_fill_rpl_fill_rpld_fill_fill) {
	nos_new_fill_rpl_fill_rpld_fill_fill();
}
TEST_F(TestSuiteC1, all) {
	nos_new_rpl_pnd_rjt();
	nos_new_rpl_pnd_rpld_fill();
	nos_new_fill_rpl_fill_rjt();
	nos_new_fill_rpl_fill_rpld_fill_fill();
}
