#pragma once
#include "ICallback.h"
#include "messages.h"

/////////////////////////////////////////////////////////////////////////
// interface to define the functionality
//
struct IRequest {
	const double AVGPX_EPSILON = 0.0001;
	// OMS handle the messages
	plasma::ICallback&	oms;
	// request generator
	ISource&			src;
	// report generator
	ITarget&			tgt;
	// expected status for comparison
	PROXY<ExecutionReport>		sts;

	/////////////////////////////////////////////////////////////////////////
	// constructor
	IRequest(plasma::ICallback& pls, ISource& sr, ITarget& tg)
		: oms(pls), src(sr), tgt(tg) {
		// empty
	}
	// basic functions on a request
	virtual void pending() = 0;
	virtual void accept() = 0;
	virtual void reject() = 0;
	virtual void resend() = 0;
	virtual void status() = 0;

	/////////////////////////////////////////////////////////////////////////
	// validate the  duplicate req response
	virtual void check_resend() {
		// no need to set any values. it should match the existing status
		auto exe = src.execRpt(sts.clOrdId());
		assert(exe.execType() == ExecType::Rejected && exe.ordStatus() == sts.ordStatus());
		assert(exe.origClOrdId() == 0 && exe.clOrdId() == sts.clOrdId() && exe.orderId() == sts.orderId());
		assert(exe.leavesQty() == sts.leavesQty() && exe.cumQty() == sts.cumQty() && exe.avgPx() == sts.avgPx());
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);
	}
};

/////////////////////////////////////////////////////////////////////////
// forward declarations
class CancelReq;
class ReplaceReq;
class NewOrderReq;

/////////////////////////////////////////////////////////////////////////
// common functionality of NewOrder  & Replace Order
//
struct OrderReq : public IRequest
{
	map<uint32_t, OrdStatus::Value> osv;
	// alternate representation of request. used to store plasma id
	PROXY<OrderStatusRequest>	osr;
public:
	/////////////////////////////////////////////////////////////////////////
	// constructor forward arguments
	OrderReq(plasma::ICallback& pls, ISource& sr, ITarget& tg) : IRequest(pls, sr, tg){ }

	virtual void update(const ExecutionReport& rpt) {
		if (sts.origClOrdId() == 0)
			sts.origClOrdId(sts.clOrdId());

		sts.ordStatus(rpt.ordStatus());
		sts.clOrdId(rpt.clOrdId());
		sts.orderId(rpt.orderId());
		sts.qty(rpt.qty());
		sts.leavesQty(rpt.leavesQty());
		sts.cumQty(rpt.cumQty());
		sts.avgPx(rpt.avgPx());
	}
	/////////////////////////////////////////////////////////////////////////
	// request generators
	CancelReq	cancel_order();
	ReplaceReq	replace_order(double qty);
	NewOrderReq slice_order(ITarget& tgt, double qty, OrdStatus::Value sts = OrdStatus::NA);
	void status() { check_status(); }
	////////////////////////////////////////////////////////////////////////////////////////////
	// send a OrderStatusRequest and validate the received message
	// validates the recovery process
	virtual void check_status() {
		// if there are any cxl/rpl requests pending, then adjust the status
		// sts.ordStatus == Canceled. then final
		auto ordS = (osv.empty() || sts.ordStatus() == OrdStatus::Canceled) ? sts.ordStatus() : osv.begin()->second;
		// query and receive the response
		oms.OnMsg(osr);	
		auto exe = src.execRpt(sts.clOrdId());
		// validate the response
		assert(exe.execType() == ExecType::Order_Status && exe.ordStatus() == ordS);
		assert(exe.origClOrdId() == sts.origClOrdId() && exe.clOrdId() == sts.clOrdId() && exe.orderId() == sts.orderId());
		assert(exe.leavesQty() == sts.leavesQty() && exe.cumQty() == sts.cumQty());
		assert(abs(exe.avgPx() - sts.avgPx()) < AVGPX_EPSILON);
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);
	}
