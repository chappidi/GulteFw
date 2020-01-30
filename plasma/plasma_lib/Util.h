#pragma once
#include <plasma_client/OrderStatusRequest.h>
#include <plasma_client/OrderCancelReject.h>
#include <plasma_client/ExecutionReport.h>

template<typename T>
struct Wrap : public T {
	char data[1024];
	Wrap() {
		wrapAndApplyHeader(data, 0, 1024);
	}
	Wrap(const T& o) {
		wrapAndApplyHeader(data, 0, 1024);
		assert(sbeSchemaVersion() == o.sbeSchemaVersion());
		memcpy(data, o.buffer(), o.bufferLength());
	}
};

static ExecutionReport& operator << (ExecutionReport& rpt, const OrderStatusRequest& osr)
{
	rpt.origClOrdId(0);
	rpt.clOrdId(osr.clOrdId());
	rpt.orderId(osr.orderId());
	rpt.symbol(osr.symbol());
	rpt.side(osr.side());

	rpt.execType(ExecType::Order_Status);
	rpt.ordStatus(OrdStatus::Rejected);
	rpt.qty(0);
	rpt.cumQty(0);
	rpt.leavesQty(0);
	rpt.avgPx(0);

	rpt.lastQty(0);
	rpt.lastPx(0);
	return rpt;
}

static OrderCancelReject& operator << (OrderCancelReject& rjt, const OrderCancelRequest& req) {
	rjt.origClOrdId(req.origClOrdId());
	rjt.clOrdId(req.clOrdId());
	rjt.orderId(req.orderId());
	rjt.status(OrdStatus::Rejected);
//	rjt.time_stamp(0);
	return rjt;
}
static OrderCancelReject& operator << (OrderCancelReject& rjt, const OrderReplaceRequest& req) {
	rjt.origClOrdId(req.origClOrdId());
	rjt.clOrdId(req.clOrdId());
	rjt.orderId(req.orderId());
	rjt.status(OrdStatus::Rejected);
//	rjt.time_stamp(0);
	return rjt;
}

static DontKnowTrade& operator << (DontKnowTrade dkt, const ExecutionReport& rpt) {
	dkt.orderId(rpt.orderId());
	dkt.execId(rpt.execId());
	dkt.symbol(rpt.symbol());
	dkt.side(rpt.side());
	dkt.reason(0);
	return dkt;
}
