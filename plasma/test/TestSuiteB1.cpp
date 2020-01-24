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
		plasma.OnMsg(epa.get_rjt(idY, idX));
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
		plasma.OnMsg(epa.get_rjt(idY, idX));
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
		plasma.OnMsg(epa.get_rjt(idY, idX));
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
		EXPECT_TRUE(clt.nfr.status() == OrdStatus::Pending_Cancel);
		EXPECT_TRUE(clt.nfr.origClOrdId() == nos.clOrdId() && clt.nfr.clOrdId() == ocr.clOrdId() && clt.nfr.orderId() == idX);
		EXPECT_TRUE(clt.nfr.leavesQty() == 10000 && clt.nfr.cumQty() == 0 && clt.nfr.avgPx() == 0);
		// ack order
		plasma.OnMsg(epa.get_new(idX));
		assert(clt.exe.execType() == ExecType::New && clt.exe.ordStatus() == OrdStatus::New);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == nos.qty() && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// accept cancel
		plasma.OnMsg(epa.get_cxld(idY, idX));
		EXPECT_TRUE(clt.nfr.status() == OrdStatus::Canceled);
		EXPECT_TRUE(clt.nfr.origClOrdId() == nos.clOrdId() && clt.nfr.clOrdId() == ocr.clOrdId() && clt.nfr.orderId() == idX);
		EXPECT_TRUE(clt.nfr.leavesQty() == 0 && clt.nfr.cumQty() == 0 && clt.nfr.avgPx() == 0);
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
		EXPECT_TRUE(clt.nfr.status() == OrdStatus::Pending_Cancel);
		EXPECT_TRUE(clt.nfr.origClOrdId() == nos.clOrdId() && clt.nfr.clOrdId() == ocr.clOrdId() && clt.nfr.orderId() == idX);
		EXPECT_TRUE(clt.nfr.leavesQty() == 10000 && clt.nfr.cumQty() == 0 && clt.nfr.avgPx() == 0);
		// accept cancel
		plasma.OnMsg(epa.get_cxld(idY, idX));
		EXPECT_TRUE(clt.nfr.status() == OrdStatus::Canceled);
		EXPECT_TRUE(clt.nfr.origClOrdId() == nos.clOrdId() && clt.nfr.clOrdId() == ocr.clOrdId() && clt.nfr.orderId() == idX);
		EXPECT_TRUE(clt.nfr.leavesQty() == 0 && clt.nfr.cumQty() == 0 && clt.nfr.avgPx() == 0);
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
		EXPECT_TRUE(clt.nfr.status() == OrdStatus::Pending_Cancel);
		EXPECT_TRUE(clt.nfr.origClOrdId() == nos.clOrdId() && clt.nfr.clOrdId() == ocr.clOrdId() && clt.nfr.orderId() == idX);
		EXPECT_TRUE(clt.nfr.leavesQty() == 10000 && clt.nfr.cumQty() == 0 && clt.nfr.avgPx() == 0);
		// accept cancel
		plasma.OnMsg(epa.get_cxld(idY, idX));
		EXPECT_TRUE(clt.nfr.status() == OrdStatus::Canceled);
		EXPECT_TRUE(clt.nfr.origClOrdId() == nos.clOrdId() && clt.nfr.clOrdId() == ocr.clOrdId() && clt.nfr.orderId() == idX);
		EXPECT_TRUE(clt.nfr.leavesQty() == 0 && clt.nfr.cumQty() == 0 && clt.nfr.avgPx() == 0);
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
		plasma.OnMsg(epa.get_rjt(idY, idX));
		EXPECT_TRUE(clt.rjt.status() == OrdStatus::Rejected);
		EXPECT_TRUE(clt.rjt.origClOrdId() == nos.clOrdId() && clt.rjt.clOrdId() == ocr.clOrdId() && clt.rjt.orderId() == idX);
	}

	void test_cxl_multi() {
		// Send Order
		auto nos = clt.get_nos(epa.id(), 10000);
		plasma.OnMsg(nos);
		auto idX = epa.ClOrdId;
		std::cout << "[" << ClientId(nos.clOrdId()) << "-->" << idX << "]" << std::endl;
		plasma.OnMsg(epa.get_new(idX));
		plasma.OnMsg(epa.get_fill(idX, 2000, 99.98));
		EXPECT_TRUE(clt.fill.status() == OrdStatus::Partially_Filled);
		EXPECT_TRUE(clt.fill.clOrdId() == nos.clOrdId() && clt.fill.orderId() == idX);
		EXPECT_TRUE(clt.fill.leavesQty() == 8000 && clt.fill.cumQty() == 2000 && clt.fill.avgPx() == 99.98);
		EXPECT_TRUE(clt.fill.lastPx() == 99.98 && clt.fill.lastQty() == 2000);

		//cancel request 1
		auto ocr1 = clt.get_cxl(idX, nos);
		plasma.OnMsg(ocr1);
		auto x1Id = epa.ClOrdId;
		std::cout << "[" << ClientId(ocr1.clOrdId()) << "-->" << epa.ClOrdId << "]" << std::endl;

		//cancel request 2
		auto ocr2 = clt.get_cxl(idX, nos);
		plasma.OnMsg(ocr2);
		auto x2Id = epa.ClOrdId;
		std::cout << "[" << ClientId(ocr2.clOrdId()) << "-->" << epa.ClOrdId << "]" << std::endl;
		// parital fill of 3000
		plasma.OnMsg(epa.get_fill(idX, 3000, 99.98));
		EXPECT_TRUE(clt.fill.status() == OrdStatus::Partially_Filled);
		EXPECT_TRUE(clt.fill.clOrdId() == nos.clOrdId() && clt.fill.orderId() == idX);
		EXPECT_TRUE(clt.fill.leavesQty() == 5000 && clt.fill.cumQty() == 5000 && clt.fill.avgPx() == 99.98);
		EXPECT_TRUE(clt.fill.lastPx() == 99.98 && clt.fill.lastQty() == 3000);
		// pending cancel
		plasma.OnMsg(epa.get_pnd_cxl(x1Id, idX));
		EXPECT_TRUE(clt.nfr.status() == OrdStatus::Pending_Cancel);
		EXPECT_TRUE(clt.nfr.origClOrdId() == nos.clOrdId() && clt.nfr.clOrdId() == ocr1.clOrdId() && clt.nfr.orderId() == idX);
		EXPECT_TRUE(clt.nfr.leavesQty() == 5000 && clt.nfr.cumQty() == 5000 && clt.nfr.avgPx() == 99.98);
		// pending cancel
		plasma.OnMsg(epa.get_pnd_cxl(x2Id, idX));
		EXPECT_TRUE(clt.nfr.status() == OrdStatus::Pending_Cancel);
		EXPECT_TRUE(clt.nfr.origClOrdId() == nos.clOrdId() && clt.nfr.clOrdId() == ocr2.clOrdId() && clt.nfr.orderId() == idX);
		EXPECT_TRUE(clt.nfr.leavesQty() == 5000 && clt.nfr.cumQty() == 5000 && clt.nfr.avgPx() == 99.98);
		// fill 1000
		plasma.OnMsg(epa.get_fill(idX, 1000, 99.98));
		EXPECT_TRUE(clt.fill.status() == OrdStatus::Pending_Cancel);
		EXPECT_TRUE(clt.fill.clOrdId() == nos.clOrdId() && clt.fill.orderId() == idX);
		EXPECT_TRUE(clt.fill.leavesQty() == 4000 && clt.fill.cumQty() == 6000 && clt.fill.avgPx() == 99.98);
		EXPECT_TRUE(clt.fill.lastPx() == 99.98 && clt.fill.lastQty() == 1000);
		// reject
		plasma.OnMsg(epa.get_rjt(x1Id, idX));
		EXPECT_TRUE(clt.rjt.status() == OrdStatus::Pending_Cancel);
		EXPECT_TRUE(clt.rjt.origClOrdId() == nos.clOrdId() && clt.rjt.clOrdId() == ocr1.clOrdId() && clt.rjt.orderId() == idX);
		// fill 1500
		plasma.OnMsg(epa.get_fill(idX, 1500, 99.98));
		EXPECT_TRUE(clt.fill.status() == OrdStatus::Pending_Cancel);
		EXPECT_TRUE(clt.fill.clOrdId() == nos.clOrdId() && clt.fill.orderId() == idX);
		EXPECT_TRUE(clt.fill.leavesQty() == 2500 && clt.fill.cumQty() == 7500 && clt.fill.avgPx() == 99.98);
		EXPECT_TRUE(clt.fill.lastPx() == 99.98 && clt.fill.lastQty() == 1500);
		// reject
		plasma.OnMsg(epa.get_rjt(x2Id, idX));
		EXPECT_TRUE(clt.rjt.status() == OrdStatus::Partially_Filled);
		EXPECT_TRUE(clt.rjt.origClOrdId() == nos.clOrdId() && clt.rjt.clOrdId() == ocr2.clOrdId() && clt.rjt.orderId() == idX);
	}

	void test_cxl_three() {
		// Send Order
		auto nos = clt.get_nos(epa.id(), 10000);
		plasma.OnMsg(nos);
		auto idX = epa.ClOrdId;
		std::cout << "[" << ClientId(nos.clOrdId()) << "-->" << idX << "]" << std::endl;
		plasma.OnMsg(epa.get_new(idX));
		plasma.OnMsg(epa.get_fill(idX, 2000, 99.98));
		EXPECT_TRUE(clt.fill.status() == OrdStatus::Partially_Filled);
		EXPECT_TRUE(clt.fill.clOrdId() == nos.clOrdId() && clt.fill.orderId() == idX);
		EXPECT_TRUE(clt.fill.leavesQty() == 8000 && clt.fill.cumQty() == 2000 && clt.fill.avgPx() == 99.98);
		EXPECT_TRUE(clt.fill.lastPx() == 99.98 && clt.fill.lastQty() == 2000);

		//cancel request 1
		auto ocr1 = clt.get_cxl(idX, nos);
		plasma.OnMsg(ocr1);
		auto x1Id = epa.ClOrdId;
		std::cout << "[" << ClientId(ocr1.clOrdId()) << "-->" << epa.ClOrdId << "]" << std::endl;
		//cancel request 2
		auto ocr2 = clt.get_cxl(idX, nos);
		plasma.OnMsg(ocr2);
		auto x2Id = epa.ClOrdId;
		std::cout << "[" << ClientId(ocr2.clOrdId()) << "-->" << epa.ClOrdId << "]" << std::endl;
		//cancel request 3
		auto ocr3 = clt.get_cxl(idX, nos);
		plasma.OnMsg(ocr3);
		auto x3Id = epa.ClOrdId;
		std::cout << "[" << ClientId(ocr3.clOrdId()) << "-->" << epa.ClOrdId << "]" << std::endl;

		// parital fill of 2000
		plasma.OnMsg(epa.get_fill(idX, 2000, 99.98));
		EXPECT_TRUE(clt.fill.status() == OrdStatus::Partially_Filled);
		EXPECT_TRUE(clt.fill.clOrdId() == nos.clOrdId() && clt.fill.orderId() == idX);
		EXPECT_TRUE(clt.fill.leavesQty() == 6000 && clt.fill.cumQty() == 4000 && clt.fill.avgPx() == 99.98);
		EXPECT_TRUE(clt.fill.lastPx() == 99.98 && clt.fill.lastQty() == 2000);
		// pending cancel
		plasma.OnMsg(epa.get_pnd_cxl(x1Id, idX));
		EXPECT_TRUE(clt.nfr.status() == OrdStatus::Pending_Cancel);
		EXPECT_TRUE(clt.nfr.origClOrdId() == nos.clOrdId() && clt.nfr.clOrdId() == ocr1.clOrdId() && clt.nfr.orderId() == idX);
		EXPECT_TRUE(clt.nfr.leavesQty() == 6000 && clt.nfr.cumQty() == 4000 && clt.nfr.avgPx() == 99.98);
		// parital fill of 1000
		plasma.OnMsg(epa.get_fill(idX, 1000, 99.98));
		EXPECT_TRUE(clt.fill.status() == OrdStatus::Pending_Cancel);
		EXPECT_TRUE(clt.fill.clOrdId() == nos.clOrdId() && clt.fill.orderId() == idX);
		EXPECT_TRUE(clt.fill.leavesQty() == 5000 && clt.fill.cumQty() == 5000 && clt.fill.avgPx() == 99.98);
		EXPECT_TRUE(clt.fill.lastPx() == 99.98 && clt.fill.lastQty() == 1000);
		// pending cancel
		plasma.OnMsg(epa.get_pnd_cxl(x2Id, idX));
		EXPECT_TRUE(clt.nfr.status() == OrdStatus::Pending_Cancel);
		EXPECT_TRUE(clt.nfr.origClOrdId() == nos.clOrdId() && clt.nfr.clOrdId() == ocr2.clOrdId() && clt.nfr.orderId() == idX);
		EXPECT_TRUE(clt.nfr.leavesQty() == 5000 && clt.nfr.cumQty() == 5000 && clt.nfr.avgPx() == 99.98);
		// pending cancel
		plasma.OnMsg(epa.get_pnd_cxl(x3Id, idX));
		EXPECT_TRUE(clt.nfr.status() == OrdStatus::Pending_Cancel);
		EXPECT_TRUE(clt.nfr.origClOrdId() == nos.clOrdId() && clt.nfr.clOrdId() == ocr3.clOrdId() && clt.nfr.orderId() == idX);
		EXPECT_TRUE(clt.nfr.leavesQty() == 5000 && clt.nfr.cumQty() == 5000 && clt.nfr.avgPx() == 99.98);
		// fill 1000
		plasma.OnMsg(epa.get_fill(idX, 1000, 99.98));
		EXPECT_TRUE(clt.fill.status() == OrdStatus::Pending_Cancel);
		EXPECT_TRUE(clt.fill.clOrdId() == nos.clOrdId() && clt.fill.orderId() == idX);
		EXPECT_TRUE(clt.fill.leavesQty() == 4000 && clt.fill.cumQty() == 6000 && clt.fill.avgPx() == 99.98);
		EXPECT_TRUE(clt.fill.lastPx() == 99.98 && clt.fill.lastQty() == 1000);
		// reject
		plasma.OnMsg(epa.get_rjt(x2Id, idX));
		EXPECT_TRUE(clt.rjt.status() == OrdStatus::Pending_Cancel);
		EXPECT_TRUE(clt.rjt.origClOrdId() == nos.clOrdId() && clt.rjt.clOrdId() == ocr2.clOrdId() && clt.rjt.orderId() == idX);
		// reject
		plasma.OnMsg(epa.get_rjt(x3Id, idX));
		EXPECT_TRUE(clt.rjt.status() == OrdStatus::Pending_Cancel);
		EXPECT_TRUE(clt.rjt.origClOrdId() == nos.clOrdId() && clt.rjt.clOrdId() == ocr3.clOrdId() && clt.rjt.orderId() == idX);
		// fill 1500
		plasma.OnMsg(epa.get_fill(idX, 1500, 99.98));
		EXPECT_TRUE(clt.fill.status() == OrdStatus::Pending_Cancel);
		EXPECT_TRUE(clt.fill.clOrdId() == nos.clOrdId() && clt.fill.orderId() == idX);
		EXPECT_TRUE(clt.fill.leavesQty() == 2500 && clt.fill.cumQty() == 7500 && clt.fill.avgPx() == 99.98);
		EXPECT_TRUE(clt.fill.lastPx() == 99.98 && clt.fill.lastQty() == 1500);
		// reject
		plasma.OnMsg(epa.get_rjt(x1Id, idX));
		EXPECT_TRUE(clt.rjt.status() == OrdStatus::Partially_Filled);
		EXPECT_TRUE(clt.rjt.origClOrdId() == nos.clOrdId() && clt.rjt.clOrdId() == ocr1.clOrdId() && clt.rjt.orderId() == idX);
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

TEST_F(TestSuiteB1, test_cxl_multi) {
	test_cxl_multi();
}

TEST_F(TestSuiteB1, test_cxl_three) {
	test_cxl_three();
}
