#include <plasma.h>
#include "GUI.h"
#include "EPA.h"
#include <gtest/gtest.h>

struct TestSuiteD1 : public testing::Test
{
	// declare variables
	plasma::OMS plasma;
	GUI clt;
	EPA epa;
	TestSuiteD1() {
		plasma.OnLogin(clt);
		plasma.OnLogin(epa);
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
	void nos_rpl_rpld_rpl_rpld() {
		// Send Order
		auto nos = clt.get_nos(epa.id(), 10000);
		plasma.OnMsg(nos);
		auto idX = epa.ClOrdId;
		std::cout << "[" << ClientId(nos.clOrdId()) << "-->" << idX << "]" << std::endl;
		//ack order
		plasma.OnMsg(epa.get_new(idX));
		// parital fill of 1000
		plasma.OnMsg(epa.get_fill(idX, 1000, 99.98));

		// replace request (qty = 8000)
		auto orY = clt.get_rpl(idX, 8000, nos);
		plasma.OnMsg(orY);
		auto idY = epa.ClOrdId;
		std::cout << "[" << ClientId(orY.clOrdId()) << "-->" << idY << "]" << std::endl;
		// pending rpl ack
		plasma.OnMsg(epa.get_pnd_rpl(idY, idX));
		assert(clt.exe.execType() == ExecType::Pending_Replace && clt.exe.ordStatus() == OrdStatus::Pending_Replace);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == orY.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 9000 && clt.exe.cumQty() == 1000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// fill 500
		plasma.OnMsg(epa.get_fill(idX, 6000, 99.98));
		assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == OrdStatus::Pending_Replace );
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 0 && clt.exe.cumQty() == 7000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 500 && clt.exe.lastPx() == 99.98);
		// replaced
		plasma.OnMsg(epa.get_rpld(idY, idX));
		assert(clt.exe.execType() == ExecType::Replace && clt.exe.ordStatus() == OrdStatus::Partially_Filled);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == orY.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 7000 && clt.exe.cumQty() == 1000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// fill 2000
		plasma.OnMsg(epa.get_fill(idY, 6000, 99.98));
		assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == OrdStatus::Partially_Filled);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == orY.clOrdId() && clt.exe.orderId() == idY);
		assert(clt.exe.leavesQty() == 0 && clt.exe.cumQty() == 7000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 2000 && clt.exe.lastPx() == 99.98);


		// replace request (qty = 6000)
		auto orZ = clt.get_rpl(idY, 6000, orY);
		plasma.OnMsg(orZ);
		auto idZ = epa.ClOrdId;
		std::cout << "[" << ClientId(orZ.clOrdId()) << "-->" << idZ << "]" << std::endl;
		// pending rpl ack
		plasma.OnMsg(epa.get_pnd_rpl(idZ, idY));
		assert(clt.exe.execType() == ExecType::Pending_Replace && clt.exe.ordStatus() == OrdStatus::Pending_Replace);
		assert(clt.exe.origClOrdId() == orY.clOrdId() && clt.exe.clOrdId() == orZ.clOrdId() && clt.exe.orderId() == idY);
		assert(clt.exe.leavesQty() == 7000 && clt.exe.cumQty() == 1000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// fill 500
		plasma.OnMsg(epa.get_fill(idY, 6000, 99.98));
		assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == OrdStatus::Pending_Replace);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == orZ.clOrdId() && clt.exe.orderId() == idZ);
		assert(clt.exe.leavesQty() == 0 && clt.exe.cumQty() == 7000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 500 && clt.exe.lastPx() == 99.98);
		// replaced
		plasma.OnMsg(epa.get_rpld(idZ, idY));
		assert(clt.exe.execType() == ExecType::Replace && clt.exe.ordStatus() == OrdStatus::Partially_Filled);
		assert(clt.exe.origClOrdId() == orY.clOrdId() && clt.exe.clOrdId() == orZ.clOrdId() && clt.exe.orderId() == idY);
		assert(clt.exe.leavesQty() == 6000 && clt.exe.cumQty() == 1000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// fill 2000
		plasma.OnMsg(epa.get_fill(idZ, 6000, 99.98));
		assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == OrdStatus::Filled);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == orZ.clOrdId() && clt.exe.orderId() == idZ);
		assert(clt.exe.leavesQty() == 0 && clt.exe.cumQty() == 7000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 2000 && clt.exe.lastPx() == 99.98);
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//  Ack				(X)
	//  Partial Fill	(X)
	//  Replace Request	(Y,X)
	//	Replace	Reject	(Y,X)
	//  Partial Fill	(X)
	//  Replace Request	(Z,X)
	//	Pending Replace	(Z,X)
	//	Replaced		(Z,X)
	//  Partial Fill	(Z)
	// https://www.onixs.biz/fix-dictionary/4.4/app_dD.1.b.html
	void nos_rpl_rjt_rpl_rpld() {
		// Send Order
		auto nos = clt.get_nos(epa.id(), 10000);
		plasma.OnMsg(nos);
		auto idX = epa.ClOrdId;
		std::cout << "[" << ClientId(nos.clOrdId()) << "-->" << idX << "]" << std::endl;
		//ack order
		plasma.OnMsg(epa.get_new(idX));
		// parital fill of 1000
		plasma.OnMsg(epa.get_fill(idX, 1000, 99.98));
		// Replace 1 
		{
			// replace request (qty = 8000)
			auto orY = clt.get_rpl(idX, 8000, nos);
			plasma.OnMsg(orY);
			auto idY = epa.ClOrdId;
			std::cout << "[" << ClientId(orY.clOrdId()) << "-->" << idY << "]" << std::endl;
			// replace reject
			plasma.OnMsg(epa.get_rjt(idY, idX, "replace rejected"));
			assert(clt.rjt.status() == OrdStatus::Partially_Filled);
			assert(clt.rjt.origClOrdId() == nos.clOrdId() && clt.rjt.clOrdId() == orY.clOrdId() && clt.rjt.orderId() == idX);
			// fill 500
			plasma.OnMsg(epa.get_fill(idX, 6000, 99.98));
			assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == OrdStatus::Partially_Filled);
			assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
			assert(clt.exe.leavesQty() == 8500 && clt.exe.cumQty() == 1500 && clt.exe.avgPx() == 99.98);
			assert(clt.exe.lastQty() == 500 && clt.exe.lastPx() == 99.98);
			// fill 2000
			plasma.OnMsg(epa.get_fill(idX, 6000, 99.98));
			assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == OrdStatus::Partially_Filled);
			assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
			assert(clt.exe.leavesQty() == 6500 && clt.exe.cumQty() == 3500 && clt.exe.avgPx() == 99.98);
			assert(clt.exe.lastQty() == 2000 && clt.exe.lastPx() == 99.98);
		}
		// Replace 2
		{
			// replace request (qty = 6000)
			auto orZ = clt.get_rpl(idX, 6000, nos);
			plasma.OnMsg(orZ);
			auto idZ = epa.ClOrdId;
			std::cout << "[" << ClientId(orZ.clOrdId()) << "-->" << idZ << "]" << std::endl;
			// pending rpl ack
			plasma.OnMsg(epa.get_pnd_rpl(idZ, idX));
			assert(clt.exe.execType() == ExecType::Pending_Replace && clt.exe.ordStatus() == OrdStatus::Pending_Replace);
			assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == orZ.clOrdId() && clt.exe.orderId() == idX);
			assert(clt.exe.leavesQty() == 6500 && clt.exe.cumQty() == 3500 && clt.exe.avgPx() == 99.98);
			assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
			// replaced
			plasma.OnMsg(epa.get_rpld(idZ, idX));
			assert(clt.exe.execType() == ExecType::Replace && clt.exe.ordStatus() == OrdStatus::Partially_Filled);
			assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == orZ.clOrdId() && clt.exe.orderId() == idX);
			assert(clt.exe.leavesQty() == 6000 && clt.exe.cumQty() == 35000 && clt.exe.avgPx() == 99.98);
			assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
			// fill 1500
			plasma.OnMsg(epa.get_fill(idZ, 1500, 99.98));
			assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == OrdStatus::Filled);
			assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == orZ.clOrdId() && clt.exe.orderId() == idZ);
			assert(clt.exe.leavesQty() == 1000 && clt.exe.cumQty() == 5000 && clt.exe.avgPx() == 99.98);
			assert(clt.exe.lastQty() == 1500 && clt.exe.lastPx() == 99.98);
		}
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//  Ack				(X)
	//  Partial Fill	(X)
	//  Replace Request	(Y,X)
	//	Pending Replace	(Y,X)
	//  Partial Fill	(X)
	//	Replace Reject	(Y,X)
	//  Partial Fill	(X)
	//  Replace Request	(Z,X)
	//	Pending Replace	(Z,X)
	//  Partial Fill	(X)
	//	Replaced		(Z,X)
	//  Partial Fill	(Z)
	// https://www.onixs.biz/fix-dictionary/4.4/app_dD.1.c.html
	void nos_rpl_pnd_rjt_rpl_rpld() {
		// Send Order
		auto nos = clt.get_nos(epa.id(), 10000);
		plasma.OnMsg(nos);
		auto idX = epa.ClOrdId;
		std::cout << "[" << ClientId(nos.clOrdId()) << "-->" << idX << "]" << std::endl;
		//ack order
		plasma.OnMsg(epa.get_new(idX));
		// parital fill of 1000
		plasma.OnMsg(epa.get_fill(idX, 1000, 99.98));
		// Replace 1 
		{
			// replace request (qty = 8000)
			auto orY = clt.get_rpl(idX, 8000, nos);
			plasma.OnMsg(orY);
			auto idY = epa.ClOrdId;
			std::cout << "[" << ClientId(orY.clOrdId()) << "-->" << idY << "]" << std::endl;
			// pending rpl ack
			plasma.OnMsg(epa.get_pnd_rpl(idY, idX));
			assert(clt.exe.execType() == ExecType::Pending_Replace && clt.exe.ordStatus() == OrdStatus::Pending_Replace);
			assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == orY.clOrdId() && clt.exe.orderId() == idX);
			assert(clt.exe.leavesQty() == 9000 && clt.exe.cumQty() == 1000 && clt.exe.avgPx() == 99.98);
			assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
			// fill 500
			plasma.OnMsg(epa.get_fill(idX, 6000, 99.98));
			assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == OrdStatus::Partially_Filled);
			assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
			assert(clt.exe.leavesQty() == 8500 && clt.exe.cumQty() == 1500 && clt.exe.avgPx() == 99.98);
			assert(clt.exe.lastQty() == 500 && clt.exe.lastPx() == 99.98);
			// replace reject
			plasma.OnMsg(epa.get_rjt(idY, idX, "replace rejected"));
			assert(clt.rjt.status() == OrdStatus::Partially_Filled);
			assert(clt.rjt.origClOrdId() == nos.clOrdId() && clt.rjt.clOrdId() == orY.clOrdId() && clt.rjt.orderId() == idX);
			// fill 2000
			plasma.OnMsg(epa.get_fill(idX, 6000, 99.98));
			assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == OrdStatus::Partially_Filled);
			assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
			assert(clt.exe.leavesQty() == 6500 && clt.exe.cumQty() == 3500 && clt.exe.avgPx() == 99.98);
			assert(clt.exe.lastQty() == 2000 && clt.exe.lastPx() == 99.98);
		}
		// Replace 2
		{
			// replace request (qty = 6000)
			auto orZ = clt.get_rpl(idX, 6000, nos);
			plasma.OnMsg(orZ);
			auto idZ = epa.ClOrdId;
			std::cout << "[" << ClientId(orZ.clOrdId()) << "-->" << idZ << "]" << std::endl;
			// pending rpl ack
			plasma.OnMsg(epa.get_pnd_rpl(idZ, idX));
			assert(clt.exe.execType() == ExecType::Pending_Replace && clt.exe.ordStatus() == OrdStatus::Pending_Replace);
			assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == orZ.clOrdId() && clt.exe.orderId() == idX);
			assert(clt.exe.leavesQty() == 6500 && clt.exe.cumQty() == 3500 && clt.exe.avgPx() == 99.98);
			assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
			// replaced
			plasma.OnMsg(epa.get_rpld(idZ, idX));
			assert(clt.exe.execType() == ExecType::Replace && clt.exe.ordStatus() == OrdStatus::Partially_Filled);
			assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == orZ.clOrdId() && clt.exe.orderId() == idX);
			assert(clt.exe.leavesQty() == 6000 && clt.exe.cumQty() == 35000 && clt.exe.avgPx() == 99.98);
			assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
			// fill 1500
			plasma.OnMsg(epa.get_fill(idZ, 1500, 99.98));
			assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == OrdStatus::Filled);
			assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == orZ.clOrdId() && clt.exe.orderId() == idZ);
			assert(clt.exe.leavesQty() == 1000 && clt.exe.cumQty() == 5000 && clt.exe.avgPx() == 99.98);
			assert(clt.exe.lastQty() == 1500 && clt.exe.lastPx() == 99.98);
		}
	}
};

TEST_F(TestSuiteD1, nos_rpl_rpld_rpl_rpld) {
	nos_rpl_rpld_rpl_rpld();
}
TEST_F(TestSuiteD1, nos_rpl_rjt_rpl_rpld) {
	nos_rpl_rjt_rpl_rpld();
}
TEST_F(TestSuiteD1, nos_rpl_pnd_rjt_rpl_rpld) {
	nos_rpl_pnd_rjt_rpl_rpld();
}