#pragma region actions
	////////////////////////////////////////////////////////////////////////////////////////////
	// order = OrdStatus::Fill or Partial_Fill or Pending_Replace or Pending_Cancel
	void fill(double qty, double px = 99.98) {
		oms.OnMsg(tgt.sink().get_fill(osr.orderId(), qty, 99.98));
		check_fill(qty, px);
		check_status();
	}
	// verify the fill response
	virtual void check_fill(double qty, double px) {
		// update expected status. request on status will compare against it
		sts.cumQty(sts.cumQty() + qty);
		sts.leavesQty(sts.leavesQty() - qty);
		sts.avgPx(99.98);
		sts.ordStatus((sts.leavesQty() == 0) ? OrdStatus::Filled : OrdStatus::Partially_Filled);

		// if there are any cxl/rpl requests pending, then its status is reported
		auto ordS = osv.empty() ? sts.ordStatus() : osv.begin()->second;
		auto exe = src.execRpt(sts.clOrdId());

		assert(exe.execType() == ExecType::Trade && exe.ordStatus() == ordS);
		assert(exe.origClOrdId() == 0 && exe.clOrdId() == sts.clOrdId() && exe.orderId() == sts.orderId());
		assert(exe.leavesQty() == sts.leavesQty() && exe.cumQty() == sts.cumQty());
		assert(abs(exe.avgPx() - sts.avgPx()) < AVGPX_EPSILON);
		assert(exe.lastQty() == qty && exe.lastPx() == px);
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	// order = OrdStatus::Done_For_Day
	void done() {
		oms.OnMsg(tgt.sink().get_done(osr.orderId()));
		check_done();
		check_status();
	}
	void check_done() {
		// update expected status. request on status will compare against it
		sts.ordStatus(OrdStatus::Done_For_Day);
		sts.leavesQty(0);

		auto exe = src.execRpt(sts.clOrdId());
		assert(exe.execType() == ExecType::Done_For_Day && exe.ordStatus() == OrdStatus::Done_For_Day);
		assert(exe.origClOrdId() == 0 && exe.clOrdId() == sts.clOrdId() && exe.orderId() == sts.orderId());
		assert(exe.leavesQty() == 0 && exe.cumQty() == sts.cumQty());
		assert(abs(exe.avgPx() - sts.avgPx()) < AVGPX_EPSILON);
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	// order = OrdStatus::Canceled
	void canceled() {
		oms.OnMsg(tgt.sink().get_cxld(osr.orderId()));
		check_canceled();
		check_status();
	}
	void check_canceled() {
		// update expected status. request on status will compare against it
		sts.ordStatus(OrdStatus::Canceled);
		sts.leavesQty(0);

		auto exe = src.execRpt(sts.clOrdId());
		assert(exe.execType() == ExecType::Canceled && exe.ordStatus() == OrdStatus::Canceled);
		assert(exe.origClOrdId() == 0 && exe.clOrdId() == sts.clOrdId() && exe.orderId() == sts.orderId());
		assert(exe.leavesQty() == 0 && exe.cumQty() == sts.cumQty());
		assert(abs(exe.avgPx() - sts.avgPx()) < AVGPX_EPSILON);
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);
	}
#pragma endregion
};

