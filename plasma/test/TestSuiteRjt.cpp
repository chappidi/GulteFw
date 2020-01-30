#include <plasma.h>
#include "GUI.h"
#include "EPA.h"
#include <gtest/gtest.h>
#include <stack>

struct TestSuiteRJT : public testing::Test
{
	// declare variables
	plasma::OMS plasma;
	GUI clt;
	EPA epa;
	PROXY<NewOrderSingle>		nos;
	PROXY<OrderCancelRequest>	ocr;
	PROXY<OrderReplaceRequest>	orr;
	double						_execQty{ 0 };
	OrdStatus::Value			_ordStatus{ OrdStatus::NA };
	stack<OrdStatus::Value>		stack;
	TestSuiteRJT() {
		// register
		plasma.OnLogin(clt);
		plasma.OnLogin(epa);

		// create requests
		nos = clt.get_nos(epa.id(), 10000);
	}
	auto new_order() {
		// New Order(X)	
		plasma.OnMsg(nos);
		std::cout << "[" << ClientId(nos.clOrdId()) << "-->" << epa.ClOrdId << "]" << std::endl;
		// validate status
		plasma.OnMsg(clt.get_sts(nos));
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == OrdStatus::NA);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == epa.ClOrdId);
		assert(clt.exe.leavesQty() == nos.qty() && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		return epa.ClOrdId;
	}
	void pending_ack(uint32_t idX) {
		_ordStatus = OrdStatus::Pending_New;
		plasma.OnMsg(epa.get_pnd_new(idX));
		assert(clt.exe.execType() == ExecType::Pending_New && clt.exe.ordStatus() == OrdStatus::Pending_New);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == nos.qty() && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// validate status
		plasma.OnMsg(clt.get_sts(nos));
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == OrdStatus::Pending_New);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == nos.qty() && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
	}
	void ack(uint32_t idX) {
		_ordStatus = OrdStatus::New;
		plasma.OnMsg(epa.get_new(idX));
		assert(clt.exe.execType() == ExecType::New && clt.exe.ordStatus() == OrdStatus::New);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == nos.qty() && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// validate status
		plasma.OnMsg(clt.get_sts(nos));
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == OrdStatus::New);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == nos.qty() && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
	}
	void reject(uint32_t idX) {
		_ordStatus = OrdStatus::Rejected;
		plasma.OnMsg(epa.get_rjt(idX));
		assert(clt.exe.execType() == ExecType::Rejected && clt.exe.ordStatus() == OrdStatus::Rejected);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 0 && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// validate status
		plasma.OnMsg(clt.get_sts(nos));
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == OrdStatus::Rejected);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 0 && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
	}
	auto cxl_order(uint32_t idX) {
		double expected_leaves = nos.qty() - _execQty;
		double avgPx = (_execQty != 0) ? 99.98 : 0;

		ocr = clt.get_cxl(idX, nos);
		plasma.OnMsg(ocr);
		std::cout << "[" << ClientId(ocr.clOrdId()) << "-->" << epa.ClOrdId << "]" << std::endl;
		// validate status
		plasma.OnMsg(clt.get_sts(nos));
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == _ordStatus);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == expected_leaves && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);

		return epa.ClOrdId;
	}
	auto rpl_order(uint32_t idX, double qty) {
		double expected_leaves = nos.qty() - _execQty;
		double avgPx = (_execQty != 0) ? 99.98 : 0;

		orr = clt.get_rpl(idX, qty, nos);
		plasma.OnMsg(orr);
		std::cout << "[" << ClientId(orr.clOrdId()) << "-->" << epa.ClOrdId << "]" << std::endl;
		// validate status
		plasma.OnMsg(clt.get_sts(nos));
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == _ordStatus);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == expected_leaves && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);

		return epa.ClOrdId;
	}
	void cxl_rjt(uint32_t idY, uint32_t idX, const std::string& reason) {
		double expected_leaves = nos.qty() - _execQty;
		double avgPx = (_execQty != 0) ? 99.98 : 0;
		if (!stack.empty())
			stack.pop();
		if (!stack.empty())
			_ordStatus = stack.top();
		else if (_execQty == nos.qty())
			_ordStatus = OrdStatus::Filled;
		else if(_execQty != 0)
			_ordStatus = OrdStatus::Partially_Filled;


		plasma.OnMsg(epa.get_rjt(idY, idX, reason));
		assert(clt.rjt.status() == _ordStatus);
		assert(clt.rjt.origClOrdId() == nos.clOrdId() && clt.rjt.clOrdId() == ocr.clOrdId() && clt.rjt.orderId() == idX);
		// validate status
		plasma.OnMsg(clt.get_sts(nos));
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == _ordStatus);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == expected_leaves && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);

		// validate status on cancel request
		plasma.OnMsg(clt.get_sts(ocr));
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == OrdStatus::Rejected);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == ocr.clOrdId() && clt.exe.orderId() == idY);
		assert(clt.exe.leavesQty() == 0 && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
	}
	void rpl_rjt(uint32_t idY, uint32_t idX, const std::string& reason) {
		double expected_leaves = nos.qty() - _execQty;
		double avgPx = (_execQty != 0) ? 99.98 : 0;
		if (!stack.empty())
			stack.pop();
		if (!stack.empty())
			_ordStatus = stack.top();
		else if (_execQty == nos.qty())
			_ordStatus = OrdStatus::Filled;
		else if (_execQty != 0)
			_ordStatus = OrdStatus::Partially_Filled;


		plasma.OnMsg(epa.get_rjt(idY, idX, reason));
		assert(clt.rjt.status() == _ordStatus);
		assert(clt.rjt.origClOrdId() == nos.clOrdId() && clt.rjt.clOrdId() == orr.clOrdId() && clt.rjt.orderId() == idX);
		// validate status
		plasma.OnMsg(clt.get_sts(nos));
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == _ordStatus);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == expected_leaves && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);

		// validate status on cancel request
		plasma.OnMsg(clt.get_sts(orr));
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == OrdStatus::Rejected);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == orr.clOrdId() && clt.exe.orderId() == idY);
		assert(clt.exe.leavesQty() == 0 && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
	}
