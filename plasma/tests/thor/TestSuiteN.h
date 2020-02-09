#pragma once
#include <TestSuite.h>
#include <messages.h>
/////////////////////////////////////////////////////////////////////////
//
//
template<typename PLASMA>
struct OrderReq {
	PLASMA& oms;
	ISource& _src;
	ITarget& _tgt;
	PROXY<NewOrderSingle> nos;
	PROXY<OrderStatusRequest> osr;
	EOrder* order;
	OrderReq(PLASMA& pls, ISource& src, ITarget& tgt, double qty, uint32_t prntid = 0)
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
	void pending() {

		PROXY<ExecutionReport> rpt;
		rpt << *order;
		rpt.execId(_tgt.rpt_seq_no());
		rpt.ordStatus(OrdStatus::Pending_New);
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

		PROXY<ExecutionReport> rpt;
		rpt << *order;
		rpt.execId(_tgt.rpt_seq_no());
		rpt.ordStatus(OrdStatus::New);
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
		rpt.ordStatus(OrdStatus::Done_For_Day);
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
		rpt.ordStatus(OrdStatus::Canceled);
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
template<typename PLASMA>
struct ChildOrderReq : public OrderReq<PLASMA> {
	const OrderReq& _prnt;
	ChildOrderReq(const OrderReq<PLASMA>& prnt, ITarget& tgt, double qty)
		:OrderReq(prnt.oms, dynamic_cast<ISource&>(prnt._tgt), tgt, qty, prnt.order->_clOrdId),
		_prnt(prnt) {
		// blank
	}
	void check(OrdStatus::Value ordSts, double lastQty, double lastPx) const {
		OrderReq::check(OrdStatus::Value::NA, 0, 0);
		_prnt.check(OrdStatus::Value::NA, 0, 0);
	}
};
/////////////////////////////////////////////////////////////////////////
//
//
struct ReplaceReq {
	void pending() {
	}
	void accept() {
	}
	void reject() {
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
struct CancelReq {
	void pending() {
	}
	void accept() {
	}
	void reject() {
	}
};

