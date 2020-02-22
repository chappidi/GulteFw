#include "Thor.h"
#include "Util.h"
#include <cassert>
namespace plasma
{
	//////////////////////////////////////////////////////////////////////
	// lookup based on orderId or clOrdId. preferred orderId for low latency
	OrderV2* OMS_V2::lookup(uint32_t orderId, uint32_t clOrdId) {
		// step 1: try to lookup based on orderId
		if (orderId != 0) {
			// check range & match clOrderId
			if (_orders.size() >= orderId && _orders[orderId]->_srcOrdId == clOrdId) {
				return _orders[orderId];
			} else {
				int k = 0;
			}
		}
		ClientId cltId(clOrdId);
		XYZ& xyz = _out_os[cltId.instance()];
		// step 2: try to lookup based on clOrdId
		if (auto itr = xyz._clnt2oms.find(clOrdId); itr != xyz._clnt2oms.end()) {
			// check range & match clOrderId
			if (_orders.size() >= itr->second && _orders[itr->second]->_srcOrdId == clOrdId) {
				return _orders[itr->second];
			}
		}
		return nullptr;
	}
	//////////////////////////////////////////////////////////////////////
	// 			New Order Request
	// 			TODO: How to handle PossibleResend=Y flag
	void OMS_V2::OnMsg(const NewOrderSingle& req)
	{
		ClientId cltId(req.clOrdId());
		XYZ& xyz = _out_os[cltId.instance()];
		try
		{
			OrderV2* sts = lookup(0, req.clOrdId());
			if (sts != nullptr) {
				// https://www.onixs.biz/fix-dictionary/4.4/app_dF.1.b.html
				Wrap<ExecutionReport> rpt;
				rpt << *sts;
//				rpt.execType((PossResend == true) ? ExecType::Order_Status : ExecType::Rejected);
				rpt.execType(ExecType::Rejected);
				xyz._cb->OnMsg(rpt);
				return;
			}
			// step : child order check for overslice.
			OrderV2* prnt = nullptr;
			// if the req is child slice
			if (req.parent() != 0) {
				if (_orders.size() >= req.parent()) {
					prnt = _orders[req.parent()];
				}
				// no parent exists or oversliced
				if (prnt == nullptr || prnt->_qty < req.qty() + prnt->_slicedQty) {
					Wrap<ExecutionReport> rpt;
					rpt << req;
					rpt.execType(ExecType::Rejected)
						.ordStatus(OrdStatus::Rejected)
						.leavesQty(0);
					xyz._cb->OnMsg(rpt);
					return;
				}
			}

			// get the plasma id.
			auto id = _orders.size();
			// Create Order. store it in _orders
			_orders.insert(_orders.end(), new OrderV2(id, req));
			xyz._clnt2oms[req.clOrdId()] = id;
			if (prnt != nullptr) {
				// associate child to parent & chain children
				_orders[id]->_next = prnt->_child;
				prnt->_child = id;
				prnt->_slicedQty += req.qty();
			}
			// step: publish new request to target
			//TODO: do we need to make another copy. or just change the clOrdId and send it out
			Wrap<NewOrderSingle> nos(req);
			nos.clOrdId(id);
			_out_os[req.target()]._cb->OnMsg(nos);
		}
		catch (exception & ex) {

		}
	}
	//////////////////////////////////////////////////////////////////////
	//		 - possible scenario unknown order (sts == nullptr)
	//			Need to return OrderCancelReject (status = Rejected)
	//		 - If order is found create a new Order
	//			and forward the request out with new ids
	void OMS_V2::OnMsg(const OrderCancelRequest& req)
	{
		ClientId cltId(req.clOrdId());
		XYZ& xyz = _out_os[cltId.instance()];
		try
		{
			OrderV2* orig = lookup(req.orderId(), req.origClOrdId());
			if (orig == nullptr) {
				Wrap<OrderCancelReject> rjt;
				rjt << req;
				xyz._cb->OnMsg(rjt);
				return;
			}
			// TODO: check if req.clOrdId() is duplicate. what to do ????
			OrderV2* sts = lookup(0, req.clOrdId());
			if (sts != nullptr) {
				assert(sts == nullptr);
				// ???? If the req already exists
				// TODO send a status message. Look at OrderStatusRequest()
				return;
			}
			// get the id.
			auto id = _orders.size();
			// Create Order. store it in _orders
			//??? what is the status = NA ???
			// I think it should be Pending_Cancel.  if a OrderStatusRequest comes what will be the status?
			_orders.insert(_orders.end(), new OrderV2(id, req, *orig));
			xyz._clnt2oms[req.clOrdId()] = id;
			// step: publish new request to target
			Wrap<OrderCancelRequest> ocr(req);
			ocr.clOrdId(id);
			ocr.origClOrdId(orig->_plsOrdId);
			ocr.orderId(orig->_dstOrdId);
			_out_os[(*orig).target()]._cb->OnMsg(ocr);
		}
		catch (exception & ex) {

		}
	}
	//////////////////////////////////////////////////////////////////////
	//
	void OMS_V2::OnMsg(const OrderReplaceRequest& req) {
		stringstream strm;
		strm << "PLS:\t\tORR[" << ClientId(req.clOrdId()) << "/" << ClientId(req.origClOrdId()) << "/" << req.orderId() << "]";
		std::cout << strm.str() << std::endl;

		ClientId cltId(req.clOrdId());
		XYZ& xyz = _out_os[cltId.instance()];
		try
		{
			OrderV2* orig = lookup(req.orderId(), req.origClOrdId());
			if (orig == nullptr) {
				Wrap<OrderCancelReject> rjt;
				rjt << req;
				xyz._cb->OnMsg(rjt);
				return;
			}
			assert(orig->_side == req.side() && orig->_symbol == req.symbol());
			// TODO: check if req.clOrdId() is duplicate. what to do ????
			OrderV2* sts = lookup(0, req.clOrdId());
			if (sts != nullptr) {
				assert(sts == nullptr);
				// ???? If the req already exists
				// TODO send a status message. Look at OrderStatusRequest()
				return;
			}

			// get the id.
			auto id = _orders.size();
			// Create Order. store it in _orders
			//??? what is the status = NA
			_orders.insert(_orders.end(), new OrderV2(id, req, *orig));
			xyz._clnt2oms[req.clOrdId()] = id;
			// step: publish new request to target
			// I think it should be Pending_Replace.  if a OrderStatusRequest comes what will be the status?
			Wrap<OrderReplaceRequest> orr(req);
			orr.clOrdId(id);
			orr.origClOrdId(orig->_plsOrdId);
			orr.orderId(orig->_dstOrdId);
			_out_os[(*orig).target()]._cb->OnMsg(orr);

		}
		catch (exception & ex) {

		}
	}
	//////////////////////////////////////////////////////////////////////
	//
	void OMS_V2::get_status(ExecutionReport& rpt, const OrderV2& req) {
		// Order is a cxl or rpl request which is still in pending state
		if (req._origPlsOrdId != 0 && (req._status == OrdStatus::Pending_Cancel || req._status == OrdStatus::Pending_Replace)) {
			OrderV2& orig = *_orders[req._origPlsOrdId];
			rpt << req;
			// check if the orig order is still is same state 
			if (orig._status == OrdStatus::Pending_Cancel || orig._status == OrdStatus::Pending_Replace) {
				assert(orig._chain == 0);
				rpt.orderId(orig._plsOrdId);
				rpt.origClOrdId(orig._srcOrdId);
			}
			else {
				// or moved on then reject
				rpt.ordStatus(OrdStatus::Rejected);
			}
		} else {
			const OrderV2* sts = &req;
			auto oid = req._plsOrdId;
			// step: publish the local status
			// check if replaced find the last one.
			while (sts->_chain != 0 && sts->_status != OrdStatus::Canceled) {
				sts = _orders[sts->_chain];
			}
			rpt << *sts;
			// if the order is replaced. then set the origClOrdId
			if (rpt.clOrdId() != req._srcOrdId) {
//				rpt.orderId(oid);
				rpt.origClOrdId(req._srcOrdId);
			}
		}
	}
	//////////////////////////////////////////////////////////////////////
	//	Requesting a status of a request.
	//	you could be asking for a status of a replaced orig order.
	//	need to return the status of the last replacement
	void OMS_V2::OnMsg(const OrderStatusRequest& req)
	{
		ClientId cltId(req.clOrdId());
		XYZ& xyz = _out_os[cltId.instance()];
		try {
			// get local status either by orderId or clOrdId
			OrderV2* sts = lookup(req.orderId(), req.clOrdId());
			if (sts == nullptr) {
				//https://www.onixs.biz/fix-dictionary/4.4/app_dG.1.a.html
				Wrap<ExecutionReport> rpt;
				rpt << req;
				xyz._cb->OnMsg(rpt);
				return;
			}
			// step: send the request out with correct sts object. 
			// we found one publish status request.
			// we need to do this only if ordStatus == NA
			if (auto tgt = _out_os.find((*sts).target()); tgt != _out_os.end()) {
				Wrap<OrderStatusRequest> osr(req);
				osr.clOrdId(sts->_plsOrdId);
				osr.orderId(sts->_dstOrdId);
				tgt->second._cb->OnMsg(osr);
			}
			Wrap<ExecutionReport> rpt;
			get_status(rpt, *sts);
			xyz._cb->OnMsg(rpt);
		}
		catch (exception & ex) {

		}
	}
	//////////////////////////////////////////////////////////////////////
	//
	void OMS_V2::process(OrderV2& sts, const ExecutionReport& rpt) {
		assert(!(rpt.execType() == ExecType::Pending_Cancel || rpt.execType() == ExecType::Pending_Replace || rpt.execType() == ExecType::Replace));
		sts.Update(rpt);
		// send rpt pit with updated status
		ClientId clt(sts._srcOrdId);
		if (auto itr = _out_os.find(clt.instance()); itr != _out_os.end())
		{
			Wrap<ExecutionReport> exe(rpt);
			exe.origClOrdId(0);
			exe.clOrdId(sts._srcOrdId);
			exe.orderId(rpt.clOrdId());
			itr->second._cb->OnMsg(exe);
		}
		// propagate the fill to parent
		if (rpt.lastQty() != 0) {
			fill_parent(rpt, sts);
		}
		if (rpt.ordStatus() == OrdStatus::Canceled || rpt.ordStatus() == OrdStatus::Done_For_Day) {
			auto diff = sts._qty - sts._cumQty;
			if (diff != 0) {
				OrderV2* prnt = sts.isChild() ? _orders[sts._parent] : nullptr;
				while (prnt != nullptr) {
					// update parent slicedQty
					prnt->_slicedQty -= diff;
					// send status message out
					ClientId clt(prnt->_srcOrdId);
					if (auto itr = _out_os.find(clt.instance()); itr != _out_os.end()) {
						Wrap<ExecutionReport> exe;
						exe << (*prnt);
						exe.execType(ExecType::Order_Status);
						itr->second._cb->OnMsg(exe);
					}
					prnt = prnt->isChild() ? _orders[prnt->_parent] : nullptr;
				}
			}
		}
	}
	//////////////////////////////////////////////////////////////////////
	//
	void OMS_V2::process(OrderV2& orig, OrderV2& sts, const ExecutionReport& rpt) {
		assert(orig._symbol == sts._symbol && orig._side == sts._side);
		assert(rpt.execType() == ExecType::Pending_Cancel || rpt.execType() == ExecType::Pending_Replace || rpt.execType() == ExecType::Canceled || rpt.execType() == ExecType::Replace);
		// update orig order
		orig.Update(rpt);
		// if Repalace or not unsolicited cancel
		if (rpt.execType() == ExecType::Canceled || rpt.execType() == ExecType::Replace) {
			orig._chain = sts._plsOrdId;
			//TODO: this should be done only when the order is replaced
			sts.Update(rpt);
		}
		else {
			int k = 0;
		}
		// send rpt pit with updated status
		ClientId clt(sts._srcOrdId);
		if (auto itr = _out_os.find(clt.instance()); itr != _out_os.end())
		{
			Wrap<ExecutionReport> exe(rpt);
			exe.origClOrdId(orig._srcOrdId);
			exe.clOrdId(sts._srcOrdId);
			exe.orderId(rpt.execType() == ExecType::Replace ? rpt.clOrdId() : rpt.origClOrdId());
			itr->second._cb->OnMsg(exe);
		}
	}
	//////////////////////////////////////////////////////////////////////
	//
	void OMS_V2::OnMsg(const ExecutionReport& rpt)
	{
		ClientId cltId(rpt.execId());
		ICallback* in = _out_os[cltId.instance()]._cb;
		try {
			// clOrdId of ExecutionReport == order index in _orders
			// check the Order exists
			if (rpt.clOrdId() == 0 || rpt.clOrdId() > _orders.size()
				|| (rpt.origClOrdId() != 0 && rpt.origClOrdId() > _orders.size())) {
				Wrap<DontKnowTrade> dkt;
				dkt << rpt;
				(*in).OnMsg(dkt);
				return;
			}
			OrderV2* orig = (rpt.origClOrdId() != 0) ? _orders[rpt.origClOrdId()] : nullptr;
			OrderV2* nxt = _orders[rpt.clOrdId()];
			// step: validate
			assert(nxt != nullptr);
			assert(nxt->_symbol == rpt.symbol() && nxt->_side == rpt.side());

			if (orig == nullptr)
				process(*nxt, rpt);
			else
				process(*orig, *nxt, rpt);
		}
		catch (exception & ex) {

		}
	}
	//////////////////////////////////////////////////////////////////////
	//
	void OMS_V2::fill_parent(const ExecutionReport& rpt, OrderV2& chld) {
		OrderV2* prnt = chld.isChild() ? _orders[chld._parent] : nullptr;
		while (prnt != nullptr) {
			prnt->fill(rpt.lastQty(), rpt.lastPx());
			// send rpt out with updated status
			ClientId clt(prnt->_srcOrdId);
			if (auto itr = _out_os.find(clt.instance()); itr != _out_os.end())
			{
				Wrap<ExecutionReport> exe;
				exe << (*prnt);
				exe.execType(ExecType::Trade);
				exe.lastPx(rpt.lastPx());
				exe.lastQty(rpt.lastQty());
				itr->second._cb->OnMsg(exe);
			}
			prnt = prnt->isChild() ? _orders[prnt->_parent] : nullptr;
		}
	}
	//////////////////////////////////////////////////////////////////////
	//
	void OMS_V2::OnMsg(const OrderCancelReject& rpt)
	{
		try
		{
			if (rpt.clOrdId() == 0 || rpt.clOrdId() > _orders.size()
				|| rpt.origClOrdId() == 0 || rpt.origClOrdId() > _orders.size()) {
				// unable to identify which Cancel/Replace is rejected
				// TODO: log error or throw  exception
				return;
			}
			OrderV2* orig = _orders[rpt.origClOrdId()];
			OrderV2* sts = _orders[rpt.clOrdId()];
			// step: validate
			assert(orig != nullptr && sts != nullptr);
			assert(sts->_symbol == orig->_symbol && sts->_side == orig->_side);
			assert(sts->_symbol == rpt.symbol() && sts->_side == rpt.side());
			// step: update status of both reqs
			sts->_status = OrdStatus::Rejected;
			sts->_avgPx = sts->_cumQty = sts->_leavesQty = 0;
			orig->_status = rpt.status();
			// step: publish 
			ClientId clt(orig->_srcOrdId);
			if (auto itr = _out_os.find(clt.instance()); itr != _out_os.end())
			{
				Wrap<OrderCancelReject> ocr(rpt);
				ocr.clOrdId(sts->_srcOrdId);
				ocr.origClOrdId(orig->_srcOrdId);
				ocr.orderId(orig->_plsOrdId);
				itr->second._cb->OnMsg(ocr);
			}
		}
		catch (exception & ex) {
			// TODO:
		}

	}
	//////////////////////////////////////////////////////////////////////
	//
	void OMS_V2::OnMsg(const DontKnowTrade& rpt) {
		stringstream strm;
		strm << "PLS:\t\tDKT[" << rpt.orderId() << "/" << rpt.execId() << "]";
		std::cout << strm.str() << std::endl;
	}
	//////////////////////////////////////////////////////////////////////
	//
	void OMS_V2::OnLogin(ICallback& cb) {
		_out_os[cb.id()]._cb = &cb;
		// publish all the orders
		XYZ& xyz = _out_os[cb.id()];
		for (auto const& [clOrdId, ordId] : xyz._clnt2oms) {
			std::cout << ClientId(clOrdId) << "/" << ordId << "/" 
				<< OrdStatus::c_str(_orders[ordId]->_status) << std::endl;
		}
	}
	//////////////////////////////////////////////////////////////////////
	//
	void OMS_V2::OnLogout(ICallback& cb) {
		// check status of all the orders
	// send logout.
		_out_os.erase(cb.id());
	}
}
