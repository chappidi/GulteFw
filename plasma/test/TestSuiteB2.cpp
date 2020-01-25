#include <plasma.h>
#include "GUI.h"
#include "EPA.h"
#include <gtest/gtest.h>

struct TestSuiteB2 : public testing::Test
{
	// declare variables
	plasma::OMS plasma;
	GUI clt;
	EPA epa;
	TestSuiteB2() {
		plasma.OnLogin(clt);
		plasma.OnLogin(epa);
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
	void nos_fill_cxl_cxl_fill_rjt_fill_rjt() {
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
		// fill 2000
		plasma.OnMsg(epa.get_fill(idX, 2000, 99.98));
		assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == OrdStatus::Partially_Filled);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 8000 && clt.exe.cumQty() == 2000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 2000 && clt.exe.lastPx() == 99.98);

		//cancel request 1
		auto ocr1 = clt.get_cxl(idX, nos);
		plasma.OnMsg(ocr1);
		auto idY = epa.ClOrdId;
		std::cout << "[" << ClientId(ocr1.clOrdId()) << "-->" << idY << "]" << std::endl;

		//cancel request 2
		auto ocr2 = clt.get_cxl(idX, nos);
		plasma.OnMsg(ocr2);
		auto idZ = epa.ClOrdId;
		std::cout << "[" << ClientId(ocr2.clOrdId()) << "-->" << idZ << "]" << std::endl;

