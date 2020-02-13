#pragma once
#include "ICallback.h"
#include "messages.h"
#include <Util.h>
#include <UtilX.h>

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
};
/////////////////////////////////////////////////////////////////////////
// forward declarations
class CancelReq;
class ReplaceReq;
class NewOrderReq;
/////////////////////////////////////////////////////////////////////////
// common functionality of NewOrder  & Replace Order
struct OrderReq : public IRequest 
{
	OrderReq(plasma::ICallback& pls, ISource& sr, ITarget& tg)
		: IRequest(pls, sr, tg) {
	}
	CancelReq	cancel_order();
	ReplaceReq	replace_order(double qty);
	NewOrderReq slice_order(ITarget& tgt, double qty);

	void fill(double qty, double px = 99.98) {
		oms.OnMsg(tgt.sink().get_fill(osr.orderId(), qty, 99.98));
	}
	void done() {
		oms.OnMsg(tgt.sink().get_done(osr.orderId()));
	}
	void canceled() {
		oms.OnMsg(tgt.sink().get_cxld(osr.orderId()));
	}
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
	// constructor ( used for slice order)
	NewOrderReq(OrderReq& prnt, ITarget& tg, double qty)
		: OrderReq(prnt.oms, dynamic_cast<ISource&>(prnt.tgt), tg), _prnt(&prnt), nos(create(qty, &prnt))
	{
		osr << nos;
		//publish nos
		oms.OnMsg(nos);
		osr.orderId(tgt.clOrdId());
		std::cout << "[" << ClientId(nos.clOrdId()) << "-->" << osr.orderId() << "]" << std::endl;
	}
public:
	/////////////////////////////////////////////////////////////////////////
	// constructor ( used for new order)
	NewOrderReq(plasma::ICallback& pls, ISource& sr, ITarget& tg, double qty)
		: OrderReq(pls, sr, tg), nos(create(qty, _prnt))
	{
		osr << nos;
		//publish
		oms.OnMsg(nos);
		// store the plasma id
		osr.orderId(tgt.clOrdId());
		std::cout << "[" << ClientId(nos.clOrdId()) << "-->" << osr.orderId() << "]" << std::endl;
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	// order = OrdStatus::Pending_New
	void pending() {
		oms.OnMsg(tgt.sink().get_pnd_new(osr.orderId()));
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	// order = OrdStatus::New
	void accept() {
		oms.OnMsg(tgt.sink().get_new(osr.orderId()));
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	// order = OrdStatus::Rejected
	void reject() {
		oms.OnMsg(tgt.sink().get_rjt(osr.orderId()));
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	// Resend order again
	void resend() {
		oms.OnMsg(nos);
	}
	friend struct OrderReq;
};
/////////////////////////////////////////////////////////////////////////
//
//
class CancelReq : public IRequest {
	// orig order which needs to be canceled
	const PROXY<OrderStatusRequest>& orig;
	// immutable cancel request. used for resend
	const PROXY<OrderCancelRequest> ocr;
	/////////////////////////////////////////////////////////////////////////
	// utility function
	auto create(const OrderStatusRequest& orig) {
		PROXY<OrderCancelRequest> req;
		// fill in the details
		req.clOrdId(src.req_seq_no())
			.origClOrdId(orig.clOrdId())
			.symbol(orig.symbol())
			.side(orig.side())
			.qty(orig.qty())
			.orderId(orig.orderId());
		return req;
	}
	/////////////////////////////////////////////////////////////////////////
	// constructor (orig request which needs to be canceled)
	CancelReq(OrderReq& org)
		: IRequest(org.oms, org.src, org.tgt), orig(org.osr), ocr(create(org.osr))
	{
		osr << ocr;
		//publish
		oms.OnMsg(ocr);
		// store the plasma id
		osr.orderId(tgt.clOrdId());
		std::cout << "[" << ClientId(ocr.clOrdId()) << "-->" << osr.orderId() << "]" << std::endl;
	}
public:
	void pending() {
		oms.OnMsg(tgt.sink().get_pnd_cxl(osr.orderId(), orig.orderId()));
	}
	void accept() {
		oms.OnMsg(tgt.sink().get_cxld(osr.orderId(), orig.orderId()));
	}
	void reject() { reject(""); }
	void reject(const std::string& reason) {
		oms.OnMsg(tgt.sink().get_rjt(osr.orderId(), orig.orderId(), reason));
	}
	void resend() {
		oms.OnMsg(ocr);
	}
	friend struct OrderReq;
};
/////////////////////////////////////////////////////////////////////////
//
//
class ReplaceReq : public OrderReq {
	// orig order which needs to be replaced
	const PROXY<OrderStatusRequest>& orig;
	// immutable replace request. used for resend
	const PROXY<OrderReplaceRequest> orr;
	/////////////////////////////////////////////////////////////////////////
	// utility function
	PROXY<OrderReplaceRequest> create(const OrderStatusRequest& orig, double_t qty) {
		PROXY<OrderReplaceRequest> req;
		// fill in the details
		req.clOrdId(src.req_seq_no())
			.origClOrdId(orig.clOrdId())
			.symbol(orig.symbol())
			.side(orig.side())
			.qty(qty);
		return req;
	}
	/////////////////////////////////////////////////////////////////////////
	// constructor ( orig request which needs to be replaced and new qty)
	ReplaceReq(OrderReq& org, double qty)
		: OrderReq(org.oms, org.src, org.tgt), orig(org.osr), orr(create(org.osr, qty)) {
		osr << orr;
		// publish
		oms.OnMsg(orr);
		// store the plasma id
		osr.orderId(tgt.clOrdId());
		std::cout << "[" << ClientId(orr.clOrdId()) << "-->" << osr.orderId() << "]" << std::endl;
	}
public:
	void pending() {
		oms.OnMsg(tgt.sink().get_pnd_rpl(osr.orderId(), orig.orderId()));
	}
	void accept() {
		oms.OnMsg(tgt.sink().get_rpld(osr.orderId(), orig.orderId()));
	}
	void reject() { reject(""); }
	void reject(const std::string& reason) {
		oms.OnMsg(tgt.sink().get_rjt(osr.orderId(), orig.orderId(), reason));
	}
	void resend() {
		oms.OnMsg(orr);
	}
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
