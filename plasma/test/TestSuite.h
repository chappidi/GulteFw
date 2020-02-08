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
	map<uint32_t, PROXY<OrderCancelRequest>> ocrs;
	map<uint32_t, PROXY<OrderReplaceRequest>> orrs;
	map<uint32_t, map<uint32_t, OrdStatus::Value>> osv;
	double					_execQty{ 0 };

	////////////////////////////////////////////////////////////////////////////////////////////
	// Constructor
	TestSuite() {
		// register
		plasma.OnLogin(clt);
		plasma.OnLogin(epa);
		//create 
		nos = clt.get_nos(epa.id(), 10000);
	}
#pragma region CLIENT
	////////////////////////////////////////////////////////////////////////////////////////////
	// New Order and Validate Satus
	auto new_order() {
		//send new order
		plasma.OnMsg(nos);
		std::cout << "[" << ClientId(nos.clOrdId()) << "-->" << epa.ClOrdId << "]" << std::endl;
		osv[epa.ClOrdId][0] = OrdStatus::NA;

		// validate status
		plasma.OnMsg(clt.get_sts(nos));
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == OrdStatus::NA);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == epa.ClOrdId);
		assert(clt.exe.leavesQty() == nos.qty() && clt.exe.cumQty() == 0 && clt.exe.avgPx() == 0);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);

		//return the plasma id
		return epa.ClOrdId;
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	// Cancel NewOrderReq or ReplaceOrderReq and Validate Status
	auto cxl_order(uint32_t idX) {
		bool isRpl = (orrs.find(idX) != orrs.end());
		//send cancel request
		auto ocr =  (isRpl == true)? clt.get_cxl(idX, orrs[idX]) : clt.get_cxl(idX, nos);
		plasma.OnMsg(ocr);
		std::cout << "[" << ClientId(ocr.clOrdId()) << "-->" << epa.ClOrdId << "]" << std::endl;
		ocrs[epa.ClOrdId] = ocr;

		// only if the orig order is found 
		if (epa.ClOrdId != 0) {
			double expected_leaves = ((isRpl == true) ? orrs[idX].qty() : nos.qty()) - _execQty;
			double avgPx = (_execQty != 0) ? 99.98 : 0;
			// validate status
			plasma.OnMsg(clt.get_sts(nos));
			assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == osv[idX][0]);
			assert(clt.exe.origClOrdId() == ((isRpl == true) ? nos.clOrdId() : 0) && clt.exe.clOrdId() == ((isRpl == true) ? orrs[idX].clOrdId() : nos.clOrdId()) && clt.exe.orderId() == idX);
			assert(clt.exe.leavesQty() == expected_leaves && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
			assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		}
		//return the plasma id
		return epa.ClOrdId;
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	// Replace NewOrderReq or ReplaceOrderReq and Validate Status
	auto rpl_order(uint32_t idX, double qty) {
		bool isRpl = (orrs.find(idX) != orrs.end());
		auto ord_qty = (isRpl == true) ? orrs[idX].qty() : nos.qty();
		// send replace request
		auto orr = (isRpl == true) ? clt.get_rpl(idX, qty, orrs[idX]) : clt.get_rpl(idX, qty, nos);
		plasma.OnMsg(orr);
		std::cout << "[" << ClientId(orr.clOrdId()) << "-->" << epa.ClOrdId << "]" << std::endl;
		orrs[epa.ClOrdId] = orr;

		// only if the orig order is found 
		if (epa.ClOrdId != 0) {
			double expected_leaves = ord_qty - _execQty;
			double avgPx = (_execQty != 0) ? 99.98 : 0;
			// validate status
			plasma.OnMsg(clt.get_sts(nos));
			assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == osv[idX][0]);
			assert(clt.exe.origClOrdId() == ((isRpl == true) ? nos.clOrdId() : 0) && clt.exe.clOrdId() == ((isRpl == true) ? orrs[idX].clOrdId() : nos.clOrdId()) && clt.exe.orderId() == idX);
			assert(clt.exe.leavesQty() == expected_leaves && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
			assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		}
		//return the plasma id
		return epa.ClOrdId;
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	// Resend New order again
	void resend_nos(uint32_t idX) {
		double expected_leaves = nos.qty() - _execQty;
		double avgPx = (_execQty != 0) ? 99.98 : 0;

		plasma.OnMsg(nos);
		assert(clt.exe.execType() == ExecType::Rejected && clt.exe.ordStatus() == osv[idX][0]);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == expected_leaves && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
	}
	// Resend Cxl
	// Resend Replace
#pragma endregion

#pragma region EPA
	////////////////////////////////////////////////////////////////////////////////////////////
	// reject, pending_new, new, partial_fill, filled, doneforday, cancelled
#pragma region NEW_ORDER
	void pending_ack(uint32_t idX) {
		osv[idX][0] = OrdStatus::Pending_New;

		//send epa response & check client report
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
		osv[idX][0] = OrdStatus::New;

		//send epa response & check client report
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
	////////////////////////////////////////////////////////////////////////////////////////////
	// Fill a NewOrder or ReplaceOrder
	void fill(uint32_t idX, double qty) {
		bool isRpl = (orrs.find(idX) != orrs.end());
		auto ord_qty = (isRpl == true) ? orrs[idX].qty() : nos.qty();

		_execQty += qty;
		double expected_leaves = ord_qty - _execQty;
		double avgPx = (_execQty != 0) ? 99.98 : 0;
		osv[idX][0] = (_execQty == ord_qty) ? OrdStatus::Filled : OrdStatus::Partially_Filled;
		auto ordStatus = osv[idX][0];
		// if there are any cxl/rpl requests pending, then its status is reported
		for (const auto& [k, v] : osv[idX]) {
			if(k != 0)
				ordStatus = v;
		}

		//send epa response & check client report
		plasma.OnMsg(epa.get_fill(idX, qty, 99.98));
		assert(clt.exe.execType() == ExecType::Trade && clt.exe.ordStatus() == ordStatus);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == (isRpl == true ? orrs[idX].clOrdId() : nos.clOrdId()) && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == expected_leaves && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
		assert(clt.exe.lastQty() == qty && clt.exe.lastPx() == 99.98);
		// validate status
		plasma.OnMsg(clt.get_sts(nos));
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == ordStatus);
		assert(clt.exe.origClOrdId() == ((isRpl == true) ? nos.clOrdId() : 0) && clt.exe.clOrdId() == (isRpl == true ? orrs[idX].clOrdId() : nos.clOrdId()) && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == expected_leaves && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	// Done a NewOrder or ReplaceOrder
	// Order is not cancel. can be restated next day
	void done(uint32_t idX) {
		bool isRpl = (orrs.find(idX) != orrs.end());
		osv[idX][0] = OrdStatus::Done_For_Day;
		double avgPx = (_execQty != 0) ? 99.98 : 0;

		//send epa response & check client report
		plasma.OnMsg(epa.get_done(idX));
		assert(clt.exe.execType() == ExecType::Done_For_Day && clt.exe.ordStatus() == OrdStatus::Done_For_Day);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == (isRpl == true ? orrs[idX].clOrdId() : nos.clOrdId()) && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 0 && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// validate status
		plasma.OnMsg(clt.get_sts(nos));
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == OrdStatus::Done_For_Day);
		assert(clt.exe.origClOrdId() == ((isRpl == true) ? nos.clOrdId() : 0) && clt.exe.clOrdId() == (isRpl == true ? orrs[idX].clOrdId() : nos.clOrdId()) && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 0 && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	// Terminal State
	void reject(uint32_t idX) {
		osv[idX][0] = OrdStatus::Rejected;

		//send epa response & check client report
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
	////////////////////////////////////////////////////////////////////////////////////////////
	// Cancelled a NewOrder or ReplaceOrder
	// Terminal State. Unsolicited Cxl by EPA
	void cxld(uint32_t idX) {
		bool isRpl = (orrs.find(idX) != orrs.end());
		osv[idX][0] = OrdStatus::Canceled;
		double avgPx = (_execQty != 0) ? 99.98 : 0;

		//send epa response & check client report
		plasma.OnMsg(epa.get_cxld(idX));
		assert(clt.exe.execType() == ExecType::Canceled && clt.exe.ordStatus() == OrdStatus::Canceled);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == (isRpl == true ? orrs[idX].clOrdId() : nos.clOrdId()) && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 0 && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// validate status
		plasma.OnMsg(clt.get_sts(nos));
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == OrdStatus::Canceled);
		assert(clt.exe.origClOrdId() == ((isRpl == true) ? nos.clOrdId() : 0) && clt.exe.clOrdId() == nos.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 0 && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
	}
#pragma endregion
	////////////////////////////////////////////////////////////////////////////////////////////
	// Cancel Reject a NewOrder or ReplaceOrder
	// reject, pending_cancel, cancelled
#pragma region CANCEL_ORDER
	void cxl_rjt(uint32_t idY, uint32_t idX, const std::string& reason) {
		bool isRpl = (orrs.find(idX) != orrs.end());
		auto& ocr = ocrs[idY];
		double avgPx = (_execQty != 0) ? 99.98 : 0;
		// only if pending_cancel is sent
		if (osv[idX].find(idY) != osv[idX].end()) {
			osv[idX].erase(idY);
		}
		auto ordStatus = osv[idX][0];
		// if there are any cxl/rpl requests pending, then its status is reported
		for (const auto& [k, v] : osv[idX]) {
			if (k != 0)
				ordStatus = v;
		}
		double expected_leaves = (ordStatus == OrdStatus::Canceled) ? 0 : (nos.qty() - _execQty);


		//send epa response & check client report
		plasma.OnMsg(epa.get_rjt(idY, idX, reason));
		assert(clt.rjt.status() == ordStatus);
		assert(clt.rjt.origClOrdId() == nos.clOrdId() && clt.rjt.clOrdId() == ocr.clOrdId() && clt.rjt.orderId() == idX);
		// validate status of nos request
		plasma.OnMsg(clt.get_sts(nos));
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == ordStatus);
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
	////////////////////////////////////////////////////////////////////////////////////////////
	// Pending Cancel a NewOrder or Replace Order
	void pending_cancel(uint32_t idY, uint32_t idX) {
		bool isRpl = (orrs.find(idX) != orrs.end());
		auto ord_qty = (isRpl == true) ? orrs[idX].qty() : nos.qty();
		auto& ocr = ocrs[idY];
		osv[idX][idY] = OrdStatus::Pending_Cancel;
		double expected_leaves = ord_qty - _execQty;
		double avgPx = (_execQty != 0) ? 99.98 : 0;


		//send epa response & check client report
		plasma.OnMsg(epa.get_pnd_cxl(idY, idX));
		assert(clt.exe.execType() == ExecType::Pending_Cancel && clt.exe.ordStatus() == OrdStatus::Pending_Cancel);
		assert(clt.exe.origClOrdId() == ((isRpl == true) ? orrs[idX].clOrdId() : nos.clOrdId()) && clt.exe.clOrdId() == ocr.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == expected_leaves && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// validate status of nos request
		plasma.OnMsg(clt.get_sts(nos));
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == OrdStatus::Pending_Cancel);
		// since it is pending, there could be multiple pending requests.
		assert(clt.exe.origClOrdId() == ((isRpl == true) ? nos.clOrdId() : 0) && clt.exe.clOrdId() == ((isRpl == true) ? orrs[idX].clOrdId() : nos.clOrdId()) && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == expected_leaves && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// validate status on cancel request
		plasma.OnMsg(clt.get_sts(ocr));
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == OrdStatus::Pending_Cancel);
		assert(clt.exe.origClOrdId() == ((isRpl == true) ? orrs[idX].clOrdId() : nos.clOrdId()) && clt.exe.clOrdId() == ocr.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == expected_leaves && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	// Canceled a NewOrder or Replace Order
	// Terminal State.
	void cxld(uint32_t idY, uint32_t idX) {
		bool isRpl = (orrs.find(idX) != orrs.end());
		auto& ocr = ocrs[idY];
		// only if pending_cancel is sent
		//TODO: remove all requests where idY != 0
		if (osv[idX].find(idY) != osv[idX].end()) {
			osv[idX].erase(idY);
		}
		osv[idX][0] = OrdStatus::Canceled;
		double avgPx = (_execQty != 0) ? 99.98 : 0;


		//send epa response & check client report
		plasma.OnMsg(epa.get_cxld(idY, idX));
		assert(clt.exe.execType() == ExecType::Canceled && clt.exe.ordStatus() == OrdStatus::Canceled);
		assert(clt.exe.origClOrdId() == ((isRpl == true) ? orrs[idX].clOrdId() : nos.clOrdId()) && clt.exe.clOrdId() == ocr.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 0 && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// validate status of nos request
		plasma.OnMsg(clt.get_sts(nos));
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == OrdStatus::Canceled);
		assert(clt.exe.origClOrdId() == ((isRpl == true) ? nos.clOrdId() : 0) && clt.exe.clOrdId() == ((isRpl == true) ? orrs[idX].clOrdId() : nos.clOrdId()) && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == 0 && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// validate status on cancel request
		plasma.OnMsg(clt.get_sts(ocr));
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == OrdStatus::Canceled);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == ocr.clOrdId() && clt.exe.orderId() == idY);
		assert(clt.exe.leavesQty() == 0 && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
	}
#pragma endregion
	////////////////////////////////////////////////////////////////////////////////////////////
	// reject, pending_replace, new, partial_fill, filled
#pragma region REPLACE_ORDER
	////////////////////////////////////////////////////////////////////////////////////////////
	// Replace Reject a NewOrder or Replace Order
	void rpl_rjt(uint32_t idY, uint32_t idX, const std::string& reason) {
		bool isRpl = (orrs.find(idX) != orrs.end());
		auto& orr = orrs[idY];
		double avgPx = (_execQty != 0) ? 99.98 : 0;
		// only if pending_replace is sent
		if (osv[idX].find(idY) != osv[idX].end()) {
			osv[idX].erase(idY);
		}
		auto ordStatus = osv[idX][0];
		// if there are any cxl/rpl requests pending, then its status is reported
		for (const auto& [k, v] : osv[idX]) {
			if (k != 0)
				ordStatus = v;
		}
		double expected_leaves = nos.qty() - _execQty;


		//send epa response & check client report
		plasma.OnMsg(epa.get_rjt(idY, idX, reason));
		assert(clt.rjt.status() == osv[idX][0]);
		assert(clt.rjt.origClOrdId() == nos.clOrdId() && clt.rjt.clOrdId() == orr.clOrdId() && clt.rjt.orderId() == idX);
		// validate status
		plasma.OnMsg(clt.get_sts(nos));
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == osv[idX][0]);
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
	////////////////////////////////////////////////////////////////////////////////////////////
	// Pending Replace a NewOrder or Replace Order
	void pending_replace(uint32_t idY, uint32_t idX) {
		bool isRpl = (orrs.find(idX) != orrs.end());
		auto ord_qty = (isRpl == true) ? orrs[idX].qty() : nos.qty();
		auto& orr = orrs[idY];
		osv[idX][idY] = OrdStatus::Pending_Replace;
		double expected_leaves = ord_qty - _execQty;
		double avgPx = (_execQty != 0) ? 99.98 : 0;


		//send epa response & check client report
		plasma.OnMsg(epa.get_pnd_rpl(idY, idX));
		assert(clt.exe.execType() == ExecType::Pending_Replace && clt.exe.ordStatus() == OrdStatus::Pending_Replace);
		assert(clt.exe.origClOrdId() == ((isRpl == true) ? orrs[idX].clOrdId() : nos.clOrdId()) && clt.exe.clOrdId() == orr.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == expected_leaves && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// validate status
		plasma.OnMsg(clt.get_sts(nos));
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == OrdStatus::Pending_Replace);
		// since it is pending, there could be multiple pending requests.
		assert(clt.exe.origClOrdId() == ((isRpl == true) ? nos.clOrdId() : 0) && clt.exe.clOrdId() == ((isRpl == true) ? orrs[idX].clOrdId() : nos.clOrdId()) && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == expected_leaves && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// validate status
		plasma.OnMsg(clt.get_sts(orr));
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == OrdStatus::Pending_Replace);
		assert(clt.exe.origClOrdId() == ((isRpl == true) ? orrs[idX].clOrdId() : nos.clOrdId()) && clt.exe.clOrdId() == orr.clOrdId() && clt.exe.orderId() == idX);
		assert(clt.exe.leavesQty() == expected_leaves && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	// Replaced a NewOrder or Replace Order
	// Start of New Order.
	void replaced(uint32_t idY, uint32_t idX) {
		bool isRpl = (orrs.find(idX) != orrs.end());
		auto& orr = orrs[idY];
		osv[idY][0] = osv[idX][0];
		double avgPx = (_execQty != 0) ? 99.98 : 0;
		double expected_leaves = orr.qty() - _execQty;

		//send epa response & check client report
		plasma.OnMsg(epa.get_rpld(idY, idX));
		assert(clt.exe.execType() == ExecType::Replace && clt.exe.ordStatus() == osv[idY][0]);
		assert(clt.exe.origClOrdId() == ((isRpl == true) ? orrs[idX].clOrdId() : nos.clOrdId()) && clt.exe.clOrdId() == orr.clOrdId() && clt.exe.orderId() == idY);
		assert(clt.exe.leavesQty() == expected_leaves && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// validate status
		plasma.OnMsg(clt.get_sts(nos));
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == osv[idY][0]);
		assert(clt.exe.origClOrdId() == nos.clOrdId() && clt.exe.clOrdId() == orr.clOrdId() && clt.exe.orderId() == idY);
		assert(clt.exe.leavesQty() == expected_leaves && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
		// validate status
		plasma.OnMsg(clt.get_sts(orr));
		assert(clt.exe.execType() == ExecType::Order_Status && clt.exe.ordStatus() == osv[idY][0]);
		assert(clt.exe.origClOrdId() == 0 && clt.exe.clOrdId() == orr.clOrdId() && clt.exe.orderId() == idY);
		assert(clt.exe.leavesQty() == expected_leaves && clt.exe.cumQty() == _execQty && clt.exe.avgPx() == avgPx);
		assert(clt.exe.lastQty() == 0 && clt.exe.lastPx() == 0);
	}
#pragma endregion
#pragma endregion
};
