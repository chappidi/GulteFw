#include <plasma.h>
#include "GUI.h"
#include "EPA.h"
#include <gtest/gtest.h>

struct TestSuiteD2 : public testing::Test
{
	// declare variables
	plasma::OMS plasma;
	GUI clt;
	EPA epa;
	TestSuiteD2() {
		plasma.OnLogin(clt);
		plasma.OnLogin(epa);
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//  Ack				(X)
	//  Partial Fill	(X)
	//  Replace Request	(Y,X)
	//  Replace Request	(Z,Y)
	//	Pending Replace	(Y,X)
	//	Replaced		(Y,X)
	//	Pending Replace	(Z,Y)
	//	Replaced		(Z,Y)
	//  Partial Fill	(Z)
	// https://www.onixs.biz/fix-dictionary/4.4/app_dD.2.a.html
	void nos_rpl_rpl_pnd_rpld_pnd_rpld_fill() {
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

		// replace request (qty = 7000)
		auto orZ = clt.get_rpl(idY, 7000, orY);
		plasma.OnMsg(orZ);
		auto idZ = epa.ClOrdId;
		std::cout << "[" << ClientId(orZ.clOrdId()) << "-->" << idZ << "]" << std::endl;

		// pending rpl ack
		plasma.OnMsg(epa.get_pnd_rpl(idY, idX));
		assert(clt.exe.execType() == ExecType::Pending_Replace && clt.exe.ordStatus() == OrdStatus::Pending_Replace);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == orY.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 9000 && clt.exe.cumQty() == 1000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// replaced
		plasma.OnMsg(epa.get_rpld(idY, idX));
		assert(clt.exe.execType() == ExecType::Replace && clt.exe.ordStatus() == OrdStatus::Partially_Filled);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == orY.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 7000 && clt.exe.cumQty() == 1000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);


		// pending rpl ack
		plasma.OnMsg(epa.get_pnd_rpl(idZ, idY));
		assert(clt.exe.execType() == ExecType::Pending_Replace && clt.exe.ordStatus() == OrdStatus::Pending_Replace);
		assert(clt.exe.origClOrdId() == orY.clOrdId() && clt.exe.clOrdId() == orZ.clOrdId() && clt.exe.orderId() == idY);
		assert(clt.exe.leavesQty() == 7000 && clt.exe.cumQty() == 1000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// replaced
		plasma.OnMsg(epa.get_rpld(idZ, idY));
		assert(clt.exe.execType() == ExecType::Replace && clt.exe.ordStatus() == OrdStatus::Partially_Filled);
		assert(clt.exe.origClOrdId() == orY.clOrdId() && clt.exe.clOrdId() == orZ.clOrdId() && clt.exe.orderId() == idY);
		assert(clt.exe.leavesQty() == 6000 && clt.exe.cumQty() == 1000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);

		// fill 6000
		plasma.OnMsg(epa.get_fill(idZ, 6000, 99.98));
		assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == OrdStatus::Filled);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == orZ.clOrdId() && clt.exe.orderId() == idZ);
		assert(clt.exe.leavesQty() == 0 && clt.exe.cumQty() == 7000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 6000 && clt.exe.lastPx() == 99.98);
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//  Ack				(X)
	//  Partial Fill	(X)
	//  Replace Request	(Y,X)
	//  Replace Request	(Z,Y)
	//	Pending Replace	(Y,X)
	//	Pending Replace	(Z,Y)
	//	Replaced		(Y,X)
	//	Replaced		(Z,Y)
	//  Partial Fill	(Z)
	// https://www.onixs.biz/fix-dictionary/4.4/app_dD.2.b.html
	void nos_rpl_rpl_pnd_pnd_rpld_rpld_fill() {
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

		// replace request (qty = 7000)
		auto orZ = clt.get_rpl(idY, 7000, orY);
		plasma.OnMsg(orZ);
		auto idZ = epa.ClOrdId;
		std::cout << "[" << ClientId(orZ.clOrdId()) << "-->" << idZ << "]" << std::endl;

		// pending rpl ack
		plasma.OnMsg(epa.get_pnd_rpl(idY, idX));
		assert(clt.exe.execType() == ExecType::Pending_Replace && clt.exe.ordStatus() == OrdStatus::Pending_Replace);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == orY.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 9000 && clt.exe.cumQty() == 1000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// pending rpl ack
		plasma.OnMsg(epa.get_pnd_rpl(idZ, idY));
		assert(clt.exe.execType() == ExecType::Pending_Replace && clt.exe.ordStatus() == OrdStatus::Pending_Replace);
		assert(clt.exe.origClOrdId() == orY.clOrdId() && clt.exe.clOrdId() == orZ.clOrdId() && clt.exe.orderId() == idY);
		assert(clt.exe.leavesQty() == 7000 && clt.exe.cumQty() == 1000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);

		// replaced
		plasma.OnMsg(epa.get_rpld(idY, idX));
		assert(clt.exe.execType() == ExecType::Replace && clt.exe.ordStatus() == OrdStatus::Pending_Replace );
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == orY.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 7000 && clt.exe.cumQty() == 1000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// replaced
		plasma.OnMsg(epa.get_rpld(idZ, idY));
		assert(clt.exe.execType() == ExecType::Replace && clt.exe.ordStatus() == OrdStatus::Partially_Filled);
		assert(clt.exe.origClOrdId() == orY.clOrdId() && clt.exe.clOrdId() == orZ.clOrdId() && clt.exe.orderId() == idY);
		assert(clt.exe.leavesQty() == 6000 && clt.exe.cumQty() == 1000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);

		// fill 6000
		plasma.OnMsg(epa.get_fill(idZ, 6000, 99.98));
		assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == OrdStatus::Filled);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == orZ.clOrdId() && clt.exe.orderId() == idZ);
		assert(clt.exe.leavesQty() == 0 && clt.exe.cumQty() == 7000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 6000 && clt.exe.lastPx() == 99.98);
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//  Ack				(X)
	//  Partial Fill	(X)
	//  Replace Request	(Y,X)
	//  Replace Request	(Z,Y)
	//	Pending Replace	(Y,X)
	//	Replace Reject	(Y,X)
	//	Pending Replace	(Z,X)  Note: Replace (Z,Y) --> Execution(Z,X)
	//	Replace Reject	(Z,X)
	//  Partial Fill	(X)
	// https://www.onixs.biz/fix-dictionary/4.4/app_dD.2.c.html
	void nos_rpl_rpl_pnd_rjt_pnd_rjt_fill() {
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

		// replace request (qty = 7000)
		auto orZ = clt.get_rpl(idY, 7000, orY);
		plasma.OnMsg(orZ);
		auto idZ = epa.ClOrdId;
		std::cout << "[" << ClientId(orZ.clOrdId()) << "-->" << idZ << "]" << std::endl;

		// pending rpl ack
		plasma.OnMsg(epa.get_pnd_rpl(idY, idX));
		assert(clt.exe.execType() == ExecType::Pending_Replace && clt.exe.ordStatus() == OrdStatus::Pending_Replace);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == orY.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 9000 && clt.exe.cumQty() == 1000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// reject
		plasma.OnMsg(epa.get_rjt(idY, idX, ""));
		assert(clt.rjt.status() == OrdStatus::Partially_Filled);
		assert(clt.rjt.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == orY.clOrdId() && clt.exe.orderId() == idX);


		// pending rpl ack
		plasma.OnMsg(epa.get_pnd_rpl(idZ, idX));
		assert(clt.exe.execType() == ExecType::Pending_Replace && clt.exe.ordStatus() == OrdStatus::Pending_Replace);
		assert(clt.exe.origClOrdId() == orY.clOrdId() && clt.exe.clOrdId() == orZ.clOrdId() && clt.exe.orderId() == idY);
		assert(clt.exe.leavesQty() == 9000 && clt.exe.cumQty() == 1000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// reject
		plasma.OnMsg(epa.get_rjt(idZ, idX, ""));
		assert(clt.rjt.status() == OrdStatus::Partially_Filled);
		assert(clt.rjt.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == orZ.clOrdId() && clt.exe.orderId() == idX);

		// fill 6000
		plasma.OnMsg(epa.get_fill(idX, 6000, 99.98));
		assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == OrdStatus::Filled);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 3000 && clt.exe.cumQty() == 7000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 6000 && clt.exe.lastPx() == 99.98);
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//  Ack				(X)
	//  Partial Fill	(X)
	//  Replace Request	(Y,X)
	//  Replace Request	(Z,Y)
	//	Pending Replace	(Y,X)
	//	Replace Reject	(Z,X)
	//	Replaced		(Y,X)
	//  Partial Fill	(Y)
	// https://www.onixs.biz/fix-dictionary/4.4/app_dD.2.d.html
	void nos_rpl_rpl_pnd_rjt_rpl_fill() {
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

		// replace request (qty = 7000)
		auto orZ = clt.get_rpl(idY, 7000, orY);
		plasma.OnMsg(orZ);
		auto idZ = epa.ClOrdId;
		std::cout << "[" << ClientId(orZ.clOrdId()) << "-->" << idZ << "]" << std::endl;

		// pending rpl ack
		plasma.OnMsg(epa.get_pnd_rpl(idY, idX));
		assert(clt.exe.execType() == ExecType::Pending_Replace && clt.exe.ordStatus() == OrdStatus::Pending_Replace);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == orY.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 9000 && clt.exe.cumQty() == 1000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);

		// Rejected because broker does not support processing of order cancel replace request whilst order is pending cancel. 
		// OrigClOrdID set to last accepted ClOrdID i.e. X
		plasma.OnMsg(epa.get_rjt(idZ, idX, "Order already in pending replace status"));
		assert(clt.rjt.status() == OrdStatus::Partially_Filled);
		assert(clt.rjt.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == orY.clOrdId() && clt.exe.orderId() == idX);

		// replaced
		plasma.OnMsg(epa.get_rpld(idY, idX));
		assert(clt.exe.execType() == ExecType::Replace && clt.exe.ordStatus() == OrdStatus::Partially_Filled);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == orY.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 7000 && clt.exe.cumQty() == 1000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);

		// fill 2000
		plasma.OnMsg(epa.get_fill(idY, 6000, 99.98));
		assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == OrdStatus::Filled);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == orY.clOrdId() && clt.exe.orderId() == idY);
		assert(clt.exe.leavesQty() == 5000 && clt.exe.cumQty() == 3000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 2000 && clt.exe.lastPx() == 99.98);
	}
};

TEST_F(TestSuiteD2, nos_rpl_rpl_pnd_rpld_pnd_rpld_fill) {
	nos_rpl_rpl_pnd_rpld_pnd_rpld_fill();
}
TEST_F(TestSuiteD2, nos_rpl_rpl_pnd_pnd_rpld_rpld_fill) {
	nos_rpl_rpl_pnd_pnd_rpld_rpld_fill();
}
TEST_F(TestSuiteD2, nos_rpl_rpl_pnd_rjt_pnd_rjt_fill) {
	nos_rpl_rpl_pnd_rjt_pnd_rjt_fill();
}
TEST_F(TestSuiteD2, nos_rpl_rpl_pnd_rjt_rpl_fill) {
	nos_rpl_rpl_pnd_rjt_rpl_fill();
}
