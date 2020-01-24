#pragma once
#include <plasma_client/OrderStatusRequest.h>
#include <plasma_client/OrderCancelReject.h>
#include <plasma_client/NonFillReport.h>

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

NonFillReport& operator << (NonFillReport& ntr, const OrderStatusRequest& osr)
{
	ntr.clOrdId(osr.clOrdId());
	ntr.orderId(osr.orderId());
	ntr.symbol(osr.symbol());
	ntr.side(osr.side());

//	ntr.action(ExecType::Order_Status);
	ntr.status(OrdStatus::Rejected);
	ntr.qty(0);
	ntr.cumQty(0);
	ntr.leavesQty(0);
	ntr.avgPx(0);
	return ntr;
}

OrderCancelReject& operator << (OrderCancelReject& rjt, const OrderCancelRequest& req) {
	rjt.origClOrdId(req.origClOrdId());
	rjt.clOrdId(req.clOrdId());
	rjt.orderId(req.orderId());
	rjt.status(OrdStatus::Rejected);
//	rjt.time_stamp(0);
	return rjt;
}
OrderCancelReject& operator << (OrderCancelReject& rjt, const OrderReplaceRequest& req) {
	rjt.origClOrdId(req.origClOrdId());
	rjt.clOrdId(req.clOrdId());
	rjt.orderId(req.orderId());
	rjt.status(OrdStatus::Rejected);
//	rjt.time_stamp(0);
	return rjt;
}

DontKnowTrade& operator << (DontKnowTrade dkt, const ExecutionReport& rpt) {
	dkt.orderId(rpt.orderId());
	dkt.execId(rpt.execId());
	dkt.symbol(rpt.symbol());
	dkt.side(rpt.side());
	dkt.reason(0);
	return dkt;
}

DontKnowTrade& operator << (DontKnowTrade dkt, const NonFillReport& rpt) {
	dkt.orderId(rpt.orderId());
	dkt.execId(rpt.execId());
	dkt.symbol(rpt.symbol());
	dkt.side(rpt.side());
	dkt.reason(0);
	return dkt;
}
DontKnowTrade& operator << (DontKnowTrade dkt, const FillReport& rpt) {
	dkt.orderId(rpt.orderId());
	dkt.execId(rpt.execId());
	dkt.symbol(rpt.symbol());
	dkt.side(rpt.side());
	dkt.reason(0);
	return dkt;
}