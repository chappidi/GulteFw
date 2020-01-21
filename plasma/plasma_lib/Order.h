#pragma once
#include <plasma_client/NewOrderSingle.h>
#include <plasma_client/OrderCancelRequest.h>
#include <plasma_client/OrderReplaceRequest.h>

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
	uint8_t		_side;
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
};