#pragma once
#include <plasma_client/NewOrderSingle.h>
#include <plasma_client/OrderCancelRequest.h>
#include <plasma_client/OrderReplaceRequest.h>
#include <cassert>
using namespace std;
using namespace plasma::client;

// terminal status = DoneForDay, Rejected, Replaced 
struct OrderV2 final
{
	// tree 
	uint32_t	_parent{ 0 };
	uint32_t	_child{ 0 };	// first child
	uint32_t    _next{ 0 };		// next Sibling  linked chain
	// plasma ref id
	uint32_t			_plsOrdId;
	uint32_t			_origPlsOrdId{ 0 };
	// external ref ids. upstream and downstream
	uint32_t			_srcOrdId;
	uint32_t			_dstOrdId{ 0 };
	// replaced by
	uint32_t			_chain{ 0 };
	// economics
	uint32_t			_symbol;
	double_t			_qty;
	double_t			_slicedQty{ 0 }; // sum of (qty of active child  + exec_qty of inactive child)
	// side is not economics u cannot compare ShortSell & Sell
	Side::Value			_side;
	uint8_t				_target;
	// other details
	OrdStatus::Value	_status = OrdStatus::NA;
	double_t			_cumQty{ 0 };
	double_t			_leavesQty{ 0 };
	double_t			_avgPx{ 0 };

	inline uint8_t target() const { return _target; }
	inline bool isChild() const { return _parent != 0; }
	inline bool isParent() const { return _parent == 0; }

	explicit OrderV2(uint32_t id, const NewOrderSingle& req)
		: _plsOrdId(id) {
		_parent = req.parentId();
		_srcOrdId = req.clOrdId();
		// economics
		_symbol = req.symbol();
		_side = req.side();
		_qty = req.qty();
		_target = req.target();	
		// status
		_leavesQty = _qty;
	}
	explicit OrderV2(uint32_t id, const OrderCancelRequest& req, const OrderV2& orig)
		: _plsOrdId(id) {
		_origPlsOrdId = orig._plsOrdId;
		_srcOrdId = req.clOrdId();
		// economics
		_symbol = req.symbol();
		_side = req.side();
		_qty = req.qty();
		_target = orig.target();
	}
	explicit OrderV2(uint32_t id, const OrderReplaceRequest& req, const OrderV2& orig)
		: _plsOrdId(id) {
		_origPlsOrdId = orig._plsOrdId;
		_srcOrdId = req.clOrdId();
		// economics
		_symbol = req.symbol();
		_side = req.side();
		_qty = req.qty();
		_target = orig.target();
	}
	void Update(const ExecutionReport& rpt)
	{
		_dstOrdId = rpt.orderId();
		_status = rpt.ordStatus();
		_cumQty = rpt.cumQty();
		_leavesQty = rpt.leavesQty();
		_avgPx = rpt.avgPx();
	}
};
/*
	Generate a default execution status report from Order
	Used when the OMS is needed to generate a ExecutionReport internally
*/
static ExecutionReport& operator << (ExecutionReport& rpt, const OrderV2& sts)
{
	// set ids override as required
	rpt.origClOrdId(0);
	rpt.clOrdId(sts._srcOrdId);
	rpt.orderId(sts._plsOrdId);
	// economics
	rpt.symbol(sts._symbol);
	rpt.side(sts._side);
	rpt.qty(sts._qty);

	// default : override as required
	rpt.execType(ExecType::Order_Status);	
	rpt.ordStatus(sts._status);
	rpt.cumQty(sts._cumQty);
	rpt.leavesQty(sts._leavesQty);
	rpt.avgPx(sts._avgPx);
	// fill details
	rpt.lastQty(0);
	rpt.lastPx(0);
	return rpt;
}
