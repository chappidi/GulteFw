#pragma once
#include "ICallback.h"
#include "messages.h"
/////////////////////////////////////////////////////////////////////////
//
//
struct IRequest {
	PROXY<OrderStatusRequest> osr;
	virtual void pending() = 0;
	virtual void accept() = 0;
	virtual void reject() = 0;
	virtual void status() = 0;
};
/////////////////////////////////////////////////////////////////////////
//
//
struct Order_Req : public IRequest {
	plasma::ICallback& oms;
	Order_Req(plasma::ICallback& pls) : oms(pls) {

	}
	void fill(double qty) {
		PROXY<ExecutionReport> rpt;
	}
	void done() {
		PROXY<ExecutionReport> rpt;
	}
	void canceled() {
		PROXY<ExecutionReport> rpt;
	}
};
/////////////////////////////////////////////////////////////////////////
//
//
struct NewOrderReq {
	plasma::ICallback& oms;
	ISource& _src;
	ITarget& _tgt;
	PROXY<NewOrderSingle> nos;
	PROXY<OrderStatusRequest> osr;
	EOrder* order;
	NewOrderReq(plasma::ICallback& pls, ISource& src, ITarget& tgt, double qty, uint32_t prntid = 0)
		: oms(pls), _src(src), _tgt(tgt) {
		// fill in nos
		nos.parent(prntid);
		nos.clOrdId(_src.req_seq_no());
		nos.target(_tgt.id());
		nos.symbol(9999);
		nos.side(plasma::client::Side::BUY);
		nos.qty(qty);
		// fill in osr
		osr.clOrdId(nos.clOrdId());
		osr.symbol(nos.symbol());
		osr.side(nos.side());
		osr.qty(nos.qty());
		//publish nos
		oms.OnMsg(nos);
		order = _tgt.order();
		std::cout << "[" << ClientId(nos.clOrdId()) << "-->" << order->_clOrdId << "]" << std::endl;

		check(OrdStatus::Value::NA, 0,0);
	}
	auto slice(ITarget& tgt, double qty) {
		return 0;
	}
	auto cancel() {
		return 0;
	}
	auto replace(double qty) {
		return 0;
	}
	void pending() {
		order->_status = OrdStatus::Pending_New;

		PROXY<ExecutionReport> rpt;
		rpt << *order;
		rpt.execId(_tgt.rpt_seq_no());
		rpt.execType(ExecType::Pending_New);
		//publish rpt
		oms.OnMsg(rpt);
		auto exe = _src.execRpt();
		assert(exe.execType() == ExecType::Pending_New && exe.ordStatus() == OrdStatus::Pending_New);
		assert(exe.origClOrdId() == 0 && exe.clOrdId() == nos.clOrdId() && exe.orderId() == order->_clOrdId);
		assert(exe.leavesQty() == nos.qty() && exe.cumQty() == 0 && exe.avgPx() == 0);
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);

