#pragma once
#include <plasma_client/NewOrderSingle.h>
#include <plasma_client/OrderCancelRequest.h>
#include <plasma_client/OrderReplaceRequest.h>
#include <plasma_client/OrderStatusRequest.h>
#include <plasma_client/OrderCancelReject.h>
#include <set>
using namespace std;
using namespace plasma::client;

struct EOrder final
{
	uint32_t	_clOrdId;
	uint32_t	_ordId;
	//order key
	uint32_t	_symbol;
	Side::Value	_side;
	// other details
	OrdStatus::Value _status = OrdStatus::NA;
	double_t	_qty;
	double_t	_cumQty{ 0 };
	double_t	_avgPx{ 0 };

	// cancel and replace chaining
	uint32_t	_head_rpl{ 0 };  // start of replace chain
	uint32_t	_head_cxl{ 0 };  // start of cancel chain
	// can be either cxl or rpl chain or none
	uint32_t _next{ 0 };
	uint32_t _prev{ 0 };
//	set<uint32_t> _cxl_pending;
//	set<uint32_t> _rpl_pending;
	double_t leavesQty() const { return _qty - _cumQty; }

	explicit EOrder(uint32_t oid, const NewOrderSingle& req)
		: _ordId(oid) {
		_clOrdId = req.clOrdId();
		_symbol = req.symbol();
		_side = req.side();
		_qty = req.qty();
	}
	explicit EOrder(uint32_t oid, const OrderCancelRequest& req, const EOrder& orig)
		: _ordId(oid) {
		_clOrdId = req.clOrdId();
		_symbol = req.symbol();
		_side = req.side();
		// link to the original req
		_prev = orig._ordId;
	}
	explicit EOrder(uint32_t oid, const OrderReplaceRequest& req, const EOrder& orig)
		: _ordId(oid) {
		_clOrdId = req.clOrdId();
		_symbol = req.symbol();
		_side = req.side();
		_qty = req.qty();
	}
};
static ExecutionReport& operator << (ExecutionReport& rpt, const EOrder& sts) {
	rpt.clOrdId(sts._clOrdId);
	rpt.orderId(sts._ordId);

	rpt.symbol(sts._symbol);
	rpt.side(sts._side);
	rpt.qty(sts._qty);

	rpt.ordStatus(sts._status);
	rpt.cumQty(sts._cumQty);
	rpt.leavesQty(sts.leavesQty());
	rpt.avgPx(sts._avgPx);

	rpt.lastQty(0);
	rpt.lastPx(0);
	return rpt;
}
static OrderCancelReject& operator << (OrderCancelReject& rjt, const EOrder& sts) {
	rjt.origClOrdId(sts._clOrdId);
	rjt.orderId(sts._ordId);

	rjt.symbol(sts._symbol);
	rjt.side(sts._side);
	rjt.status(sts._status);
	return rjt;
}
