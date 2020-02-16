#pragma once
#include "EOrder.h"
#include "messages.h"
#include <map>
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
	auto get_pnd_new(uint32_t clOrdId) {
		EOrder& sts = *_clt2Ord[clOrdId];
		sts._status = OrdStatus::Pending_New;

		PROXY<ExecutionReport> rpt;
		rpt << sts;
		rpt.execId(rpt_id++);
		rpt.execType(ExecType::Pending_New);
		return rpt;
	}
	auto get_new(uint32_t clOrdId) {
		EOrder& sts = *_clt2Ord[clOrdId];
		sts._status = OrdStatus::New;

		PROXY<ExecutionReport> rpt;
		rpt << sts;
		rpt.execId(rpt_id++);
		rpt.execType(ExecType::New);
		// pending cancel requests
		if (sts._head_cxl != 0) {
			rpt.ordStatus(OrdStatus::Pending_Cancel);
		}
		// pending replace requests
		if (sts._head_rpl != 0) {
			rpt.ordStatus(OrdStatus::Pending_Replace);
		}
		return rpt;
	}
	// Reject NewOrderSingle
	auto get_rjt(uint32_t clOrdId) {
		EOrder& sts = *_clt2Ord[clOrdId];
		sts._status = OrdStatus::Rejected;

		PROXY<ExecutionReport> rpt;
		rpt << sts;
		rpt.execId(rpt_id++);
		rpt.execType(ExecType::Rejected);
		rpt.leavesQty(0);
		return rpt;
	}
	// Done_For_Day Execution Report
	auto get_done(uint32_t clOrdId) {
		EOrder& sts = *_clt2Ord[clOrdId];
		sts._status = OrdStatus::Done_For_Day;

		PROXY<ExecutionReport> rpt;
		rpt << sts;
		rpt.execId(rpt_id++);
		rpt.execType(ExecType::Done_For_Day);
		rpt.leavesQty(0);
		return rpt;
	}
	// Unsolicited cancel 
	auto get_cxld(uint32_t clOrdId) {
		EOrder& sts = *_clt2Ord[clOrdId];
		sts._status = OrdStatus::Canceled;

		PROXY<ExecutionReport> rpt;
		rpt << sts;
		rpt.execId(rpt_id++);
		rpt.execType(ExecType::Canceled);
		rpt.leavesQty(0);
		return rpt;
	}
	/////////////////////////////////////////////////////////////
	// execute order. 
	// OrderStatus can be Partial_Fill, Filled, Pending_Cancel, Pending_Replace
	auto get_fill(uint32_t clOrdId, double lastQty, double lastPx) {
		EOrder& sts = *_clt2Ord[clOrdId];
		sts._avgPx = (sts._avgPx * sts._cumQty + lastQty * lastPx) / (sts._cumQty + lastQty);
		sts._cumQty += lastQty;
		if (sts.leavesQty() == 0) {
			sts._status = OrdStatus::Filled;
		}
		else {
			sts._status = OrdStatus::Partially_Filled;
		}

		PROXY<ExecutionReport> rpt;
		rpt << sts;
		rpt.execId(rpt_id++);
		rpt.execType(ExecType::Trade);
		rpt.lastQty(lastQty);
		rpt.lastPx(lastPx);
		// pending cancel requests
		if (sts._head_cxl != 0) {
			rpt.ordStatus(OrdStatus::Pending_Cancel);
		}
		// pending replace requests
		if (sts._head_rpl != 0) {
			rpt.ordStatus(OrdStatus::Pending_Replace);
		}
		return rpt;
	}
	/////////////////////////////////////////////////////////////
	// reject replace or cancel request
	auto get_rjt(uint32_t clOrdId, uint32_t origClOrdId, const std::string& rjtReason) {
		// cannot find the orig request
		if (_clt2Ord.find(origClOrdId) == _clt2Ord.end()) {
			// Reject
			PROXY<OrderCancelReject> rjt;
			rjt.clOrdId(clOrdId);
			rjt.origClOrdId(origClOrdId);
			rjt.status(OrdStatus::Rejected);
			return rjt;
		}

		EOrder& orig = *_clt2Ord[origClOrdId];
		EOrder& sts = *_clt2Ord[clOrdId];

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
		// more cancel requests are there
		if (orig._head_cxl != 0) {
			rjt.status(OrdStatus::Pending_Cancel);
		}
		// more replace requests are there
		if (orig._head_rpl != 0) {
			rjt.status(OrdStatus::Pending_Replace);
		}
		return rjt;
	}
	/////////////////////////////////////////////////////////////
	// add to the chain of pending requests
	auto get_pnd_cxl(uint32_t clOrdId, uint32_t origClOrdId) {
		EOrder& orig = *_clt2Ord[origClOrdId];
		EOrder& sts = *_clt2Ord[clOrdId];

		// either first or existing current cxl does not have a chain
		assert(orig._head_cxl == 0 || _oid2Ord[orig._head_cxl]->_prev == 0);

		sts._next = orig._head_cxl;
		// there is a cancel req pending
		if (orig._head_cxl != 0) {
			_oid2Ord[orig._head_cxl]->_prev = sts._ordId;
		}
		// set the new cancel as head
		orig._head_cxl = sts._ordId;

		PROXY<ExecutionReport> rpt;
		rpt << *_clt2Ord[origClOrdId];
		// adjust the clOrdId & origClOrdId
		rpt.origClOrdId(origClOrdId);
		rpt.clOrdId(clOrdId);
		rpt.execId(rpt_id++);
		rpt.execType(ExecType::Pending_Cancel);
		rpt.ordStatus(OrdStatus::Pending_Cancel);
		return rpt;
	}
	/////////////////////////////////////////////////////////////
	// add to the chain of pending requests
	auto get_pnd_rpl(uint32_t clOrdId, uint32_t origClOrdId) {
		EOrder& orig = *_clt2Ord[origClOrdId];
		EOrder& sts = *_clt2Ord[clOrdId];

		// either first or existing current rpl does not have a chain
		assert(orig._head_rpl == 0 || _oid2Ord[orig._head_rpl]->_prev == 0);

		sts._next = orig._head_rpl;
		// there is a replace req pending
		if (orig._head_rpl != 0) {
			_oid2Ord[orig._head_rpl]->_prev = sts._ordId;
		}
		// set the new replace as head
		orig._head_rpl = sts._ordId;

		// publish the orig order details
		PROXY<ExecutionReport> rpt;
		rpt << *_clt2Ord[origClOrdId];
		// adjust the clOrdId & origClOrdId
		rpt.origClOrdId(origClOrdId);
		rpt.clOrdId(clOrdId);
		rpt.execId(rpt_id++);
		rpt.execType(ExecType::Pending_Replace);
		rpt.ordStatus(OrdStatus::Pending_Replace);
		return rpt;
	}
	/////////////////////////////////////////////////////////////
	// End state for origClOrdId and clOrdId
	// accept a cancel request and cancel the order
	// remove it from the pending requests
	// Actual we can empty the pending requests. 
	auto get_cxld(uint32_t clOrdId, uint32_t origClOrdId) {
		EOrder& orig = *_clt2Ord[origClOrdId];
		EOrder& sts = *_clt2Ord[clOrdId];

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

		// change status
		orig._status = OrdStatus::Canceled;

		PROXY<ExecutionReport> rpt;
		rpt << *_clt2Ord[origClOrdId];
		// adjust the clOrdId & origClOrdId
		rpt.origClOrdId(origClOrdId);
		rpt.clOrdId(clOrdId);
		rpt.execId(rpt_id++);
		rpt.execType(ExecType::Canceled);
		rpt.leavesQty(0);
		return rpt;
	}
	/////////////////////////////////////////////////////////////
	// End state for origClOrd
	// accept a replace request and replace the order
	// remove it from the pending requests
	// Actual we can empty the pending requests of origClOrdId
	auto get_rpld(uint32_t clOrdId, uint32_t origClOrdId) {
		EOrder& orig = *_clt2Ord[origClOrdId];
		EOrder& sts = *_clt2Ord[clOrdId];
		// copy the execution details to sts
		sts.Update(orig);
		if (sts.leavesQty() == 0) {
			sts._status = OrdStatus::Filled;
		}
		else {
			sts._status = OrdStatus::Partially_Filled;
		}
		// terminal state
		orig._status = OrdStatus::Replaced;

		PROXY<ExecutionReport> rpt;
		rpt << sts;
		rpt.origClOrdId(origClOrdId);
		rpt.execId(rpt_id++);
		rpt.execType(ExecType::Replace);
		// pending cancel requests
		if (sts._head_cxl != 0) {
			rpt.ordStatus(OrdStatus::Pending_Cancel);
		}
		// pending replace requests
		if (sts._head_rpl != 0) {
			rpt.ordStatus(OrdStatus::Pending_Replace);
		}
		return rpt;
	}
};
