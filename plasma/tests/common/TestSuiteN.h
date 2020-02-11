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
	uint32_t			idX{ 0 };
	PROXY<ExecutionReport>		sts;
	IRequest(plasma::ICallback& pls, ISource& src, ITarget& tgt)
		: oms(pls), _src(src), _tgt(tgt) {
	}
	virtual PROXY<OrderStatusRequest> get_osr() const = 0;
	virtual void pending() = 0;
	virtual void accept() = 0;
	virtual void reject() = 0;
	//////////////////////////////////////////////////////////////////////
	// lastQty and lastPx is always zero for status
	virtual void status() const {
		// publish order status request
		oms.OnMsg(get_osr());
		// wait & compare to expected value (sts)
		auto exe = _src.execRpt();
		assert(exe.execType() == ExecType::Order_Status && exe.ordStatus() == sts.ordStatus());
		assert(exe.origClOrdId() == sts.origClOrdId() && exe.clOrdId() == sts.clOrdId() && exe.orderId() == sts.orderId());
		assert(exe.leavesQty() == sts.leavesQty() && exe.cumQty() == sts.cumQty() && exe.avgPx() == sts.avgPx());
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);
	}
};
/////////////////////////////////////////////////////////////////////////
//
//
struct CancelReq : public IRequest {
	IRequest& chain;
	PROXY<OrderCancelRequest> ocr;
	auto get_cxl(const OrderStatusRequest& osr)
	{
		PROXY<OrderCancelRequest> req;
		req.clOrdId(_src.req_seq_no());
		req.origClOrdId(osr.clOrdId());
		req.symbol(osr.symbol());
		req.side(osr.side());
		req.qty(osr.qty());
		req.orderId(osr.orderId());

		// fill in status
		sts.clOrdId(req.clOrdId());
		sts.origClOrdId(0);
		sts.symbol(req.symbol());
		sts.side(req.side());
		sts.qty(req.qty());
		sts.ordStatus(OrdStatus::NA)
			.avgPx(0).lastQty(0).lastPx(0);

		return req;
	}
	PROXY<OrderStatusRequest> get_osr() const {
		PROXY<OrderStatusRequest> req;
		req.clOrdId(ocr.clOrdId());
		req.symbol(ocr.symbol());
		req.side(ocr.side());
		req.qty(ocr.qty());
		req.orderId(idX);
		return req;
	}
	CancelReq(IRequest& or)
		: IRequest(or.oms, or._src, or._tgt), chain(or), ocr(get_cxl(or.get_osr())) 
	{
		//publish ocr
		oms.OnMsg(ocr);
		idX = _tgt.clOrdId();
		std::cout << "[" << ClientId(ocr.clOrdId()) << "-->" << idX << "]" << std::endl;

		//query and check
		sts.orderId(idX);
		status();
	}
	void pending() {
		auto& orig = chain.sts;
		//send response & check client report
		oms.OnMsg(_tgt.sink().get_pnd_cxl(idX, orig.orderId()));
		auto exe = _src.execRpt();
		assert(exe.execType() == ExecType::Pending_Cancel && exe.ordStatus() == OrdStatus::Pending_Cancel);
		assert(exe.origClOrdId() == orig.clOrdId() && exe.clOrdId() == ocr.clOrdId() && exe.orderId() == orig.orderId());
		assert(exe.leavesQty() == orig.leavesQty() && exe.cumQty() == orig.cumQty() && exe.avgPx() == orig.avgPx());
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);