public:
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//  Reject			(X)
	//	https://www.onixs.biz/fix-dictionary/4.4/app_dA.1.a.html
	void nos_rjt() {
		auto idX = new_order();
		reject(idX);
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//  Pending Ack		(X)
	//	Reject			(X)
	//	https://www.onixs.biz/fix-dictionary/4.4/app_dA.1.a.html
	void nos_pnd_rjt() {
		auto idX = new_order();
		pending_ack(idX);
		reject(idX);
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//  Pending Ack		(X)
	//  Ack				(X)
	//	Reject			(X)
	//	https://www.onixs.biz/fix-dictionary/4.4/app_dA.1.a.html
	void nos_pnd_ack_rjt() {
		auto idX = new_order();
		pending_ack(idX);
		ack(idX);
		reject(idX);
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//  Pending Ack		(X)
	//  Ack				(X)
	//	Reject			(X)
	//	https://www.onixs.biz/fix-dictionary/4.4/app_dA.1.a.html
	void nos_ack_rjt() {
		auto idX = new_order();
		ack(idX);
		reject(idX);
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//  Cancel Request	(Y,X)
	//	Cancel Reject	(Y,X)
	//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.a.html
	void nos_cxl_rjt() {
		auto idX = new_order();
		auto idY = cxl_order(idX);
		cxl_rjt(idY, idX, "");
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//  Cancel Request	(Y,X)
	//  Pending Ack		(X)
	//	Cancel Reject	(Y,X)
	//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.a.html
	void nos_pnd_cxl_rjt() {
		auto idX = new_order();
		auto idY = cxl_order(idX);
		pending_ack(idX);
		cxl_rjt(idY, idX, "");
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//  Ack				(X)
	//  Cancel Request	(Y,X)
	//	Cancel Reject	(Y,X)
	//	https://www.onixs.biz/fix-dictionary/4.4/app_dB.1.a.html
	void nos_new_cxl_rjt() {
		auto idX = new_order();
		ack(idX);
		auto idY = cxl_order(idX);
		cxl_rjt(idY, idX, "");
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//  Replace Request	(Y,X)
	//	Replace Reject	(Y,X)
	// https://www.onixs.biz/fix-dictionary/4.4/app_dC.1.a.html
	void nos_rpl_rjt() {
		auto idX = new_order();
		auto idY = rpl_order(idX, 11000);
		rpl_rjt(idY, idX, "");
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//  Replace Request	(Y,X)
	//  Pending Ack		(X)
	//	Replace Reject	(Y,X)
	// https://www.onixs.biz/fix-dictionary/4.4/app_dC.1.a.html
	void nos_pnd_rpl_rjt() {
		auto idX = new_order();
		auto idY = rpl_order(idX, 11000);
		pending_ack(idX);
		rpl_rjt(idY, idX, "");
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//  Ack				(X)
	//  Replace Request	(Y,X)
	//	Replace Reject	(Y,X)
	// https://www.onixs.biz/fix-dictionary/4.4/app_dC.1.a.html
	void nos_new_rpl_rjt() {
		auto idX = new_order();
		ack(idX);
		auto idY = rpl_order(idX, 11000);
		rpl_rjt(idY, idX, "");
	}
};
// new order
TEST_F(TestSuiteRJT, nos_rjt) {
	nos_rjt();
}
TEST_F(TestSuiteRJT, nos_pnd_rjt) {
	nos_pnd_rjt();
}
TEST_F(TestSuiteRJT, nos_pnd_ack_rjt) {
	nos_pnd_ack_rjt();
}
TEST_F(TestSuiteRJT, nos_ack_rjt) {
	nos_ack_rjt();
}
// cancel
TEST_F(TestSuiteRJT, nos_cxl_rjt) {
	nos_cxl_rjt();
}
TEST_F(TestSuiteRJT, nos_pnd_cxl_rjt) {
	nos_pnd_cxl_rjt();
}
TEST_F(TestSuiteRJT, nos_new_cxl_rjt) {
	nos_new_cxl_rjt();
}
// replace
TEST_F(TestSuiteRJT, nos_rpl_rjt) {
	nos_rpl_rjt();
}
TEST_F(TestSuiteRJT, nos_pnd_rpl_rjt) {
	nos_pnd_rpl_rjt();
}
TEST_F(TestSuiteRJT, nos_new_rpl_rjt) {
	nos_new_rpl_rjt();
}
