#pragma once
#include <plasma_client/NewOrderSingle.h>
#include <plasma_client/OrderCancelRequest.h>
#include <plasma_client/OrderReplaceRequest.h>
#include <cassert>
using namespace std;
using namespace plasma::client;

// terminal status = DoneForDay, Rejected, Replaced 
struct Order final
{
	uint32_t	_srcOrdId;
	uint32_t	_plsOrdId;
	uint32_t	_dstOrdId{ 0 };
	uint32_t	_origPlsOrdId{ 0 };
	//order key
	uint32_t	_symbol;
	Side::Value	_side;
	uint8_t		_target;
	double_t	_qty;
	// other details
	double_t	_cumQty{ 0 };
	double_t	_leavesQty{ 0 };
	double_t	_avgPx{ 0 };
	OrdStatus::Value	_status = OrdStatus::NA;

	// replaced by
	uint32_t	_rpldOrdId{ 0 };  
	// cancel and replace chaining
	uint32_t	_head_lklt{ 0 };
	// can be either cxl or rpl chain or none
	uint32_t _next{ 0 };
	uint32_t _prev{ 0 };

	inline uint8_t target() const { return _target; }

	explicit Order(uint32_t id, const NewOrderSingle& req)
		: _plsOrdId(id) {
		_srcOrdId = req.clOrdId();
		// economics
		_symbol = req.symbol();
		_side = req.side();
		_qty = req.qty();
		_target = req.target();	
		// status
		_leavesQty = _qty;
	}
	// OrderCancelReq = OrderReplaceRequest ( qty = 0)
	explicit Order(uint32_t id, const OrderCancelRequest& req, const Order& orig)
		: _plsOrdId(id) {
		_origPlsOrdId = orig._plsOrdId;
		_srcOrdId = req.clOrdId();
		_symbol = req.symbol();
		_side = req.side();
		_qty = 0;
		_target = orig.target();
	}
	explicit Order(uint32_t id, const OrderReplaceRequest& req, const Order& orig)
		: _plsOrdId(id) {
		_origPlsOrdId = orig._plsOrdId;
		_srcOrdId = req.clOrdId();
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
static ExecutionReport& operator << (ExecutionReport& rpt, const Order& sts)
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
