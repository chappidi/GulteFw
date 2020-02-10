#pragma once
#include "ICallback.h"
#include "messages.h"
#include <Util.h>
/////////////////////////////////////////////////////////////////////////
//
//
struct IRequest {
	plasma::ICallback&	oms;
	ISource&			_src;
	ITarget&			_tgt;
	PROXY<OrderStatusRequest>	osr;
	PROXY<ExecutionReport>		sts;
	IRequest(plasma::ICallback& pls, ISource& src, ITarget& tgt)
		: oms(pls), _src(src), _tgt(tgt) {
	}
	virtual void pending() = 0;
	virtual void accept() = 0;
	virtual void reject() = 0;
	//////////////////////////////////////////////////////////////////////
	// lastQty and lastPx is always zero for status
	virtual void status() const {
		// publish order status request
		oms.OnMsg(osr);
		// wait & compare to expected value (sts)
		auto exe = _src.execRpt();
		assert(exe.execType() == ExecType::Order_Status && exe.ordStatus() == sts.ordStatus());
		assert(exe.origClOrdId() == sts.origClOrdId() && exe.clOrdId() == sts.clOrdId() && exe.orderId() == sts.orderId());
		assert(exe.leavesQty() == sts.leavesQty() && exe.cumQty() == sts.cumQty() && exe.avgPx() == sts.avgPx());
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);
	}
};
/////////////////////////////////////////////////////////////////////////
// Need Target to generate exec_rpt id
//
struct OrderReq : public IRequest {
	EOrder* order{ nullptr };
	OrderReq(plasma::ICallback& pls, ISource& src, ITarget& tgt)
		: IRequest(pls, src, tgt) {
	}
	void fill(double qty, double px = 99.98) {
		order->fill(qty, 99.98);
		PROXY<ExecutionReport> rpt;
		rpt << *order;
		rpt.execId(_tgt.rpt_seq_no());
		rpt.execType(ExecType::Trade);
		rpt.lastQty(qty);
		rpt.lastPx(px);
		// pending cancel requests
		if (order->_head_cxl != 0)
			rpt.ordStatus(OrdStatus::Pending_Cancel);
		// pending replace requests
		if (order->_head_rpl != 0)
			rpt.ordStatus(OrdStatus::Pending_Replace);

		//update status
		sts.cumQty(sts.cumQty() + qty);
		sts.leavesQty(sts.leavesQty() - qty);
		sts.avgPx(rpt.avgPx());
		sts.ordStatus(rpt.ordStatus());
		//publish rpt
		oms.OnMsg(rpt);
		auto exe = _src.execRpt();
		assert(exe.execType() == ExecType::Trade && exe.ordStatus() == sts.ordStatus());
		assert(exe.origClOrdId() == 0 && exe.clOrdId() == sts.clOrdId() && exe.orderId() == order->_clOrdId);
		assert(exe.leavesQty() == sts.leavesQty() && exe.cumQty() == sts.cumQty() && exe.avgPx() == sts.avgPx());
		assert(exe.lastQty() == qty && exe.lastPx() == px);

		//query and check
		status();
	}
	void done() {
		order->_status = OrdStatus::Done_For_Day;

		PROXY<ExecutionReport> rpt;
		rpt << *order;
		rpt.execId(_tgt.rpt_seq_no());
		rpt.execType(ExecType::Done_For_Day);
		rpt.leavesQty(0);
		//publish rpt
		oms.OnMsg(rpt);
		auto exe = _src.execRpt();
		assert(exe.execType() == ExecType::Done_For_Day && exe.ordStatus() == OrdStatus::Done_For_Day);
		assert(exe.origClOrdId() == 0 && exe.clOrdId() == sts.clOrdId() && exe.orderId() == order->_clOrdId);
		assert(exe.leavesQty() == 0 && exe.cumQty() == sts.cumQty() && exe.avgPx() == sts.avgPx());
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);

