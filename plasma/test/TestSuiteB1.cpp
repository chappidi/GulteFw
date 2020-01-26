#include <plasma.h>
#include "GUI.h"
#include "EPA.h"
#include <gtest/gtest.h>

struct TestSuiteB1 : public testing::Test
{
	// declare variables
	plasma::OMS plasma;
	GUI clt;
	EPA epa;
	TestSuiteB1() {
		plasma.OnLogin(clt);
		plasma.OnLogin(epa);
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//  Ack				(X)
	//  Cancel Request	(Y,X)
	//	Pending Cxl		(Y,X)
	//	Cancel Reject	(Y,X)
	//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.a.html
	void nos_new_cxl_pnd_rjt() {
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
		//cancel request
		auto ocr = clt.get_cxl(idX, nos);
		plasma.OnMsg(ocr);
		auto idY = epa.ClOrdId;
		std::cout << "[" << ClientId(ocr.clOrdId()) << "-->" << idY << "]" << std::endl;
		// pending cxl ack
		plasma.OnMsg(epa.get_pnd_cxl(idY, idX));
		assert(clt.exe.execType() == ExecType::Pending_Cancel && clt.exe.ordStatus() == OrdStatus::Pending_Cancel);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == ocr.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == nos.qty() && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// reject
		plasma.OnMsg(epa.get_rjt(idY, idX, ""));
		assert(clt.rjt.status() == OrdStatus::New);
		assert(clt.rjt.origClOrdId() == nos.clOrdId() && clt.rjt.clOrdId() == ocr.clOrdId() && clt.rjt.orderId() == idX);
	} 
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//  Ack				(X)
	//  Cancel Request	(Y,X)
	//	Pending Cxl		(Y,X)
	//	Canceled		(Y,X)
	//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.a.html
	void nos_new_cxl_pnd_cxld() {
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
		//cancel request
		auto ocr = clt.get_cxl(idX, nos);
		plasma.OnMsg(ocr);
		auto idY = epa.ClOrdId;
		std::cout << "[" << ClientId(ocr.clOrdId()) << "-->" << idY << "]" << std::endl;
		// pending cancel
		plasma.OnMsg(epa.get_pnd_cxl(idY, idX));
		assert(clt.exe.execType() == ExecType::Pending_Cancel && clt.exe.ordStatus() == OrdStatus::Pending_Cancel);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == ocr.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == nos.qty() && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// accept cancel
		plasma.OnMsg(epa.get_cxld(idY, idX));
		assert(clt.exe.execType() == ExecType::Canceled && clt.exe.ordStatus() == OrdStatus::Canceled);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == ocr.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 0 && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//  Ack				(X)
	//  Partial Fill	(X)
	//  Cancel Request	(Y,X)
	//  Partial Fill	(X)
	//	Pending Cxl		(Y,X)
	//  Partial Fill	(X)
	//	Canceled		(Y,X)
	//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.b.html
	void nos_cxl_partfill_cxld() {
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

		// cancel request
		auto ocr = clt.get_cxl(idX, nos);
		plasma.OnMsg(ocr);
		auto idY = epa.ClOrdId;
		std::cout << "[" << ClientId(ocr.clOrdId()) << "-->" << idY << "]" << std::endl;
		// fill 3000
		plasma.OnMsg(epa.get_fill(idX, 3000, 99.98));
		assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == OrdStatus::Partially_Filled);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 5000 && clt.exe.cumQty() == 5000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 3000 && clt.exe.lastPx() == 99.98);
		// pending cancel
		plasma.OnMsg(epa.get_pnd_cxl(idY, idX));
		assert(clt.exe.execType() == ExecType::Pending_Cancel && clt.exe.ordStatus() == OrdStatus::Pending_Cancel);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == ocr.clOrdId() && clt.exe.orderId() == idX);
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
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == ocr.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 0 && clt.exe.cumQty() == 6000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//  Ack				(X)
	//  Partial Fill	(X)
	//  Cancel Request	(Y,X)
	//  Partial Fill	(X)
	//	Pending Cxl		(Y,X)
	//  Filled			(X)
	//	Cancel Reject	(Y,X)
	//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.c.html
	void nos_cxl_fill_rjt() {
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

		// cancel request
		auto ocr = clt.get_cxl(idX, nos);
		plasma.OnMsg(ocr);
		auto idY = epa.ClOrdId;
		std::cout << "[" << ClientId(ocr.clOrdId()) << "-->" << idY << "]" << std::endl;
		// fill 3000
		plasma.OnMsg(epa.get_fill(idX, 3000, 99.98));
		assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == OrdStatus::Partially_Filled);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 5000 && clt.exe.cumQty() == 5000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 3000 && clt.exe.lastPx() == 99.98);
		// pending cancel
		plasma.OnMsg(epa.get_pnd_cxl(idY, idX));
		assert(clt.exe.execType() == ExecType::Pending_Cancel && clt.exe.ordStatus() == OrdStatus::Pending_Cancel);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == ocr.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 5000 && clt.exe.cumQty() == 5000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// fill 5000
		plasma.OnMsg(epa.get_fill(idX, 5000, 99.98));
		assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == OrdStatus::Pending_Cancel);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 0 && clt.exe.cumQty() == 10000 && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 5000 && clt.exe.lastPx() == 99.98);
		// reject
		plasma.OnMsg(epa.get_rjt(idY, idX, ""));
		assert(clt.rjt.status() == OrdStatus::Filled);
		assert(clt.rjt.origClOrdId() == nos.clOrdId() && clt.rjt.clOrdId() == ocr.clOrdId() && clt.rjt.orderId() == idX);
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//  Cancel Request	(Y,X)
	//  Ack				(X)
	//	Cancel Reject	(Y,X)
	//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.d.html
	void nos_cxl_new_rjt() {
		// Send Order
		auto nos = clt.get_nos(epa.id(), 10000);
		plasma.OnMsg(nos);
		auto idX = epa.ClOrdId;
		std::cout << "[" << ClientId(nos.clOrdId()) << "-->" << idX << "]" << std::endl;
		//cancel request
		auto ocr = clt.get_cxl(idX, nos);
		plasma.OnMsg(ocr);
		auto idY = epa.ClOrdId;
		std::cout << "[" << ClientId(ocr.clOrdId()) << "-->" << idY << "]" << std::endl;
		// ack order
		plasma.OnMsg(epa.get_new(idX));
		assert(clt.exe.execType() == ExecType::New && clt.exe.ordStatus() == OrdStatus::New);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == nos.qty() && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// reject
		plasma.OnMsg(epa.get_rjt(idY, idX, ""));
		assert(clt.rjt.status() == OrdStatus::New);
		assert(clt.rjt.origClOrdId() == nos.clOrdId() && clt.rjt.clOrdId() == ocr.clOrdId() && clt.rjt.orderId() == idX);
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//  Cancel Request	(Y,X)
	//  Pending	Cxl		(Y,X)
	//  Ack				(X)
	//	Canceled		(Y,X)
	//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.d.html
	void nos_cxl_pnd_new_cxld() {
		// Send Order
		auto nos = clt.get_nos(epa.id(), 10000);
		plasma.OnMsg(nos);
		auto idX = epa.ClOrdId;
		std::cout << "[" << ClientId(nos.clOrdId()) << "-->" << idX << "]" << std::endl;
		//cancel request
		auto ocr = clt.get_cxl(idX, nos);
		plasma.OnMsg(ocr);
		auto idY = epa.ClOrdId;
		std::cout << "[" << ClientId(ocr.clOrdId()) << "-->" << idY << "]" << std::endl;
		// pending cancel
		plasma.OnMsg(epa.get_pnd_cxl(idY, idX));
		assert(clt.exe.execType() == ExecType::Pending_Cancel && clt.exe.ordStatus() == OrdStatus::Pending_Cancel);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == ocr.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == nos.qty() && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// ack order
		plasma.OnMsg(epa.get_new(idX));
		assert(clt.exe.execType() == ExecType::New && clt.exe.ordStatus() == OrdStatus::New);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == nos.qty() && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// accept cancel
		plasma.OnMsg(epa.get_cxld(idY, idX));
		assert(clt.exe.execType() == ExecType::Canceled && clt.exe.ordStatus() == OrdStatus::Canceled);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == ocr.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 0 && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//  Cancel Request	(Y,X)
	//  Pending	Cxl		(Y,X)
	//	Canceled		(Y,X)
	//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.d.html
	void nos_cxl_pnd_cxld() {
		// Send Order
		auto nos = clt.get_nos(epa.id(), 10000);
		plasma.OnMsg(nos);
		auto idX = epa.ClOrdId;
		std::cout << "[" << ClientId(nos.clOrdId()) << "-->" << idX << "]" << std::endl;
		//cancel request
		auto ocr = clt.get_cxl(idX, nos);
		plasma.OnMsg(ocr);
		auto idY = epa.ClOrdId;
		std::cout << "[" << ClientId(ocr.clOrdId()) << "-->" << idY << "]" << std::endl;
		// pending cancel
		plasma.OnMsg(epa.get_pnd_cxl(idY, idX));
		assert(clt.exe.execType() == ExecType::Pending_Cancel && clt.exe.ordStatus() == OrdStatus::Pending_Cancel);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == ocr.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == nos.qty() && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// accept cancel
		plasma.OnMsg(epa.get_cxld(idY, idX));
		assert(clt.exe.execType() == ExecType::Canceled && clt.exe.ordStatus() == OrdStatus::Canceled);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == ocr.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 0 && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//  Cancel Request	(Y,X)
	//  Ack				(X)
	//  Pending	Cxl		(Y,X)
	//	Canceled		(Y,X)
	//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.e.html
	void nos_cxl_new_pnd_cxld() {
		// Send Order
		auto nos = clt.get_nos(epa.id(), 10000);
		plasma.OnMsg(nos);
		auto idX = epa.ClOrdId;
		std::cout << "[" << ClientId(nos.clOrdId()) << "-->" << idX << "]" << std::endl;
		//cancel request
		auto ocr = clt.get_cxl(idX, nos);
		plasma.OnMsg(ocr);
		auto idY = epa.ClOrdId;
		std::cout << "[" << ClientId(ocr.clOrdId()) << "-->" << idY << "]" << std::endl;
		// ack order
		plasma.OnMsg(epa.get_new(idX));
		assert(clt.exe.execType() == ExecType::New && clt.exe.ordStatus() == OrdStatus::New);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == nos.qty() && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// pending cancel
		plasma.OnMsg(epa.get_pnd_cxl(idY, idX));
		assert(clt.exe.execType() == ExecType::Pending_Cancel && clt.exe.ordStatus() == OrdStatus::Pending_Cancel);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == ocr.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == nos.qty() && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// accept cancel
		plasma.OnMsg(epa.get_cxld(idY, idX));
		assert(clt.exe.execType() == ExecType::Canceled && clt.exe.ordStatus() == OrdStatus::Canceled);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == ocr.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 0 && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
	}
	///////////////////////////////////////////////////////////////
	//  Cancel Request	(Y,X)
	//	Cancel Reject	(Y,X)
	//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.f.html
	void cxl_rjt_unknwn() {
		// Send Order
		auto nos = clt.get_nos(epa.id(), 10000);
		auto idX = 929;
		//cancel request
		auto ocr = clt.get_cxl(idX, nos);
		plasma.OnMsg(ocr);
		auto idY = epa.ClOrdId;
		std::cout << "[" << ClientId(ocr.clOrdId()) << "-->" << idY << "]" << std::endl;
		// reject
		plasma.OnMsg(epa.get_rjt(idY, idX, "Unknown Order"));
		assert(clt.rjt.status() == OrdStatus::Rejected);
		assert(clt.rjt.origClOrdId() == nos.clOrdId() && clt.rjt.clOrdId() == ocr.clOrdId() && clt.rjt.orderId() == idX);
	}
};
TEST_F(TestSuiteB1, nos_new_cxl_pnd_rjt) {
	nos_new_cxl_pnd_rjt();
}
TEST_F(TestSuiteB1, nos_new_cxl_pnd_cxld) {
	nos_new_cxl_pnd_cxld();
}
TEST_F(TestSuiteB1, nos_cxl_partfill_cxld) {
	nos_cxl_partfill_cxld();
}
TEST_F(TestSuiteB1, nos_cxl_fill_rjt) {
	nos_cxl_fill_rjt();
}
TEST_F(TestSuiteB1, nos_cxl_new_rjt) {
	nos_cxl_new_rjt();
}
TEST_F(TestSuiteB1, nos_cxl_pnd_new_cxld) {
	nos_cxl_pnd_new_cxld();
}
TEST_F(TestSuiteB1, nos_cxl_pnd_cxld) {
	nos_cxl_pnd_cxld();
}
TEST_F(TestSuiteB1, nos_cxl_new_pnd_cxld) {
	nos_cxl_new_pnd_cxld();
}
TEST_F(TestSuiteB1, cxl_rjt_unknwn) {
	cxl_rjt_unknwn();
}

TEST_F(TestSuiteB1, all) {
	nos_new_cxl_pnd_rjt();
	nos_new_cxl_pnd_cxld();
	nos_cxl_partfill_cxld();
	nos_cxl_fill_rjt();
	nos_cxl_new_rjt();
	nos_cxl_pnd_new_cxld();
	nos_cxl_pnd_cxld();
	nos_cxl_new_pnd_cxld();
	cxl_rjt_unknwn();
}