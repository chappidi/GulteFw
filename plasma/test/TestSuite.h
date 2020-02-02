#pragma once
#include <plasma.h>
#include "GUI.h"
#include "EPA.h"
#include <gtest/gtest.h>
#include <stack>

struct TestSuite : public testing::Test
{
	// declare variables
	const string reason = "";
	plasma::OMS plasma;
	GUI clt;
	EPA epa;
	PROXY<NewOrderSingle>		nos;
	PROXY<OrderCancelRequest>	ocr;
	PROXY<OrderReplaceRequest>	orr;
	double						_execQty{ 0 };
	OrdStatus::Value			_ordStatus{ OrdStatus::NA };
	stack<OrdStatus::Value>		stack;
	TestSuite() {
		// register
		plasma.OnLogin(clt);
		plasma.OnLogin(epa);

		// create requests
		nos = clt.get_nos(epa.id(), 10000);
	}
	auto new_order() {
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
		stack.push(OrdStatus::New);
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
		if (!stack.empty() && stack.top() == OrdStatus::Pending_Cancel)
			stack.pop();
		if (!stack.empty())
			_ordStatus = stack.top();
		else if (_execQty == nos.qty())
			_ordStatus = OrdStatus::Filled;
		else if (_execQty != 0)
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
	void pending_cancel(uint32_t idY, uint32_t idX) {
		_ordStatus = OrdStatus::Pending_Cancel;
		stack.push(OrdStatus::Pending_Cancel);

		double expected_leaves = nos.qty() - _execQty;
		double avgPx = (_execQty != 0) ? 99.98 : 0;

		plasma.OnMsg(epa.get_pnd_cxl(idY, idX));
		assert(clt.exe.execType() == ExecType::Pending_Cancel && clt.exe.ordStatus() == OrdStatus::Pending_Cancel);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == ocr.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == expected_leaves && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// validate status
		plasma.OnMsg(clt.get_sts(nos));
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == OrdStatus::Pending_Cancel);
		// since it is pending, there could be multiple pending requests.
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == expected_leaves && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// validate status
		plasma.OnMsg(clt.get_sts(ocr));
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == OrdStatus::Pending_Cancel);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == ocr.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == expected_leaves && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
	}
	void pending_replace(uint32_t idY, uint32_t idX) {
		_ordStatus = OrdStatus::Pending_Replace;
		stack.push(OrdStatus::Pending_Replace);

		double expected_leaves = nos.qty() - _execQty;
		double avgPx = (_execQty != 0) ? 99.98 : 0;

		plasma.OnMsg(epa.get_pnd_rpl(idY, idX));
		assert(clt.exe.execType() == ExecType::Pending_Replace && clt.exe.ordStatus() == OrdStatus::Pending_Replace);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == orr.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == expected_leaves && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// validate status
		plasma.OnMsg(clt.get_sts(nos));
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == OrdStatus::Pending_Replace);
		// since it is pending, there could be multiple pending requests.
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == expected_leaves && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// validate status
		plasma.OnMsg(clt.get_sts(orr));
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == OrdStatus::Pending_Replace);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == orr.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == expected_leaves && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
	}
	void fill(uint32_t idX, double qty) {
		_execQty += qty;
		if (stack.top() == OrdStatus::Pending_Cancel || stack.top() == OrdStatus::Pending_Replace)
			_ordStatus = stack.top();
		else {
			_ordStatus = (_execQty == nos.qty()) ? OrdStatus::Filled : OrdStatus::Partially_Filled;
			stack.push(_ordStatus);
		}
		double expected_leaves = nos.qty() - _execQty;
		double avgPx = (_execQty != 0) ? 99.98 : 0;

		// fill
		plasma.OnMsg(epa.get_fill(idX, qty, 99.98));
		assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == _ordStatus);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == expected_leaves && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
		assert(clt.exe.lastQty() == qty && clt.exe.lastPx() == 99.98);
		// validate status
		plasma.OnMsg(clt.get_sts(nos));
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == _ordStatus);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == expected_leaves && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
	}
	void done(uint32_t idX) {
		_ordStatus = OrdStatus::Done_For_Day;
		double avgPx = (_execQty != 0) ? 99.98 : 0;

		plasma.OnMsg(epa.get_done(idX));
		assert(clt.exe.execType() == ExecType::Done_For_Day && clt.exe.ordStatus() == OrdStatus::Done_For_Day);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 0 && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// validate status
		plasma.OnMsg(clt.get_sts(nos));
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == OrdStatus::Done_For_Day);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 0 && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
	}
	void cxld(uint32_t idX) {
		_ordStatus = OrdStatus::Canceled;
		double avgPx = (_execQty != 0) ? 99.98 : 0;

		plasma.OnMsg(epa.get_cxld(idX));
		assert(clt.exe.execType() == ExecType::Canceled && clt.exe.ordStatus() == OrdStatus::Canceled);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 0 && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// validate status
		plasma.OnMsg(clt.get_sts(nos));
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == OrdStatus::Canceled);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 0 && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
	}
	void resend_nos(uint32_t idX) {
		double expected_leaves = nos.qty() - _execQty;
		double avgPx = (_execQty != 0) ? 99.98 : 0;
		plasma.OnMsg(nos);
		assert(clt.exe.execType() == ExecType::Rejected && clt.exe.ordStatus() == _ordStatus);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == expected_leaves && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
	}
};