/////////////////////////////////////////////////////////////////////////
//
//
class NewOrderReq : public OrderReq 
{
	// parent order to represent if slice
	OrderReq* _prnt{ nullptr };
	// immutable new order single. used for resend
	const PROXY<NewOrderSingle> nos;
	/////////////////////////////////////////////////////////////////////////
	// utility function
	auto create(double_t qty, OrderReq* prnt) {

		return src.get_nos(tgt.id(), qty, prnt != nullptr ? prnt->osr.orderId() : 0);
		PROXY<NewOrderSingle> req;
		// fill in the details
		req.parent(prnt != nullptr ? prnt->osr.orderId() : 0)
			.clOrdId(src.req_seq_no())
			.target(tgt.id())
			.symbol(9999)
			.side(plasma::client::Side::BUY)
			.qty(qty);

		return req;
	}
	/////////////////////////////////////////////////////////////////////////
	// publish request and fill in osr and expected sts 
	void check_sent() {
		std::cout << "[" << ClientId(nos.clOrdId()) << "-->" << tgt.clOrdId() << "]" << std::endl;
		// fill in osr
		osr.clOrdId(nos.clOrdId()).orderId(tgt.clOrdId())
			.symbol(nos.symbol()).side(nos.side()).qty(nos.qty());
		// fill in sts
		sts.origClOrdId(0).clOrdId(nos.clOrdId()).orderId(tgt.clOrdId())
			.symbol(nos.symbol()).side(nos.side()).qty(nos.qty())
			.ordStatus(OrdStatus::NA).cumQty(0).leavesQty(nos.qty())
			.avgPx(0).lastQty(0).lastPx(0);
	}
	/////////////////////////////////////////////////////////////////////////
	// constructor ( used for slice order)
	NewOrderReq(OrderReq& prnt, ITarget& tg, double qty, OrdStatus::Value expSts)
		: OrderReq(prnt.oms, dynamic_cast<ISource&>(prnt.tgt), tg), _prnt(&prnt), nos(create(qty, &prnt))
	{
		oms.OnMsg(nos);
		check_sent();
		if (expSts == OrdStatus::Rejected) {
			sts.ordStatus(OrdStatus::Rejected);
			sts.leavesQty(0);
		}
		check_status();
	}
public:
	/////////////////////////////////////////////////////////////////////////
	// constructor ( used for new order)
	NewOrderReq(plasma::ICallback& pls, ISource& sr, ITarget& tg, double qty)
		: OrderReq(pls, sr, tg), nos(create(qty, _prnt)) 
	{
		oms.OnMsg(nos);
		check_sent();
		check_status();
	}
#pragma region actions
	////////////////////////////////////////////////////////////////////////////////////////////
	// order = OrdStatus::Pending_New
	void pending() {
		oms.OnMsg(tgt.sink().get_pnd_new(osr.orderId()));
		check_pending();
		check_status();
	}
	void check_pending() {
		// need to set. because resend will compare against it
		sts.ordStatus(OrdStatus::Pending_New);

		auto exe = src.execRpt(sts.clOrdId());
		assert(exe.execType() == ExecType::Pending_New && exe.ordStatus() == OrdStatus::Pending_New);
		assert(exe.origClOrdId() == 0 && exe.clOrdId() == sts.clOrdId() && exe.orderId() == sts.orderId());
		assert(exe.leavesQty() == sts.qty() && exe.cumQty() == 0 && exe.avgPx() == 0);
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	// order = OrdStatus::New
	void accept() {
		oms.OnMsg(tgt.sink().get_new(osr.orderId()));
		check_accept();
		check_status();
	}
	void check_accept() {
		// need to set. because resend will compare against it
		sts.ordStatus(OrdStatus::New);
		auto ordS = osv.empty() ? sts.ordStatus() : osv.begin()->second;

		auto exe = src.execRpt(sts.clOrdId());
		assert(exe.execType() == ExecType::New && exe.ordStatus() == ordS);
		assert(exe.origClOrdId() == 0 && exe.clOrdId() == sts.clOrdId() && exe.orderId() == sts.orderId());
		assert(exe.leavesQty() == sts.qty() && exe.cumQty() == 0 && exe.avgPx() == 0);
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	// order = OrdStatus::Rejected
	void reject() {
		oms.OnMsg(tgt.sink().get_rjt(osr.orderId()));
		check_reject();
		check_status();
	}
	void check_reject() {
		// need to set. because resend will compare against it
		sts.ordStatus(OrdStatus::Rejected);
		sts.leavesQty(0);

		auto exe = src.execRpt(sts.clOrdId());
		assert(exe.execType() == ExecType::Rejected && exe.ordStatus() == OrdStatus::Rejected);
		assert(exe.origClOrdId() == 0 && exe.clOrdId() == sts.clOrdId() && exe.orderId() == sts.orderId());
		assert(exe.leavesQty() == 0 && exe.cumQty() == 0 && exe.avgPx() == 0);
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	// Resend order again
	void resend() {
		oms.OnMsg(nos);
		check_resend();
	}
	void check_status() {
		OrderReq::check_status();
		if (_prnt != nullptr)
			_prnt->check_status();
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	// if it is a sliced order. u need to check if the fill is propagated to parent
	void check_fill(double qty, double px) {
		OrderReq::check_fill(qty, px);
		// TODO: verify if this logic works for sliced order
		if (_prnt != nullptr)
			_prnt->check_fill(qty, px);
	}
#pragma endregion
	friend struct OrderReq;
};

/////////////////////////////////////////////////////////////////////////
// Cancel existing Order and state management validation
//
class CancelReq : public IRequest {
	// orig order which needs to be canceled
	OrderReq& orig;
	// immutable cancel request. used for resend
	const PROXY<OrderCancelRequest> ocr;
	/////////////////////////////////////////////////////////////////////////
	// utility function
	auto create(const ExecutionReport& orig) {
		PROXY<OrderCancelRequest> req;
		// fill in the details
		req.clOrdId(src.req_seq_no()).origClOrdId(orig.clOrdId()).orderId(orig.orderId())
			.symbol(orig.symbol()).side(orig.side()).qty(orig.qty());
		return req;
	}
	/////////////////////////////////////////////////////////////////////////
	// publish request and fill in osr and expected sts 
	void check_sent() {
		std::cout << "[" << ClientId(ocr.clOrdId()) << "-->" << tgt.clOrdId() << "]" << std::endl;
		// fill in osr
//		osr.clOrdId(ocr.clOrdId()).orderId(tgt.clOrdId()).symbol(ocr.symbol()).side(ocr.side()).qty(ocr.qty());
		// fill in sts
		sts.origClOrdId(ocr.origClOrdId()).clOrdId(ocr.clOrdId()).orderId(tgt.clOrdId())
			.symbol(ocr.symbol()).side(ocr.side()).qty(ocr.qty())
			.ordStatus(OrdStatus::NA).cumQty(0).leavesQty(0)
			.avgPx(0).lastQty(0).lastPx(0);
	}
	/////////////////////////////////////////////////////////////////////////
	// constructor (orig request which needs to be canceled)
	CancelReq(OrderReq& org)
		: IRequest(org.oms, org.src, org.tgt), orig(org), ocr(create(org.sts))
	{
		oms.OnMsg(ocr);
		check_sent();
		check_status();
	}
public:
	void status() { check_status(); }
	////////////////////////////////////////////////////////////////////////////////////////////
	// check the status of orig req. 
	// there is no action to be done on cancel req. no need to check status
	void check_status() {
		// orig request status
		orig.check_status();

		// No Need to check status of cancel req.
		return;

		// cancel request status
		//oms.OnMsg(osr);
		//auto exe = src.execRpt(sts.clOrdId());
		//assert(exe.execType() == ExecType::Order_Status && exe.ordStatus() == sts.ordStatus());
		//assert(exe.origClOrdId() == 0 && exe.clOrdId() == sts.clOrdId() && exe.orderId() == sts.orderId());
		//assert(exe.leavesQty() == sts.leavesQty() && exe.cumQty() == sts.cumQty() && exe.avgPx() == sts.avgPx());
		//assert(exe.lastQty() == 0 && exe.lastPx() == 0);
	}
#pragma region actions
	////////////////////////////////////////////////////////////////////////////////////////////
	// order = OrdStatus::Pending_Cancel
	void pending() {
		oms.OnMsg(tgt.sink().get_pnd_cxl(sts.orderId(), orig.sts.orderId()));
		// add to the orig awaiting pending_cancel
		orig.osv[sts.orderId()] = OrdStatus::Pending_Cancel;
		check_pending();
		check_status();
	}
	void check_pending() {
		sts.ordStatus(OrdStatus::Pending_Cancel);

		auto exe = src.execRpt(sts.clOrdId());
		assert(exe.execType() == ExecType::Pending_Cancel && exe.ordStatus() == OrdStatus::Pending_Cancel);
		assert(exe.origClOrdId() == orig.sts.clOrdId() && exe.clOrdId() == ocr.clOrdId() && exe.orderId() == orig.sts.orderId());
		assert(exe.leavesQty() == orig.sts.leavesQty() && exe.cumQty() == orig.sts.cumQty());
		assert(abs(exe.avgPx() - orig.sts.avgPx()) < AVGPX_EPSILON);
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	// order = OrdStatus::Canceled
	void accept() {
		// erase this request from pending
		orig.osv.erase(sts.orderId());
		oms.OnMsg(tgt.sink().get_cxld(sts.orderId(), orig.sts.orderId()));
		check_accept();
		check_status();
	}
	void check_accept() {
		sts.ordStatus(OrdStatus::Canceled);
		orig.sts.ordStatus(OrdStatus::Canceled);
		orig.sts.leavesQty(0);

		auto exe = src.execRpt(sts.clOrdId());
		assert(exe.execType() == ExecType::Canceled && exe.ordStatus() == OrdStatus::Canceled);
		assert(exe.origClOrdId() == orig.sts.clOrdId() && exe.clOrdId() == ocr.clOrdId() && exe.orderId() == orig.sts.orderId());
		assert(exe.leavesQty() == 0 && exe.cumQty() == orig.sts.cumQty());
		assert(abs(exe.avgPx() - orig.sts.avgPx()) < AVGPX_EPSILON);
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	// order = OrdStatus::Rejected
	void reject() { reject(""); }
	void reject(const std::string& reason) {
		// erase this request from pending
		orig.osv.erase(sts.orderId());
		oms.OnMsg(tgt.sink().get_rjt(sts.orderId(), orig.sts.orderId(), reason));
		check_reject();
		check_status();
	}
	void check_reject() {
		sts.ordStatus(OrdStatus::Rejected);
		// if there are any cxl/rpl requests pending, then its status is reported
		auto ordS = orig.osv.empty() ? orig.sts.ordStatus() : orig.osv.begin()->second;

		auto rjt = src.cxlRjt();
		assert(rjt.status() == ordS);
		assert(rjt.origClOrdId() == orig.sts.clOrdId() && rjt.clOrdId() == sts.clOrdId() && rjt.orderId() == orig.sts.orderId());
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	// Resend cancel request again
	void resend() {
		oms.OnMsg(ocr);
		check_resend();
	}
#pragma endregion
	friend struct OrderReq;
};

/////////////////////////////////////////////////////////////////////////
// Replace existing Order and state management validation
//
class ReplaceReq : public OrderReq {
	// orig order which needs to be replaced
	OrderReq& orig;
	// immutable replace request. used for resend
	const PROXY<OrderReplaceRequest> orr;
	/////////////////////////////////////////////////////////////////////////
	// utility function
	PROXY<OrderReplaceRequest> create(const ExecutionReport& orig, double_t qty) {
		PROXY<OrderReplaceRequest> req;
		// fill in the details
		req.clOrdId(src.req_seq_no()).origClOrdId(orig.clOrdId()).orderId(orig.orderId())
			.symbol(orig.symbol()).side(orig.side()).qty(qty);
		return req;
	}
	/////////////////////////////////////////////////////////////////////////
	// publish request and fill in osr and expected sts 
	void check_sent() {
		std::cout << "[" << ClientId(orr.clOrdId()) << "-->" << tgt.clOrdId() << "]" << std::endl;
		// fill in osr
		osr.clOrdId(orr.clOrdId()).orderId(tgt.clOrdId())
			.symbol(orr.symbol()).side(orr.side()).qty(orr.qty());
		// fill in sts
		sts.origClOrdId(orr.origClOrdId()).clOrdId(orr.clOrdId()).orderId(tgt.clOrdId())
			.symbol(orr.symbol()).side(orr.side()).qty(orr.qty())
			.ordStatus(OrdStatus::NA).cumQty(0).leavesQty(orr.qty())
			.avgPx(0).lastQty(0).lastPx(0);
	}
	/////////////////////////////////////////////////////////////////////////
	// constructor ( orig request which needs to be replaced and new qty)
	ReplaceReq(OrderReq& org, double qty, OrdStatus::Value expSts = OrdStatus::NA)
		: OrderReq(org.oms, org.src, org.tgt), orig(org), orr(create(org.sts, qty)) {
		oms.OnMsg(orr);
		check_sent();
		if (expSts == OrdStatus::Rejected) {
			sts.ordStatus(OrdStatus::Rejected);
//			sts.leavesQty(0);
		}
		// just check the orig status. 
		// no need to check the replace status as it is not pending/accepted/rejected yet
		orig.check_status();
	}
public:
	void update(const ExecutionReport& rpt) {
		orig.update(rpt);		
		OrderReq::update(rpt);
	}
	void check_fill(double qty, double px) {
		OrderReq::check_fill(qty,px);
		auto exe = src.execRpt(sts.clOrdId());
		// orig order update
		orig.update(exe);
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	//
	void check_status() {
		// orig request status
		orig.check_status();

		// replace request status
		oms.OnMsg(osr);
		auto exe = src.execRpt(sts.clOrdId());
		assert(exe.execType() == ExecType::Order_Status && exe.ordStatus() == sts.ordStatus());
		assert(exe.origClOrdId() == 0 && exe.clOrdId() == sts.clOrdId() && exe.orderId() == sts.orderId());
		assert(exe.leavesQty() == sts.leavesQty() && exe.cumQty() == sts.cumQty());
		assert(abs(exe.avgPx() - sts.avgPx()) < AVGPX_EPSILON);
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);
	}
#pragma region actions
	////////////////////////////////////////////////////////////////////////////////////////////
	// order = OrdStatus::Pending_Replace
	void pending() {
		oms.OnMsg(tgt.sink().get_pnd_rpl(osr.orderId(), orig.sts.orderId()));
		// add to the orig awaiting Pending_Replace
		orig.osv[osr.orderId()] = OrdStatus::Pending_Replace;
		check_pending();
		// no need to check the rpl ordsts.
		orig.check_status();
	}
	void check_pending() {
		sts.ordStatus(OrdStatus::Pending_Replace);

		auto exe = src.execRpt(sts.clOrdId());
		assert(exe.execType() == ExecType::Pending_Replace && exe.ordStatus() == OrdStatus::Pending_Replace);
		assert(exe.origClOrdId() == orig.sts.clOrdId() && exe.clOrdId() == orr.clOrdId() && exe.orderId() == orig.sts.orderId());
		assert(exe.leavesQty() == orig.sts.leavesQty() && exe.cumQty() == orig.sts.cumQty());
		assert(abs(exe.avgPx() - orig.sts.avgPx()) < AVGPX_EPSILON);
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	// order = replaced. new order now
	void accept() {
		// erase this request from pending
		orig.osv.erase(osr.orderId());
		oms.OnMsg(tgt.sink().get_rpld(osr.orderId(), orig.sts.orderId()));
		check_accept();
		check_status();
	}
	void check_accept() {
		sts.leavesQty(sts.qty() - orig.sts.cumQty());
		sts.cumQty(orig.sts.cumQty());
		sts.avgPx(orig.sts.avgPx());
		sts.ordStatus(orig.sts.ordStatus());

		auto exe = src.execRpt(sts.clOrdId());
		assert(exe.execType() == ExecType::Replace && exe.ordStatus() == orig.sts.ordStatus());
		assert(exe.origClOrdId() == orig.sts.clOrdId() && exe.clOrdId() == orr.clOrdId() && exe.orderId() == sts.orderId());
		assert(exe.leavesQty() == sts.leavesQty() && exe.cumQty() == orig.sts.cumQty());
		assert(abs(exe.avgPx() - orig.sts.avgPx()) < AVGPX_EPSILON);
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);

		// orig order replaced
		orig.update(exe);
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	// order = OrdStatus::Rejected
	void reject() { reject(""); }
	void reject(const std::string& reason) {
		// erase this request from pending
		orig.osv.erase(osr.orderId());
		oms.OnMsg(tgt.sink().get_rjt(osr.orderId(), orig.sts.orderId(), reason));
		check_reject();
		check_status();
	}
	void check_reject() {
		sts.ordStatus(OrdStatus::Rejected);
		sts.leavesQty(0);
		// if there are any cxl/rpl requests pending, then its status is reported
		auto ordS = orig.osv.empty() ? orig.sts.ordStatus() : orig.osv.begin()->second;

		auto rjt = src.cxlRjt();
		assert(rjt.status() == ordS);
		assert(rjt.origClOrdId() == orig.sts.clOrdId() && rjt.clOrdId() == sts.clOrdId() && rjt.orderId() == orig.sts.orderId());
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	// Resend replace request again
	void resend() {
		oms.OnMsg(orr);
		check_resend();
	}
#pragma endregion
	friend struct OrderReq;
};

/////////////////////////////////////////////////////////////////////////
//
inline CancelReq OrderReq::cancel_order() {
	return CancelReq(*this);
}
inline ReplaceReq OrderReq::replace_order(double qty) {
	return ReplaceReq(*this, qty);
}
inline NewOrderReq OrderReq::slice_order(ITarget& tgt, double qty, OrdStatus::Value sts) {
	return NewOrderReq(*this, tgt, qty, sts);
}