		//query and check
		orig.ordStatus(OrdStatus::Value::Pending_Cancel);
		sts.ordStatus(OrdStatus::Value::Pending_Cancel);
		sts.origClOrdId(orig.clOrdId());
		sts.orderId(orig.orderId());
		status();
	}
	void accept() {
		auto& orig = chain.sts;
		//send response & check client report
		oms.OnMsg(_tgt.sink().get_cxld(idX, orig.orderId()));
		auto exe = _src.execRpt();
		assert(exe.execType() == ExecType::Canceled && exe.ordStatus() == OrdStatus::Canceled);
		assert(exe.origClOrdId() == orig.clOrdId() && exe.clOrdId() == ocr.clOrdId() && exe.orderId() == orig.orderId());
		assert(exe.leavesQty() == 0 && exe.cumQty() == orig.cumQty() && exe.avgPx() == orig.avgPx());
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);

		//query and check
		orig.ordStatus(OrdStatus::Value::Canceled);
		sts.ordStatus(OrdStatus::Value::Canceled);
		sts.origClOrdId(orig.clOrdId());
		sts.orderId(orig.orderId());
		status();
	}
	void reject() { reject(""); }
	void reject(const std::string& reason) {
		auto& orig = chain.sts;
		//send response & check client report
		oms.OnMsg(_tgt.sink().get_rjt(idX, orig.orderId(), reason));
		auto rjt = _src.cxlRjt();
		assert(rjt.status() == orig.ordStatus());
		assert(rjt.origClOrdId() == orig.clOrdId() && rjt.clOrdId() == sts.clOrdId() && rjt.orderId() == orig.orderId());

		//query and check
		sts.ordStatus(OrdStatus::Value::Rejected);
		status();
	}
	void status() const {
		chain.status();
		IRequest::status();
	}
};
/////////////////////////////////////////////////////////////////////////
// Need Target to generate exec_rpt id
//
struct OrderReq : public IRequest 
{
	OrderReq(plasma::ICallback& pls, ISource& src, ITarget& tgt)
		: IRequest(pls, src, tgt) {
	}
	auto cancel_order() {
		return CancelReq(*this);
	}
	auto replace_order(double qty) {
		return 0;
	}
	auto slice(ITarget& tgt, double qty) {
		return 0; // ChildOrderReq(*this, tgt, qty);
	}
	void fill(double qty, double px = 99.98) {
		//update status
		sts.cumQty(sts.cumQty() + qty);
		sts.leavesQty(sts.leavesQty() - qty);
		sts.avgPx(99.98);
		auto ordS = (sts.leavesQty() == 0) ? OrdStatus::Filled : OrdStatus::Partially_Filled;
		sts.ordStatus(ordS);

		//send response & check client report
		oms.OnMsg(_tgt.sink().get_fill(idX, qty, 99.98));
		auto exe = _src.execRpt();
		assert(exe.execType() == ExecType::Trade && exe.ordStatus() == sts.ordStatus());
		assert(exe.origClOrdId() == 0 && exe.clOrdId() == sts.clOrdId() && exe.orderId() == idX);
		assert(exe.leavesQty() == sts.leavesQty() && exe.cumQty() == sts.cumQty() && exe.avgPx() == sts.avgPx());
		assert(exe.lastQty() == qty && exe.lastPx() == px);

		//query and check
		status();
	}
	void done() 
	{
		//send response & check client report
		oms.OnMsg(_tgt.sink().get_done(idX));
		auto exe = _src.execRpt();
		assert(exe.execType() == ExecType::Done_For_Day && exe.ordStatus() == OrdStatus::Done_For_Day);
		assert(exe.origClOrdId() == 0 && exe.clOrdId() == sts.clOrdId() && exe.orderId() == idX);
		assert(exe.leavesQty() == 0 && exe.cumQty() == sts.cumQty() && exe.avgPx() == sts.avgPx());
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);

