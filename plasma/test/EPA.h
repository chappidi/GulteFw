#pragma once
#include "messages.h"
#include "ICallback.h"
#include"EOrder.h"
#include <iostream>
#include <sstream>
#include <map>
#include <cassert>

using namespace std;
using namespace plasma::client;
/////////////////////////////////////////////////////////////////////////
//
//
struct Sink {
	static const uint8_t ID = 9;

	map<uint32_t, EOrder*> _clt2Ord;
	map<uint32_t, EOrder*> _oid2Ord;
	uint32_t rpt_id = (ID << 24) + 8001;
	NonFillReport get_pnd_new(uint32_t clOrdId) {
		PROXY<NonFillReport> rpt;
		rpt << *_clt2Ord[clOrdId];
		rpt.execId(rpt_id++);
//		rpt.action(ExecType::Pending_New);
		rpt.status(OrdStatus::Pending_New);
		return rpt;
	}
	NonFillReport get_new(uint32_t clOrdId) {
		PROXY<NonFillReport> rpt;
		rpt << *_clt2Ord[clOrdId];
		rpt.execId(rpt_id++);
//		rpt.action(ExecType::New);
		rpt.status(OrdStatus::New);
		return rpt;
	}
	NonFillReport get_rjt(uint32_t clOrdId) {
		PROXY<NonFillReport> rpt;
		rpt << *_clt2Ord[clOrdId];
		rpt.execId(rpt_id++);
//		rpt.action(ExecType::Rejected);
		rpt.status(OrdStatus::Rejected);
		rpt.leavesQty(0);
		return rpt;
	}
	NonFillReport get_done(uint32_t clOrdId) {
		PROXY<NonFillReport> rpt;
		rpt << *_clt2Ord[clOrdId];
		rpt.execId(rpt_id++);
//		rpt.action(ExecType::Done_For_Day);
		rpt.status(OrdStatus::Done_For_Day);
		rpt.leavesQty(0);
		return rpt;
	}
	FillReport get_fill(uint32_t clOrdId, double lastQty, double lastPx) {
		EOrder& sts = *_clt2Ord[clOrdId];
		sts._avgPx = (sts._avgPx * sts._cumQty + lastQty * lastPx) / (sts._cumQty + lastQty);
		sts._cumQty += lastQty;
		if (sts.leavesQty() == 0) {
			sts._status = OrdStatus::Filled;
		}
		else {
			sts._status = OrdStatus::Partially_Filled;
		}
		PROXY<FillReport> rpt;
		rpt << sts;
		rpt.lastQty(lastQty);
		rpt.lastPx(lastPx);
		if (sts._head_cxl != 0) {
			rpt.status(OrdStatus::Pending_Cancel);
		}
		return rpt;
	}
	OrderCancelReject get_rjt(uint32_t clOrdId, uint32_t origClOrdId) {
		EOrder& orig = *_clt2Ord[origClOrdId];
		EOrder& sts = *_clt2Ord[clOrdId];

		// either a cxl or rpl reject
		assert(orig._head_cxl != 0 || orig._head_rpl != 0);
		// node to be deleted is first
		if (orig._head_cxl == sts._ordId)
			orig._head_cxl = sts._next;
		else if (orig._head_rpl == sts._ordId)
			orig._head_rpl = sts._next;

		// Change next only if node to be deleted is NOT the last node
		if (sts._next != 0)
			_oid2Ord[sts._next]->_prev = sts._prev;
		// Change prev only if node to be deleted is NOT the first node
		if (sts._prev != 0)
			_oid2Ord[sts._prev]->_next = sts._next;

		PROXY<OrderCancelReject> rjt;
		rjt.clOrdId(clOrdId);
		rjt << orig;
		rjt.status((rjt.status() == OrdStatus::NA) ? OrdStatus::New : rjt.status());
		if (orig._head_cxl != 0) {
			rjt.status(OrdStatus::Pending_Cancel);
		}
		if (orig._head_rpl != 0) {
			rjt.status(OrdStatus::Pending_Replace);
		}
		return rjt;
	}
	NonFillReport get_pnd_cxl(uint32_t clOrdId, uint32_t origClOrdId) {
		EOrder& orig = *_clt2Ord[origClOrdId];
		EOrder& sts = *_clt2Ord[clOrdId];

		// either first or existing current cxl does not have a chain
		assert(orig._head_cxl == 0 || _oid2Ord[orig._head_cxl]->_prev == 0);

		sts._next = orig._head_cxl;
		assert(sts._prev == 0);
		// there is a cancel req pending
		if (orig._head_cxl != 0) {
			_oid2Ord[orig._head_cxl]->_prev = sts._ordId;
		}
		// set the new cancel as head
		orig._head_cxl = sts._ordId;

		PROXY<NonFillReport> rpt;
		rpt << *_clt2Ord[origClOrdId];
		rpt.origClOrdId(origClOrdId);
		rpt.clOrdId(clOrdId);
//		rpt.orderId(ord_id++);

		rpt.execId(rpt_id++);
//		rpt.action(ExecType::Pending_Cancel);
		rpt.status(OrdStatus::Pending_Cancel);
		return rpt;
	}
	NonFillReport get_pnd_rpl(uint32_t clOrdId, uint32_t origClOrdId) {
		EOrder& orig = *_clt2Ord[origClOrdId];
		EOrder& sts = *_clt2Ord[clOrdId];

		// either first or existing current rpl does not have a chain
		assert(orig._head_rpl == 0 || _oid2Ord[orig._head_rpl]->_prev == 0);

		sts._next = orig._head_cxl;
		assert(sts._prev == 0);
		// there is a replace req pending
		if (orig._head_rpl != 0) {
			_oid2Ord[orig._head_rpl]->_prev = sts._ordId;
		}
		// set the new replace as head
		orig._head_rpl = sts._ordId;

		PROXY<NonFillReport> rpt;
		rpt << *_clt2Ord[origClOrdId];
		rpt.origClOrdId(origClOrdId);
		rpt.clOrdId(clOrdId);
//		rpt.orderId(ord_id++);

		rpt.execId(rpt_id++);
//		rpt.action(ExecType::Pending_Replace);
		rpt.status(OrdStatus::Pending_Replace);
		return rpt;
	}
	NonFillReport get_cxld(uint32_t clOrdId, uint32_t origClOrdId) {
		PROXY<NonFillReport> rpt;
		rpt << *_clt2Ord[origClOrdId];
		rpt.origClOrdId(origClOrdId);
		rpt.clOrdId(clOrdId);
//		rpt.orderId(ord_id++);

		rpt.execId(rpt_id++);
		rpt.leavesQty(0);
//		rpt.action(ExecType::Canceled);
		rpt.status(OrdStatus::Canceled);
		return rpt;
	}
	NonFillReport get_rpld(uint32_t clOrdId, uint32_t origClOrdId) {
		PROXY<NonFillReport> rpt;
		rpt << *_clt2Ord[clOrdId];
		rpt.origClOrdId(origClOrdId);
//		rpt.clOrdId(clOrdId);
//		rpt.orderId(ord_id++);

		rpt.execId(rpt_id++);
//		rpt.action(ExecType::Replace);
		rpt.status(OrdStatus::New);
		return rpt;
	}
};
/////////////////////////////////////////////////////////////////////////
//
//
class EPA final : public plasma::ICallback, public Sink
{
	uint32_t ord_id = 9001;
public:
	int ClOrdId = 0;
	uint8_t id() { return ID; }
	void OnMsg(const NewOrderSingle& req) override {
		stringstream strm;
		strm << "\tEPA:\tNOS[" << req.clOrdId() << "]";
		std::cout << strm.str() << std::endl;

		auto oid = ord_id++;
		_clt2Ord[req.clOrdId()] = new EOrder(oid, req);
		_oid2Ord[oid] = _clt2Ord[req.clOrdId()];
		ClOrdId = req.clOrdId();
	}
	void OnMsg(const OrderCancelRequest& req) override {
		stringstream strm;
		strm << "\tEPA:\tOCR[" << req.clOrdId() << "/" << req.origClOrdId() << "/" << req.orderId() << "]";
		std::cout << strm.str() << std::endl;

		auto oid = ord_id++;
		_clt2Ord[req.clOrdId()] = new EOrder(oid, req);
		_oid2Ord[oid] = _clt2Ord[req.clOrdId()];
		ClOrdId = req.clOrdId();
	}
	void OnMsg(const OrderReplaceRequest& req) override {
		stringstream strm;
		strm << "\tEPA:\tORR[" << req.clOrdId() << "/" << req.origClOrdId() << "/" << req.orderId() << "]";
		std::cout << strm.str() << std::endl;

		auto oid = ord_id++;
		_clt2Ord[req.clOrdId()] = new EOrder(oid, req);
		_oid2Ord[oid] = _clt2Ord[req.clOrdId()];
		ClOrdId = req.clOrdId();
	}
	void OnMsg(const OrderStatusRequest& req) override {
		stringstream strm;
		strm << "\tEPA:\tOSR[" << req.clOrdId() << "/ NA /" << req.orderId() << "]";
		std::cout << strm.str() << std::endl;
		ClOrdId = req.clOrdId();
	}
	void OnMsg(const ExecutionReport& rpt) override {
		// EPA sends it. does not receive it 
	}
	void OnMsg(const NonFillReport& rpt) override {
		// EPA sends it. does not receive it 
	}
	void OnMsg(const FillReport& rpt) override {
		// EPA sends it. does not receive it 
	}
	void OnMsg(const OrderCancelReject& rpt) override {
		// EPA sends it. does not receive it 
	}
	void OnMsg(const DontKnowTrade& rpt) override {
		stringstream strm;
		strm << "\tEPA\tDKT[" << rpt.orderId() << "/" << rpt.execId() << "]";
		std::cout << strm.str() << std::endl;
		// Just Log it. Nothing to do
	}
};