		//query and check
		sts.ordStatus(OrdStatus::Done_For_Day);
		sts.leavesQty(0);
		status();
	}
	void canceled() {
		order->_status = OrdStatus::Canceled;

		PROXY<ExecutionReport> rpt;
		rpt << *order;
		rpt.execId(_tgt.rpt_seq_no());
		rpt.execType(ExecType::Canceled);
		rpt.leavesQty(0);
		//publish rpt
		oms.OnMsg(rpt);
		auto exe = _src.execRpt();
		assert(exe.execType() == ExecType::Canceled && exe.ordStatus() == OrdStatus::Canceled);
		assert(exe.origClOrdId() == 0 && exe.clOrdId() == sts.clOrdId() && exe.orderId() == order->_clOrdId);
		assert(exe.leavesQty() == 0 && exe.cumQty() == sts.cumQty() && exe.avgPx() == sts.avgPx());
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);

		//query and check
		sts.ordStatus(OrdStatus::Canceled);
		sts.leavesQty(0);
		status();
	}
};
/////////////////////////////////////////////////////////////////////////
//
//
struct NewOrderReq : public OrderReq {
	PROXY<NewOrderSingle> nos;
	NewOrderReq(plasma::ICallback& pls, ISource& src, ITarget& tgt, double qty, uint32_t prntid = 0)
		: OrderReq(pls, src, tgt) {
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

		//query and check
		sts << nos;
		sts.ordStatus(OrdStatus::NA)
			.orderId(order->_clOrdId);
		status();
	}
	auto slice(ITarget& tgt, double qty) {
		return 0; // ChildOrderReq(*this, tgt, qty);
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

		//query and check
		sts.ordStatus(OrdStatus::Pending_New);
		status();
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

		//query and check
		sts.ordStatus(OrdStatus::New);
		status();
	}
	void reject() {
		order->_status = OrdStatus::Rejected;

		PROXY<ExecutionReport> rpt;
		rpt << *order;
		rpt.execId(_tgt.rpt_seq_no());
		rpt.execType(ExecType::Rejected);
		rpt.leavesQty(0);
		//publish rpt
		oms.OnMsg(rpt);
		auto exe = _src.execRpt();
		assert(exe.execType() == ExecType::Rejected && exe.ordStatus() == OrdStatus::Rejected);
		assert(exe.origClOrdId() == 0 && exe.clOrdId() == nos.clOrdId() && exe.orderId() == order->_clOrdId);
		assert(exe.leavesQty() == 0 && exe.cumQty() == 0 && exe.avgPx() == 0);
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);

		//query and check
		sts.ordStatus(OrdStatus::Rejected);
		sts.leavesQty(0);
		status();
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	// Resend New order again
	void resend() {

		oms.OnMsg(nos);
		auto exe = _src.execRpt();
		assert(exe.execType() == ExecType::Rejected && exe.ordStatus() == sts.ordStatus());
		assert(exe.origClOrdId() == 0 && exe.clOrdId() == nos.clOrdId() && exe.orderId() == order->_clOrdId);
		assert(exe.leavesQty() == sts.leavesQty() && exe.cumQty() == sts.cumQty() && exe.avgPx() == sts.avgPx());
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);
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
	void status() const {
		NewOrderReq::status();
		_prnt.status();
	}
};
/////////////////////////////////////////////////////////////////////////
//
//
struct ReplaceReq : public OrderReq {
	PROXY<OrderReplaceRequest> orr;
	ReplaceReq(const OrderReq& or, double qty) : OrderReq(or.oms, or._src, or._tgt) {
	}
	void pending() {
		PROXY<ExecutionReport> rpt;
		rpt.execId(_tgt.rpt_seq_no());
		rpt.execType(ExecType::Pending_Replace);
		rpt.ordStatus(OrdStatus::Pending_Replace);
		//publish rpt
		oms.OnMsg(rpt);
		auto exe = _src.execRpt();
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);
	}
	void accept() {
		PROXY<ExecutionReport> rpt;
		rpt.execId(_tgt.rpt_seq_no());
		rpt.execType(ExecType::Replace);
		//publish rpt
		oms.OnMsg(rpt);
		auto exe = _src.execRpt();
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);
	}
	void reject() {
		PROXY<OrderCancelReject> rpt;
		//publish rjt
		oms.OnMsg(rpt);
		auto rjt = _src.cxlRjt();
	}
	void fill(double qty) {
		OrderReq::fill(qty);
	}
	void done() {
		OrderReq::done();
	}
	void canceled() {
		OrderReq::canceled();
	}
};
/////////////////////////////////////////////////////////////////////////
//
//
struct CancelReq : public IRequest {
	PROXY<OrderCancelRequest> ocr;
	CancelReq(const OrderReq& or) 
		: IRequest(or.oms, or._src, or._tgt) {
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
		rpt.execId(_tgt.rpt_seq_no());
		rpt.execType(ExecType::Pending_Cancel);
		rpt.ordStatus(OrdStatus::Pending_Cancel);
		//publish rpt
		oms.OnMsg(rpt);
		auto exe = _src.execRpt();
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);
	}
	void accept() {
		PROXY<ExecutionReport> rpt;
		rpt.execId(_tgt.rpt_seq_no());
		rpt.execType(ExecType::Canceled);
		rpt.leavesQty(0);
		//publish rpt
		oms.OnMsg(rpt);
		auto exe = _src.execRpt();
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);
	}
	void reject() {
		PROXY<OrderCancelReject> rpt;
		//publish rjt
		oms.OnMsg(rpt);
		auto rjt = _src.cxlRjt();
	}
	void status() {

	}
};