		// parital fill of 3000
		plasma.OnMsg(epa.get_fill(idX, 3000, 99.98));
		assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == OrdStatus::Partially_Filled);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 5000 && clt.exe.cumQty() == 5000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 3000 && clt.exe.lastPx() == 99.98);
		// pending cancel
		plasma.OnMsg(epa.get_pnd_cxl(idY, idX));
		assert(clt.exe.execType() == ExecType::Pending_Cancel && clt.exe.ordStatus() == OrdStatus::Pending_Cancel);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == ocr1.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 5000 && clt.exe.cumQty() == 5000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// pending cancel
		plasma.OnMsg(epa.get_pnd_cxl(idZ, idX));
		assert(clt.exe.execType() == ExecType::Pending_Cancel && clt.exe.ordStatus() == OrdStatus::Pending_Cancel);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == ocr2.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 5000 && clt.exe.cumQty() == 5000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// fill 1000
		plasma.OnMsg(epa.get_fill(idX, 1000, 99.98));
		assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == OrdStatus::Pending_Cancel);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 4000 && clt.exe.cumQty() == 6000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 1000 && clt.exe.lastPx() == 99.98);
		// reject
		plasma.OnMsg(epa.get_rjt(idY, idX, ""));
		assert(clt.rjt.status() == OrdStatus::Pending_Cancel);
		assert(clt.rjt.origClOrdId() == nos.clOrdId() && clt.rjt.clOrdId() == ocr1.clOrdId() && clt.rjt.orderId() == idX);
		// fill 1500
		plasma.OnMsg(epa.get_fill(idX, 1500, 99.98));
		assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == OrdStatus::Pending_Cancel);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 2500 && clt.exe.cumQty() == 7500 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 1500 && clt.exe.lastPx() == 99.98);
		// reject
		plasma.OnMsg(epa.get_rjt(idZ, idX, ""));
		assert(clt.rjt.status() == OrdStatus::Partially_Filled);
		assert(clt.rjt.origClOrdId() == nos.clOrdId() && clt.rjt.clOrdId() == ocr2.clOrdId() && clt.rjt.orderId() == idX);
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
	void nos_fill_cxl_cxl_fill_cxld_rjt() {
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
		// fill 2000
		plasma.OnMsg(epa.get_fill(idX, 2000, 99.98));
		assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == OrdStatus::Partially_Filled);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 8000 && clt.exe.cumQty() == 2000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 2000 && clt.exe.lastPx() == 99.98);

		//cancel request 1
		auto ocr1 = clt.get_cxl(idX, nos);
		plasma.OnMsg(ocr1);
		auto idY = epa.ClOrdId;
		std::cout << "[" << ClientId(ocr1.clOrdId()) << "-->" << idY << "]" << std::endl;

		//cancel request 2
		auto ocr2 = clt.get_cxl(idX, nos);
		plasma.OnMsg(ocr2);
		auto idZ = epa.ClOrdId;
		std::cout << "[" << ClientId(ocr2.clOrdId()) << "-->" << idZ << "]" << std::endl;

		// parital fill of 3000
		plasma.OnMsg(epa.get_fill(idX, 3000, 99.98));
		assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == OrdStatus::Partially_Filled);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 5000 && clt.exe.cumQty() == 5000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 3000 && clt.exe.lastPx() == 99.98);
		// pending cancel
		plasma.OnMsg(epa.get_pnd_cxl(idY, idX));
		assert(clt.exe.execType() == ExecType::Pending_Cancel && clt.exe.ordStatus() == OrdStatus::Pending_Cancel);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == ocr1.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 5000 && clt.exe.cumQty() == 5000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// pending cancel
		plasma.OnMsg(epa.get_pnd_cxl(idZ, idX));
		assert(clt.exe.execType() == ExecType::Pending_Cancel && clt.exe.ordStatus() == OrdStatus::Pending_Cancel);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == ocr2.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 5000 && clt.exe.cumQty() == 5000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// fill 1000
		plasma.OnMsg(epa.get_fill(idX, 1000, 99.98));
		assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == OrdStatus::Pending_Cancel);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 4000 && clt.exe.cumQty() == 6000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 1000 && clt.exe.lastPx() == 99.98);
		// canceled
		plasma.OnMsg(epa.get_cxld(idY, idX));
		assert(clt.exe.execType() == ExecType::Canceled && clt.exe.ordStatus() == OrdStatus::Canceled);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == ocr1.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 0 && clt.exe.cumQty() == 6000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// reject
		plasma.OnMsg(epa.get_rjt(idZ, idX, ""));
		assert(clt.rjt.status() == OrdStatus::Canceled);
		assert(clt.rjt.origClOrdId() == nos.clOrdId() && clt.rjt.clOrdId() == ocr2.clOrdId() && clt.rjt.orderId() == idX);
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
	//  Pending	Cxl		(Z,X)
	//  Partial Fill	(X)
	//	Canceled		(Y,X)
	//	Cancel Reject	(Z,X)
	void nos_cxl_cxl_cxl_fill_rjt_rjt_fill_rjt() {
		// Send Order
		auto nos = clt.get_nos(epa.id(), 10000);
		plasma.OnMsg(nos);
		auto idX = epa.ClOrdId;
		std::cout << "[" << ClientId(nos.clOrdId()) << "-->" << idX << "]" << std::endl;
		// ack order
		plasma.OnMsg(epa.get_new(idX));
		// fill 2000
		plasma.OnMsg(epa.get_fill(idX, 2000, 99.98));

		//cancel request 1
		auto ocr1 = clt.get_cxl(idX, nos);
		plasma.OnMsg(ocr1);
		auto idY = epa.ClOrdId;
		std::cout << "[" << ClientId(ocr1.clOrdId()) << "-->" << idY << "]" << std::endl;
		//cancel request 2
		auto ocr2 = clt.get_cxl(idX, nos);
		plasma.OnMsg(ocr2);
		auto idZ = epa.ClOrdId;
		std::cout << "[" << ClientId(ocr2.clOrdId()) << "-->" << idZ << "]" << std::endl;
		//cancel request 3
		auto ocr3 = clt.get_cxl(idX, nos);
		plasma.OnMsg(ocr3);
		auto idQ = epa.ClOrdId;
		std::cout << "[" << ClientId(ocr3.clOrdId()) << "-->" << idQ << "]" << std::endl;

		// parital fill of 2000
		plasma.OnMsg(epa.get_fill(idX, 2000, 99.98));
		assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == OrdStatus::Partially_Filled);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 6000 && clt.exe.cumQty() == 4000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 2000 && clt.exe.lastPx() == 99.98);
		//  Pending	Cxl		(Y,X)
		plasma.OnMsg(epa.get_pnd_cxl(idY, idX));
		assert(clt.exe.execType() == ExecType::Pending_Cancel && clt.exe.ordStatus() == OrdStatus::Pending_Cancel);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == ocr1.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 6000 && clt.exe.cumQty() == 4000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// parital fill of 1000
		plasma.OnMsg(epa.get_fill(idX, 1000, 99.98));
		assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == OrdStatus::Pending_Cancel);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 5000 && clt.exe.cumQty() == 5000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 1000 && clt.exe.lastPx() == 99.98);
		//  Pending	Cxl		(Z,X)
		plasma.OnMsg(epa.get_pnd_cxl(idZ, idX));
		assert(clt.exe.execType() == ExecType::Pending_Cancel && clt.exe.ordStatus() == OrdStatus::Pending_Cancel);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == ocr2.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 5000 && clt.exe.cumQty() == 5000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		//  Pending	Cxl		(Q,X)
		plasma.OnMsg(epa.get_pnd_cxl(idQ, idX));
		assert(clt.exe.execType() == ExecType::Pending_Cancel && clt.exe.ordStatus() == OrdStatus::Pending_Cancel);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == ocr3.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 5000 && clt.exe.cumQty() == 5000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// fill 1000
		plasma.OnMsg(epa.get_fill(idX, 1000, 99.98));
		assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == OrdStatus::Pending_Cancel);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 4000 && clt.exe.cumQty() == 6000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 1000 && clt.exe.lastPx() == 99.98);
		// reject
		plasma.OnMsg(epa.get_rjt(idZ, idX, ""));
		assert(clt.rjt.status() == OrdStatus::Pending_Cancel);
		assert(clt.rjt.origClOrdId() == nos.clOrdId() && clt.rjt.clOrdId() == ocr2.clOrdId() && clt.rjt.orderId() == idX);
		// reject
		plasma.OnMsg(epa.get_rjt(idQ, idX, ""));
		assert(clt.rjt.status() == OrdStatus::Pending_Cancel);
		assert(clt.rjt.origClOrdId() == nos.clOrdId() && clt.rjt.clOrdId() == ocr3.clOrdId() && clt.rjt.orderId() == idX);
		// fill 1500
		plasma.OnMsg(epa.get_fill(idX, 1500, 99.98));
		assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == OrdStatus::Pending_Cancel);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 2500 && clt.exe.cumQty() == 7500 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 1500 && clt.exe.lastPx() == 99.98);
		// reject
		plasma.OnMsg(epa.get_rjt(idY, idX, ""));
		assert(clt.rjt.status() == OrdStatus::Partially_Filled);
		assert(clt.rjt.origClOrdId() == nos.clOrdId() && clt.rjt.clOrdId() == ocr1.clOrdId() && clt.rjt.orderId() == idX);
	}
};


TEST_F(TestSuiteB2, nos_fill_cxl_cxl_fill_rjt_fill_rjt) {
	nos_fill_cxl_cxl_fill_rjt_fill_rjt();
}
TEST_F(TestSuiteB2, nos_fill_cxl_cxl_fill_cxld_rjt) {
	nos_fill_cxl_cxl_fill_cxld_rjt();
}
TEST_F(TestSuiteB2, nos_cxl_cxl_cxl_fill_rjt_rjt_fill_rjt) {
	nos_cxl_cxl_cxl_fill_rjt_rjt_fill_rjt();
}