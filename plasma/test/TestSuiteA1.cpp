#include <plasma.h>
#include "GUI.h"
#include "EPA.h"
#include <gtest/gtest.h>

struct TestSuiteA1 : public testing::Test
{
	// declare variables
	plasma::OMS plasma;
	GUI clt;
	EPA epa;
	PROXY<NewOrderSingle>		nos;
	PROXY<OrderStatusRequest>	osr;
	int32_t						idX{ 0 };
	double						_execQty{ 0 };
	OrdStatus::Value			_ordStatus{ OrdStatus::NA };
	TestSuiteA1() {
		plasma.OnLogin(clt);
		plasma.OnLogin(epa);

		nos = clt.get_nos(epa.id(), 10000);
		osr = clt.get_sts(nos);

	}
	auto new_order() {
		// New Order(X)	
		plasma.OnMsg(nos);
		idX = epa.ClOrdId;
		std::cout << "[" << ClientId(nos.clOrdId()) << "-->" << idX << "]" << std::endl;
		// validate status
		plasma.OnMsg(osr);
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == OrdStatus::NA);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == nos.qty() && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		return idX;
	}
	void pending_ack() {
		_ordStatus = OrdStatus::Pending_New;
		plasma.OnMsg(epa.get_pnd_new(idX));
		assert(clt.exe.execType() == ExecType::Pending_New && clt.exe.ordStatus() == OrdStatus::Pending_New);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == nos.qty() && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// validate status
		plasma.OnMsg(osr);
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == OrdStatus::Pending_New);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == nos.qty() && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
	}
	void ack() {
		_ordStatus = OrdStatus::New;
		plasma.OnMsg(epa.get_new(idX));
		assert(clt.exe.execType() == ExecType::New && clt.exe.ordStatus() == OrdStatus::New);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == nos.qty() && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// validate status
		plasma.OnMsg(osr);
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == OrdStatus::New);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == nos.qty() && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
	}
	void reject() {
		_ordStatus = OrdStatus::Rejected;
		plasma.OnMsg(epa.get_rjt(idX));
		assert(clt.exe.execType() == ExecType::Rejected && clt.exe.ordStatus() == OrdStatus::Rejected);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 0 && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// validate status
		plasma.OnMsg(osr);
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == OrdStatus::Rejected);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 0 && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
	}
	void fill(double qty) {
		_execQty += qty;
		double expected_leaves = nos.qty() - _execQty;
		_ordStatus = (_execQty == nos.qty()) ? OrdStatus::Filled : OrdStatus::Partially_Filled;
		// fill
		plasma.OnMsg(epa.get_fill(idX, qty, 99.98));
		assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == _ordStatus);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == expected_leaves && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == qty && clt.exe.lastPx() == 99.98);
		// validate status
		plasma.OnMsg(osr);
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == _ordStatus);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == expected_leaves && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
	}
	void done() {
		_ordStatus = OrdStatus::Done_For_Day;
		plasma.OnMsg(epa.get_done(idX));
		assert(clt.exe.execType() == ExecType::Done_For_Day && clt.exe.ordStatus() == OrdStatus::Done_For_Day);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 0 && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// validate status
		plasma.OnMsg(osr);
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == OrdStatus::Done_For_Day);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 0 && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == 99.98);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);

	}
	void resend() {
		double expected_leaves = nos.qty() - _execQty;
		double avgPx = (_execQty != 0) ? 99.98 : 0;
		plasma.OnMsg(nos);
		assert(clt.exe.execType() == ExecType::Rejected && clt.exe.ordStatus() == _ordStatus);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == expected_leaves && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
	}
public:
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//  Reject			(X)
	//	https://www.onixs.biz/fix-dictionary/4.4/app_dA.1.a.html
	void nos_rjt() {
		new_order();
		reject();
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//  Pending Ack		(X)
	//	Reject			(X)
	//	https://www.onixs.biz/fix-dictionary/4.4/app_dA.1.a.html
	void nos_pnd_new_rjt() {
		new_order();
		pending_ack();
		reject();
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//  Pending Ack		(X)
	//  Ack				(X)
	//	Reject			(X)
	//	https://www.onixs.biz/fix-dictionary/4.4/app_dA.1.a.html
	void nos_pnd_new_new_rjt() {
		new_order();
		pending_ack();
		ack();
		reject();
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//  Ack				(X)
	//	Fill			(X)
	//	https://www.onixs.biz/fix-dictionary/4.4/app_dA.1.a.html
	void nos_new_fill() {
		new_order();
		// ack order
		ack();
		// parital fill 2000
		fill(2000);
		// parital fill 1000
		fill(1000);
		// complete fill 7000
		fill(7000);
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//  Ack				(X)
	//	Fill			(X)
	//	Fill			(X)
	//  DoneForDay		(X)
	//	https://www.onixs.biz/fix-dictionary/4.4/app_dA.1.b.html
	void nos_new_fill_done() {
		new_order();
		// ack order
		ack();
		// parital fill 2000
		fill(2000);
		// parital fill 1000
		fill(1000);
		// done for day
		done();
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//	Ack				(X)
	//  NewOrderSingle	(X)
	//	Fill			(X)
	//  NewOrderSingle	(X)
	//	https://www.onixs.biz/fix-dictionary/4.2/app_d22.html	
	void nos_duplicate() {
		new_order();
		// ack order
		ack();
		// resend
		resend();
		// parital fill 2000
		fill(2000);
		// resend
		resend();
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
TEST_F(TestSuiteA1, nos_duplicate) {
	nos_duplicate();
}