		//query and check
		sts.ordStatus(OrdStatus::Done_For_Day);
		sts.leavesQty(0);
		status();
	}
	void canceled() 
	{
		//send response & check client report
		oms.OnMsg(_tgt.sink().get_cxld(idX));
		auto exe = _src.execRpt();
		assert(exe.execType() == ExecType::Canceled && exe.ordStatus() == OrdStatus::Canceled);
		assert(exe.origClOrdId() == 0 && exe.clOrdId() == sts.clOrdId() && exe.orderId() == idX);
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
struct NewOrderReq : public OrderReq 
{
	// store for resend
	const PROXY<NewOrderSingle> nos;
	// utility to create NewOrderSingle
	auto get_nos(ITarget& tgt, double_t qty) {
		PROXY<NewOrderSingle> req;
		req.clOrdId(_src.req_seq_no());
		req.target(tgt.id());
		req.symbol(9999);
		req.side(plasma::client::Side::BUY);
		req.qty(qty);
		// fill in status
		sts.origClOrdId(0).clOrdId(req.clOrdId())
			.symbol(req.symbol()).side(req.side()).qty(req.qty())
			.ordStatus(OrdStatus::NA)
			.cumQty(0).leavesQty(req.qty())
			.avgPx(0).lastQty(0).lastPx(0);

		return req;
	}
	// utility to create OrderStatusRequest
	PROXY<OrderStatusRequest> get_osr() const {
		PROXY<OrderStatusRequest> req;
		req.clOrdId(nos.clOrdId());
		req.symbol(nos.symbol());
		req.side(nos.side());
		req.qty(nos.qty());
		req.orderId(idX);
		return req;
	}
	// constructor
	NewOrderReq(plasma::ICallback& pls, ISource& src, ITarget& tgt, double qty, uint32_t prntid = 0)
		: OrderReq(pls, src, tgt), nos(get_nos(_tgt, qty))
	{
		//publish nos
		oms.OnMsg(nos);
		idX = _tgt.clOrdId();
		std::cout << "[" << ClientId(nos.clOrdId()) << "-->" << idX << "]" << std::endl;

		//query and check
		sts.orderId(idX);
		status();
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	// order = OrdStatus::Pending_New
	void pending()
	{
		//send response & check client report
		oms.OnMsg(_tgt.sink().get_pnd_new(idX));
		auto exe = _src.execRpt();
		assert(exe.execType() == ExecType::Pending_New && exe.ordStatus() == OrdStatus::Pending_New);
		assert(exe.origClOrdId() == 0 && exe.clOrdId() == nos.clOrdId() && exe.orderId() == idX);
		assert(exe.leavesQty() == nos.qty() && exe.cumQty() == 0 && exe.avgPx() == 0);
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);

		//query and check
		sts.ordStatus(OrdStatus::Pending_New);
		status();
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	// order = OrdStatus::New
	void accept()
	{
		//send response & check client report
		oms.OnMsg(_tgt.sink().get_new(idX));
		auto exe = _src.execRpt();
		assert(exe.execType() == ExecType::New && exe.ordStatus() == OrdStatus::New);
		assert(exe.origClOrdId() == 0 && exe.clOrdId() == nos.clOrdId() && exe.orderId() == idX);
		assert(exe.leavesQty() == nos.qty() && exe.cumQty() == 0 && exe.avgPx() == 0);
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);

		//query and check
		sts.ordStatus(OrdStatus::New);
		status();
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	// order = OrdStatus::Rejected
	void reject()
	{
		//send response & check client report
		oms.OnMsg(_tgt.sink().get_rjt(idX));
		auto exe = _src.execRpt();
		assert(exe.execType() == ExecType::Rejected && exe.ordStatus() == OrdStatus::Rejected);
		assert(exe.origClOrdId() == 0 && exe.clOrdId() == nos.clOrdId() && exe.orderId() == idX);
		assert(exe.leavesQty() == 0 && exe.cumQty() == 0 && exe.avgPx() == 0);
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);

		//query and check
		sts.ordStatus(OrdStatus::Rejected);
		sts.leavesQty(0);
		status();
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	// Resend order again
	void resend() 
	{
		//send request & check client report
		oms.OnMsg(nos);
		auto exe = _src.execRpt();
		assert(exe.execType() == ExecType::Rejected && exe.ordStatus() == sts.ordStatus());
		assert(exe.origClOrdId() == 0 && exe.clOrdId() == nos.clOrdId() && exe.orderId() == idX);
		assert(exe.leavesQty() == sts.leavesQty() && exe.cumQty() == sts.cumQty() && exe.avgPx() == sts.avgPx());
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);

		//query and check
		status();
	}
};
/////////////////////////////////////////////////////////////////////////
//
//
struct ChildOrderReq : public NewOrderReq {
	const NewOrderReq& _prnt;
	ChildOrderReq(const NewOrderReq& prnt, ITarget& tgt, double qty)
		:NewOrderReq(prnt.oms, dynamic_cast<ISource&>(prnt._tgt), tgt, qty, prnt.idX),
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
	PROXY<OrderStatusRequest> get_osr() const
	{
		PROXY<OrderStatusRequest> req;
		req.clOrdId(orr.clOrdId());
		req.symbol(orr.symbol());
		req.side(orr.side());
		req.qty(orr.qty());
		req.orderId(idX);
		return req;
	}
	ReplaceReq(const OrderReq& or, double qty) : OrderReq(or.oms, or._src, or._tgt) {
	}
	void pending() 
	{
		//send response & check client report
		oms.OnMsg(_tgt.sink().get_pnd_rpl(idX, -1));
		auto exe = _src.execRpt();
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);
	}
	void accept() 
	{
		//send response & check client report
		oms.OnMsg(_tgt.sink().get_rpld(idX, -1));
		auto exe = _src.execRpt();
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);
	}
	void reject() { reject(""); }
	void reject(const std::string& reason) {
		//send response & check client report
		oms.OnMsg(_tgt.sink().get_rjt(idX, -1, reason));
		auto rjt = _src.cxlRjt();
	}
};

