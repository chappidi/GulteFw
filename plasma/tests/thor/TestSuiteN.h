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
	uint32_t oms_id;
	OrderReq(PLASMA& pls, ISource& src, ITarget& tgt, double qty, uint32_t prntid = 0)
		: oms(pls), _src(src), _tgt(tgt) {
		// fill in nos
		nos.parent(prntid);
		nos.clOrdId(_src.seq_no());
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
		oms_id = _tgt.clOrdId();
		std::cout << "[" << ClientId(nos.clOrdId()) << "-->" << oms_id << "]" << std::endl;
		check();
	}
	void pending() {

		check();
	}
	void accept() {
		check();
	}
	void reject() {
		check();
	}
	void fill(double qty) {
		check();
	}
	void done() {
		check();
	}
	void canceled() {
		check();
	}
	virtual void check() const {
		// publish order status request
		oms.OnMsg(osr);
		// wait & check the response
		auto exe = _src.execRpt();
		assert(exe.execType() == ExecType::Order_Status && exe.ordStatus() == OrdStatus::NA);
		assert(exe.origClOrdId() == 0 && exe.clOrdId() == nos.clOrdId() && exe.orderId() == oms_id);
		assert(exe.leavesQty() == nos.qty() && exe.cumQty() == 0 && exe.avgPx() == 0);
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);
	}
};
/////////////////////////////////////////////////////////////////////////
//
//
template<typename PLASMA>
struct ChildOrderReq : public OrderReq<PLASMA> {
	const OrderReq& _prnt;
	ChildOrderReq(const OrderReq& prnt, ITarget& tgt, double qty)
		:OrderReq(prnt.oms, dynamic_cast<ISource&>(prnt._tgt), tgt, qty, prnt.oms_id),
		_prnt(prnt) {
		// blank
	}
	void check() const {
		OrderReq::check();
		_prnt.check();
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

