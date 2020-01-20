#pragma once
#include "messages.h"
#include "ICallback.h"

struct Sink {
	static const uint8_t ID = 9;
	NonFillReport get_pnd_new(uint32_t clOrdId) {
		PROXY<NonFillReport> rpt;
//		rpt.status(plasma::client::OrdStatus::Pending_New);
		return rpt;
	}
	NonFillReport get_new(uint32_t clOrdId) {
		PROXY<NonFillReport> rpt;
		return rpt;
	}
	NonFillReport get_rjt(uint32_t clOrdId) {
		PROXY<NonFillReport> rpt;
		return rpt;
	}
	NonFillReport get_done(uint32_t clOrdId) {
		PROXY<NonFillReport> rpt;
		return rpt;
	}
	FillReport get_fill(uint32_t clOrdId, double lastQty, double lastPx) {
		PROXY<FillReport> rpt;
		rpt.lastQty(lastQty);
		rpt.lastPx(lastPx);
		return rpt;
	}
	OrderCancelReject get_rjt(uint32_t clOrdId, uint32_t origClOrdId) {
		PROXY<OrderCancelReject> rpt;
		return rpt;
	}
	NonFillReport get_pnd_cxl(uint32_t clOrdId, uint32_t origClOrdId) {
		PROXY<NonFillReport> rpt;
		return rpt;
	}
	NonFillReport get_pnd_rpl(uint32_t clOrdId, uint32_t origClOrdId) {
		PROXY<NonFillReport> rpt;
		return rpt;
	}
	NonFillReport get_cxld(uint32_t clOrdId, uint32_t origClOrdId) {
		PROXY<NonFillReport> rpt;
		return rpt;
	}
	NonFillReport get_rpld(uint32_t clOrdId, uint32_t origClOrdId) {
		PROXY<NonFillReport> rpt;
		return rpt;
	}
};

class EPA final : public plasma::ICallback, public Sink
{
public:
	uint8_t id() { return ID; }
	void OnMsg(const NewOrderSingle& req) override {
		// Never receives it
	}
	void OnMsg(const OrderCancelRequest& req) override {
		// Never receives it
	}
	void OnMsg(const OrderReplaceRequest& req) override {
		// Never receives it
	}
	void OnMsg(const OrderStatusRequest& req) override {
		// Never receives it
	}
	void OnMsg(const NonFillReport& rpt) override {
	}
	void OnMsg(const FillReport& rpt) override {
	}
	void OnMsg(const OrderCancelReject& rpt) override {
	}
	void OnMsg(const DontKnowTrade& rpt) override {
		// Never receives it
	}
};