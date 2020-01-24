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
	uint32_t	_rplOrdId{ 0 };  // next in replace chain
	//order key
	uint32_t	_symbol;
	Side::Value	_side;
	uint8_t		_target;
	// other details
	OrdStatus::Value	_status = OrdStatus::NA;
	double_t	_qty;
	double_t	_cumQty{ 0 };
	double_t	_avgPx{ 0 };

	inline uint8_t target() const { return _target; }

	double_t leavesQty() const { return _qty - _cumQty; }

	explicit Order(uint32_t id, const NewOrderSingle& req)
		: _plsOrdId(id) {
		_srcOrdId = req.clOrdId();
		_symbol = req.symbol();
		_side = req.side();
		_qty = req.qty();
		_target = req.target();
		_avgPx = 0;
	}
	explicit Order(uint32_t id, const OrderCancelRequest& req, uint8_t tgt)
		: _plsOrdId(id) {
		_srcOrdId = req.clOrdId();
		_symbol = req.symbol();
		_side = req.side();
		_target = tgt;
	}
	explicit Order(uint32_t id, const OrderReplaceRequest& req, uint8_t tgt)
		: _plsOrdId(id) {
		_srcOrdId = req.clOrdId();
		_symbol = req.symbol();
		_side = req.side();
		_qty = req.qty();
		_target = tgt;
	}
	void Update(const ExecutionReport& rpt)
	{
		_dstOrdId = rpt.orderId();
		_status = rpt.ordStatus();
		_cumQty = rpt.cumQty();
		_avgPx = rpt.avgPx();
	}
	void Update(const NonFillReport& rpt)
	{
//		assert(_dstOrdId == rpt.orderId());
		_dstOrdId = rpt.orderId();
		_status = rpt.status();
		_cumQty = rpt.cumQty();
		_avgPx = rpt.avgPx();
	}
	void Update(const FillReport& rpt) {
		assert(_dstOrdId == rpt.orderId());
//		_dstOrdId = rpt._orderId;
		_status = rpt.status();
		_cumQty = rpt.cumQty();
		_avgPx = rpt.avgPx();
	}
};
static ExecutionReport& operator << (ExecutionReport& rpt, const Order& sts)
{
	rpt.clOrdId(sts._srcOrdId);
	rpt.orderId(sts._plsOrdId);

	rpt.symbol(sts._symbol);
	rpt.side(sts._side);
	rpt.qty(sts._qty);
	//populate execType outside
	rpt.ordStatus(sts._status);
	rpt.cumQty(sts._cumQty);
	rpt.leavesQty(sts.leavesQty());
	rpt.avgPx(sts._avgPx);
	return rpt;
}

static NonFillReport& operator << (NonFillReport& ntr, const Order& sts)
{
	ntr.clOrdId(sts._srcOrdId);
	ntr.orderId(sts._plsOrdId);

	ntr.symbol(sts._symbol);
	ntr.side(sts._side);
	ntr.qty(sts._qty);

	ntr.status(sts._status);
	ntr.cumQty(sts._cumQty);
	ntr.leavesQty(sts.leavesQty());
	ntr.avgPx(sts._avgPx);
	return ntr;
}
