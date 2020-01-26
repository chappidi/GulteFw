#include "plasma.h"
#include "Util.h"
#include <cassert>
namespace plasma
{
	//////////////////////////////////////////////////////////////////////
	// lookup based on orderId or clOrdId. preferred orderId for low latency
	Order* OMS::lookup(uint32_t orderId, uint32_t clOrdId) {
		// step 1: try to lookup based on orderId
		if (orderId != 0) {
			// check range & match clOrderId
			if (_orders.size() >= orderId && _orders[orderId]->_srcOrdId == clOrdId) {
				return _orders[orderId];
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
	void OMS::OnMsg(const NewOrderSingle& req) 
	{
		ClientId cltId(req.clOrdId());
		XYZ& xyz = _out_os[cltId.instance()];
		try
		{
			Order* sts = lookup(0, req.clOrdId());
			if (sts != nullptr) {
				// https://www.onixs.biz/fix-dictionary/4.4/app_dF.1.b.html
				Wrap<ExecutionReport> rpt;
				rpt << *sts;
//				rpt.execType((PossResend == true) ? ExecType::Order_Status : ExecType::Rejected);
				rpt.execType(ExecType::Rejected);
				xyz._cb->OnMsg(rpt);
				return;
			}
			// get the plasma id.
			auto pid = _orders.size();
			// Create Order. store it in _orders
			_orders.insert(_orders.end(), new Order(pid, req));
			xyz._clnt2oms[req.clOrdId()] = pid;
			// step: publish new request to target
			//TODO: do we need to make another copy. or just change the clOrdId and send it out
			Wrap<NewOrderSingle> nos(req);
			nos.clOrdId(pid);
			_out_os[req.target()]._cb->OnMsg(nos);
		}
		catch (exception & ex) {

		}
	}
	//////////////////////////////////////////////////////////////////////
	//			Requesting a status of a order.
	//			you could be asking for a status of a replaced orig order.
	//			need to return the status of the last replacement
	void OMS::OnMsg(const OrderStatusRequest& req) 
	{
		ClientId cltId(req.clOrdId());
		XYZ& xyz = _out_os[cltId.instance()];
		try {
			// get local status either by orderId or clOrdId
			Order* sts = lookup(req.orderId(), req.clOrdId());
			if (sts == nullptr) {
				//https://www.onixs.biz/fix-dictionary/4.4/app_dG.1.a.html
				Wrap<ExecutionReport> rpt;
				rpt << req;
				xyz._cb->OnMsg(rpt);
				return;
			}
			// we found one publish status.
			// step: send the request out with correct sts object. 
			// Do we need to do this ???
			if (auto tgt = _out_os.find((*sts).target()); tgt != _out_os.end()) {
				Wrap<OrderStatusRequest> osr(req);
				osr.clOrdId(sts->_plsOrdId);
				osr.orderId(sts->_dstOrdId);

				tgt->second._cb->OnMsg(osr);
			}
			// step: publish the local status
			// check if replaced find the last one.
			while (sts->_rplOrdId != 0) {
				sts = _orders[sts->_rplOrdId];
			}
			Wrap<ExecutionReport> rpt;
			rpt << *sts;
			// if the order is replaced. then set the origClOrdId
			if (sts->_srcOrdId != req.clOrdId())
				rpt.origClOrdId(req.clOrdId());
			xyz._cb->OnMsg(rpt);

		}
		catch (exception & ex) {

		}
	}
	//////////////////////////////////////////////////////////////////////
	//		 - possible scenario unknown order (sts == nullptr)
	//			Need to return OrderCancelReject (status = Rejected)
	//		 - If order is found create a new Order
	//			and forward the request out with new ids
	void OMS::OnMsg(const OrderCancelRequest& req) 
	{
		ClientId cltId(req.clOrdId());
		XYZ& xyz = _out_os[cltId.instance()];
		try
		{
			Order* orig = lookup(req.orderId(), req.origClOrdId());
			if (orig == nullptr) {
				Wrap<OrderCancelReject> rjt;
				rjt << req;
				xyz._cb->OnMsg(rjt);
				return;
			}
			// get the id.
			auto id = _orders.size();
			// Create Order. store it in _orders
			//??? what is the status = NA ???
			// I think it should be Pending_Cancel.  if a OrderStatusRequest comes what will be the status?
			_orders.insert(_orders.end(), new Order(id, req, *orig));
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
	void OMS::OnMsg(const OrderReplaceRequest& req) {
		stringstream strm;
		strm << "PLS:\t\tORR[" << ClientId(req.clOrdId()) << "/" << ClientId(req.origClOrdId()) << "/" << req.orderId() << "]";
		std::cout << strm.str() << std::endl;

		ClientId cltId(req.clOrdId());
		XYZ& xyz = _out_os[cltId.instance()];
		try
		{
			Order* orig = lookup(req.orderId(), req.origClOrdId());
			if (orig == nullptr) {
				Wrap<OrderCancelReject> rjt;
				rjt << req;
				xyz._cb->OnMsg(rjt);
				return;
			}
			assert(orig->_side == req.side() && orig->_symbol == req.symbol());

			// get the id.
			auto id = _orders.size();
			// Create Order. store it in _orders
			//??? what is the status = NA
			_orders.insert(_orders.end(), new Order(id, req, *orig));
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
	void OMS::OnMsg(const ExecutionReport& rpt) 
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
			Order* orig = (rpt.origClOrdId() != 0) ? _orders[rpt.origClOrdId()] : nullptr;
			Order* nxt = _orders[rpt.clOrdId()];
			// step: validate
			assert(nxt != nullptr);
			assert(nxt->_symbol == rpt.symbol() && nxt->_side == rpt.side());
			assert((orig == nullptr) || (orig->_symbol == nxt->_symbol && orig->_side == nxt->_side));

			// TODO: update state
			if (orig != nullptr) {
				(*orig).Update(rpt);
			}
			// special case
/*
			if (rpt.action() == ExecType::Replace) {
				(*orig)._status = OrdStatus::Replaced;
				(*orig)._rplOrdId = (*nxt)._plsOrdId;
			}
*/
			// when action = replace or cancel. orig != nullptr. 
			// so next in chain has to be kept updated.
			(*nxt).Update(rpt);
			// send rpt out with updated status
			ClientId clt(nxt->_srcOrdId);
			if (auto itr = _out_os.find(clt.instance()); itr != _out_os.end())
			{
				Wrap<ExecutionReport> exe(rpt);
				exe.origClOrdId((orig != nullptr) ? orig->_srcOrdId : 0);
				exe.clOrdId(nxt->_srcOrdId);
				exe.orderId((orig != nullptr) ? orig->_plsOrdId : nxt->_plsOrdId);
				itr->second._cb->OnMsg(exe);
			}
		}
		catch (exception & ex) {

		}

	}
	//////////////////////////////////////////////////////////////////////
	//
	void OMS::OnMsg(const OrderCancelReject& rpt) 
	{
		try
		{
			if (rpt.clOrdId() == 0 || rpt.clOrdId() > _orders.size()
				|| rpt.origClOrdId() == 0 || rpt.origClOrdId() > _orders.size()) {
				// unable to identify which Cancel/Replace is rejected
				// TODO: log error or throw  exception
				return;
			}
			Order* orig = _orders[rpt.origClOrdId()];
			Order* sts = _orders[rpt.clOrdId()];
			// step: validate
			assert(orig != nullptr && sts != nullptr);
			assert(sts->_symbol == orig->_symbol && sts->_side == orig->_side);
			assert(sts->_symbol == rpt.symbol() && sts->_side == rpt.side());
			// step: update status of both reqs
			sts->_status = OrdStatus::Rejected;
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
	void OMS::OnMsg(const DontKnowTrade& rpt) {
		stringstream strm;
		strm << "PLS:\t\tDKT[" << rpt.orderId() << "/" << rpt.execId() << "]";
		std::cout << strm.str() << std::endl;
	}
	//////////////////////////////////////////////////////////////////////
	//
	void OMS::OnLogin(ICallback& cb) {
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
	void OMS::OnLogout(ICallback& cb) {
		// check status of all the orders
	// send logout.
		_out_os.erase(cb.id());
	}
}
