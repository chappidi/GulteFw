#pragma once
#include "ICallback.h"
#include "messages.h"

/////////////////////////////////////////////////////////////////////////
// interface to define the functionality
//
struct IRequest {
	// oms which handles the messages
	plasma::ICallback&	oms;
	// sender (request generation)
	ISource&			src;
	// target (report generation)
	ITarget&			tgt;
	// alternate representation of request. used to store plasma id
	PROXY<OrderStatusRequest>	osr;
	// expected sts
	PROXY<ExecutionReport>		sts;

	/////////////////////////////////////////////////////////////////////////
	// constructor
	IRequest(plasma::ICallback& pls, ISource& sr, ITarget& tg)
		: oms(pls), src(sr), tgt(tg) {
	}
	virtual void pending() = 0;
	virtual void accept() = 0;
	virtual void reject() = 0;
	virtual void resend() = 0;
	virtual void status() const { oms.OnMsg(osr); }

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
public:
	OrderReq(plasma::ICallback& pls, ISource& sr, ITarget& tg)
		: IRequest(pls, sr, tg) {
	}
	CancelReq	cancel_order();
	ReplaceReq	replace_order(double qty);
	NewOrderReq slice_order(ITarget& tgt, double qty);
#pragma region actions
	////////////////////////////////////////////////////////////////////////////////////////////
	// order = OrdStatus::Fill or Partial_Fill or Pending_Replace or Pending_Cancel
	void fill(double qty, double px = 99.98) {
		oms.OnMsg(tgt.sink().get_fill(osr.orderId(), qty, 99.98));
		check_fill(qty, px);
	}
	virtual void check_fill(double qty, double px) {
		//update status
		sts.cumQty(sts.cumQty() + qty);
		sts.leavesQty(sts.leavesQty() - qty);
		sts.avgPx(99.98);
		auto ordS = (sts.leavesQty() == 0) ? OrdStatus::Filled : OrdStatus::Partially_Filled;
		sts.ordStatus(ordS);
		// if there are any cxl/rpl requests pending, then its status is reported
		for (const auto& [k, v] : osv) 
			ordS = v;

		auto exe = src.execRpt(sts.clOrdId());
		assert(exe.execType() == ExecType::Trade && exe.ordStatus() == ordS);
		assert(exe.origClOrdId() == 0 && exe.clOrdId() == sts.clOrdId() && exe.orderId() == sts.orderId());
		assert(exe.leavesQty() == sts.leavesQty() && exe.cumQty() == sts.cumQty() && exe.avgPx() == sts.avgPx());
		assert(exe.lastQty() == qty && exe.lastPx() == px);
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	// order = OrdStatus::Done_For_Day
	void done() {
		oms.OnMsg(tgt.sink().get_done(osr.orderId()));
		check_done();
	}
	void check_done() {
		// need to set. because resend will compare against it
		sts.ordStatus(OrdStatus::Done_For_Day);

		auto exe = src.execRpt(sts.clOrdId());
		assert(exe.execType() == ExecType::Done_For_Day && exe.ordStatus() == OrdStatus::Done_For_Day);
		assert(exe.origClOrdId() == 0 && exe.clOrdId() == sts.clOrdId() && exe.orderId() == sts.orderId());
		assert(exe.leavesQty() == 0 && exe.cumQty() == sts.cumQty() && exe.avgPx() == sts.avgPx());
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	// order = OrdStatus::Canceled
	void canceled() {
		oms.OnMsg(tgt.sink().get_cxld(osr.orderId()));
		check_canceled();
	}
	void check_canceled() {
		// need to set. because resend will compare against it
		sts.ordStatus(OrdStatus::Canceled);

		auto exe = src.execRpt(sts.clOrdId());
		assert(exe.execType() == ExecType::Canceled && exe.ordStatus() == OrdStatus::Canceled);
		assert(exe.origClOrdId() == 0 && exe.clOrdId() == sts.clOrdId() && exe.orderId() == sts.orderId());
		assert(exe.leavesQty() == 0 && exe.cumQty() == sts.cumQty() && exe.avgPx() == sts.avgPx());
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
	void send_request() {
		//publish nos
		oms.OnMsg(nos);
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
	NewOrderReq(OrderReq& prnt, ITarget& tg, double qty)
		: OrderReq(prnt.oms, dynamic_cast<ISource&>(prnt.tgt), tg), _prnt(&prnt), nos(create(qty, &prnt))
	{
		send_request();
		std::cout << "[" << ClientId(nos.clOrdId()) << "-->" << osr.orderId() << "]" << std::endl;
	}
public:
	/////////////////////////////////////////////////////////////////////////
	// constructor ( used for new order)
	NewOrderReq(plasma::ICallback& pls, ISource& sr, ITarget& tg, double qty)
		: OrderReq(pls, sr, tg), nos(create(qty, _prnt)) 
	{
		send_request();
	}
#pragma region actions
	////////////////////////////////////////////////////////////////////////////////////////////
	// order = OrdStatus::Pending_New
	void pending() {
		oms.OnMsg(tgt.sink().get_pnd_new(osr.orderId()));
		check_pending();
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
	}
	void check_accept() {
		// need to set. because resend will compare against it
		sts.ordStatus(OrdStatus::New);

		auto exe = src.execRpt(sts.clOrdId());
		assert(exe.execType() == ExecType::New && exe.ordStatus() == OrdStatus::New);
		assert(exe.origClOrdId() == 0 && exe.clOrdId() == sts.clOrdId() && exe.orderId() == sts.orderId());
		assert(exe.leavesQty() == sts.qty() && exe.cumQty() == 0 && exe.avgPx() == 0);
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	// order = OrdStatus::Rejected
	void reject() {
		oms.OnMsg(tgt.sink().get_rjt(osr.orderId()));
		check_reject();
	}
	void check_reject() {
		// need to set. because resend will compare against it
		sts.ordStatus(OrdStatus::Rejected);

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
//
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
	void send_request() {
		//publish
		oms.OnMsg(ocr);
		// fill in osr
		osr.clOrdId(ocr.clOrdId()).orderId(tgt.clOrdId())
			.symbol(ocr.symbol()).side(ocr.side()).qty(ocr.qty());
		// fill in sts
		sts.origClOrdId(ocr.origClOrdId()).clOrdId(ocr.clOrdId()).orderId(tgt.clOrdId())
			.symbol(ocr.symbol()).side(ocr.side()).qty(ocr.qty())
			.ordStatus(OrdStatus::NA).cumQty(0).leavesQty(ocr.qty())
			.avgPx(0).lastQty(0).lastPx(0);
	}
	/////////////////////////////////////////////////////////////////////////
	// constructor (orig request which needs to be canceled)
	CancelReq(OrderReq& org)
		: IRequest(org.oms, org.src, org.tgt), orig(org), ocr(create(org.sts))
	{
		send_request();
		std::cout << "[" << ClientId(ocr.clOrdId()) << "-->" << osr.orderId() << "]" << std::endl;
	}
public:
#pragma region actions
	////////////////////////////////////////////////////////////////////////////////////////////
	// order = OrdStatus::Pending_Cancel
	void pending() {
		oms.OnMsg(tgt.sink().get_pnd_cxl(osr.orderId(), orig.sts.orderId()));
		// add to the orig awaiting pending_cancel
		orig.osv[osr.orderId()] = OrdStatus::Pending_Cancel;
		check_pending();
	}
	void check_pending() {
		sts.ordStatus(OrdStatus::Pending_Cancel);

		auto exe = src.execRpt(sts.clOrdId());
		assert(exe.execType() == ExecType::Pending_Cancel && exe.ordStatus() == OrdStatus::Pending_Cancel);
		assert(exe.origClOrdId() == orig.sts.clOrdId() && exe.clOrdId() == ocr.clOrdId() && exe.orderId() == orig.sts.orderId());
		assert(exe.leavesQty() == orig.sts.leavesQty() && exe.cumQty() == orig.sts.cumQty() && exe.avgPx() == orig.sts.avgPx());
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	// order = OrdStatus::Canceled
	void accept() {
		// erase this request from pending
		orig.osv.erase(osr.orderId());
		oms.OnMsg(tgt.sink().get_cxld(osr.orderId(), orig.sts.orderId()));
		check_accept();
	}
	void check_accept() {
		sts.ordStatus(OrdStatus::Canceled);
		orig.sts.ordStatus(OrdStatus::Canceled);

		auto exe = src.execRpt(sts.clOrdId());
		assert(exe.execType() == ExecType::Canceled && exe.ordStatus() == OrdStatus::Canceled);
		assert(exe.origClOrdId() == orig.sts.clOrdId() && exe.clOrdId() == ocr.clOrdId() && exe.orderId() == orig.sts.orderId());
		assert(exe.leavesQty() == 0 && exe.cumQty() == orig.sts.cumQty() && exe.avgPx() == orig.sts.avgPx());
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	// order = OrdStatus::Rejected
	void reject() { reject(""); }
	void reject(const std::string& reason) {
		// erase this request from pending
		orig.osv.erase(osr.orderId());
		oms.OnMsg(tgt.sink().get_rjt(osr.orderId(), orig.sts.orderId(), reason));
		check_reject();
	}
	void check_reject() {
		sts.ordStatus(OrdStatus::Rejected);
		auto ordS = orig.sts.ordStatus();
		// if there are any cxl/rpl requests pending, then its status is reported
		for (const auto& [k, v] : orig.osv) 
			ordS = v;

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
//
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
	void send_request() {
		// publish
		oms.OnMsg(orr);
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
	ReplaceReq(OrderReq& org, double qty)
		: OrderReq(org.oms, org.src, org.tgt), orig(org), orr(create(org.sts, qty)) {
		send_request();
		std::cout << "[" << ClientId(orr.clOrdId()) << "-->" << osr.orderId() << "]" << std::endl;
	}
public:
#pragma region actions
	////////////////////////////////////////////////////////////////////////////////////////////
	// order = OrdStatus::Pending_Replace
	void pending() {
		oms.OnMsg(tgt.sink().get_pnd_rpl(osr.orderId(), orig.sts.orderId()));
		// add to the orig awaiting Pending_Replace
		orig.osv[osr.orderId()] = OrdStatus::Pending_Replace;
		check_pending();
	}
	void check_pending() {
		sts.ordStatus(OrdStatus::Pending_Replace);

		auto exe = src.execRpt(sts.clOrdId());
		assert(exe.execType() == ExecType::Pending_Replace && exe.ordStatus() == OrdStatus::Pending_Replace);
		assert(exe.origClOrdId() == orig.sts.clOrdId() && exe.clOrdId() == orr.clOrdId() && exe.orderId() == orig.sts.orderId());
		assert(exe.leavesQty() == orig.sts.leavesQty() && exe.cumQty() == orig.sts.cumQty() && exe.avgPx() == orig.sts.avgPx());
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	// order = replaced. new order now
	void accept() {
		// erase this request from pending
		orig.osv.erase(osr.orderId());
		oms.OnMsg(tgt.sink().get_rpld(osr.orderId(), orig.sts.orderId()));
		check_accept();
	}
	void check_accept() {
		sts.leavesQty(sts.qty() - orig.sts.cumQty());
		sts.cumQty(orig.sts.cumQty());
		sts.avgPx(orig.sts.avgPx());

		auto exe = src.execRpt(sts.clOrdId());
		assert(exe.execType() == ExecType::Replace && exe.ordStatus() == orig.sts.ordStatus());
		assert(exe.origClOrdId() == orig.sts.clOrdId() && exe.clOrdId() == orr.clOrdId() && exe.orderId() == sts.orderId());
		assert(exe.leavesQty() == sts.leavesQty() && exe.cumQty() == orig.sts.cumQty() && exe.avgPx() == orig.sts.avgPx());
		assert(exe.lastQty() == 0 && exe.lastPx() == 0);
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	// order = OrdStatus::Rejected
	void reject() { reject(""); }
	void reject(const std::string& reason) {
		// erase this request from pending
		orig.osv.erase(osr.orderId());
		oms.OnMsg(tgt.sink().get_rjt(osr.orderId(), orig.sts.orderId(), reason));
		check_reject();
	}
	void check_reject() {
		sts.ordStatus(OrdStatus::Rejected);
		auto ordS = orig.sts.ordStatus();
		// if there are any cxl/rpl requests pending, then its status is reported
		for (const auto& [k, v] : orig.osv)
			ordS = v;

		auto rjt = src.cxlRjt();
		assert(rjt.status() == orig.sts.ordStatus());
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
inline NewOrderReq OrderReq::slice_order(ITarget& tgt, double qty) {
	return NewOrderReq(*this, tgt, qty);
}