		check(OrdStatus::Value::Pending_New, 0,0);
	}
	void accept() {
		order->_status = OrdStatus::New;

		PROXY<ExecutionReport> rpt;
		rpt << *order;
		rpt.execId(_tgt.rpt_seq_no());
		rpt.execType(ExecType::New);
		//publish rpt
		oms.OnMsg(rpt);
		auto exe = _src.execRpt();
		assert(exe.execType() == ExecType::New && exe.ordStatus() == OrdStatus::New);
		assert(exe.origClOrdId() == 0 && exe.clOrdId() == nos.clOrdId() && exe.orderId() == order->_clOrdId);
		assert(exe.leavesQty() == nos.qty() && exe.cumQty() == 0 && exe.avgPx() == 0);
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);

		check(OrdStatus::Value::New, 0, 0);
	}
	void reject() {
		PROXY<ExecutionReport> rpt;
		rpt << *order;
		rpt.execId(_tgt.rpt_seq_no());
		rpt.execType(ExecType::Rejected);
		rpt.leavesQty(0);
		//publish rpt
		oms.OnMsg(rpt);

		check(OrdStatus::Value::NA, 0, 0);
	}
	void fill(double qty) {
		PROXY<ExecutionReport> rpt;
		rpt << *order;
		rpt.execId(_tgt.rpt_seq_no());
		rpt.execType(ExecType::Trade);
//		rpt.lastQty(lastQty);
//		rpt.lastPx(lastPx);
		//publish rpt
		oms.OnMsg(rpt);


		check(OrdStatus::Value::NA, 0, 0);
	}
	void done() {
		PROXY<ExecutionReport> rpt;
		rpt << *order;
		rpt.execId(_tgt.rpt_seq_no());
		rpt.execType(ExecType::Done_For_Day);
		rpt.leavesQty(0);
		//publish rpt
		oms.OnMsg(rpt);

		check(OrdStatus::Value::Done_For_Day, 0, 0);
	}
	void canceled() {
		PROXY<ExecutionReport> rpt;
		rpt << *order;
		rpt.execId(_tgt.rpt_seq_no());
		rpt.execType(ExecType::Canceled);
		rpt.leavesQty(0);
		//publish rpt
		oms.OnMsg(rpt);

		check(OrdStatus::Value::Canceled, 0, 0);
	}
	virtual void check(OrdStatus::Value ordSts, double lastQty, double lastPx) const {
		// publish order status request
		oms.OnMsg(osr);
		// wait & check the response
		auto exe = _src.execRpt();
		assert(exe.execType() == ExecType::Order_Status && exe.ordStatus() == ordSts);
		assert(exe.origClOrdId() == 0 && exe.clOrdId() == nos.clOrdId() && exe.orderId() == order->_clOrdId);
		assert(exe.leavesQty() == nos.qty() && exe.cumQty() == 0 && exe.avgPx() == 0);
		assert(exe.lastQty() == lastQty && exe.lastPx() == lastPx);
 	}
};
/////////////////////////////////////////////////////////////////////////
//
//
struct ChildOrderReq : public NewOrderReq {
	const NewOrderReq& _prnt;
	ChildOrderReq(const NewOrderReq& prnt, ITarget& tgt, double qty)
		:NewOrderReq(prnt.oms, dynamic_cast<ISource&>(prnt._tgt), tgt, qty, prnt.order->_clOrdId),
		_prnt(prnt) {
		// blank
	}
	void check(OrdStatus::Value ordSts, double lastQty, double lastPx) const {
		NewOrderReq::check(OrdStatus::Value::NA, 0, 0);
		_prnt.check(OrdStatus::Value::NA, 0, 0);
	}
};
/////////////////////////////////////////////////////////////////////////
//
//
struct ReplaceReq : public Order_Req {
	PROXY<OrderReplaceRequest> orr;
	ReplaceReq(const Order_Req& or, double qty) : Order_Req(or.oms) {
	}
	void pending() {
		PROXY<ExecutionReport> rpt;
	}
	void accept() {
		PROXY<ExecutionReport> rpt;
	}
	void reject() {
		PROXY<OrderCancelReject> rjt;
	}
	void fill(double qty) {
	}
	void done() {
	}
	void canceled() {
	}
};
/////////////////////////////////////////////////////////////////////////
//
//
struct CancelReq : public IRequest {
	plasma::ICallback& oms;
	PROXY<OrderCancelRequest> ocr;
	CancelReq(const Order_Req& or) : oms(or.oms) {
/*
		ocr.clOrdId(req_id++);
		ocr.origClOrdId(orr.clOrdId());
		ocr.symbol(orr.symbol());
		ocr.side(orr.side());
		ocr.qty(orr.qty());
		ocr.orderId(ordId);
*/
	}
	void pending() {
		PROXY<ExecutionReport> rpt;
	}
	void accept() {
		PROXY<ExecutionReport> rpt;
	}
	void reject() {
		PROXY<OrderCancelReject> rjt;
	}
	void status() {

	}
};

