#pragma once

struct Sink {
	static const uint8_t ID = 9;
};

class EPA final : public plasma::ICallback, public Sink
{
	uint8_t id() { return ID; }
	void OnMsg(const plasma::client::NewOrderSingle& req) {
		// Never receives it
	}
	void OnMsg(const plasma::client::OrderCancelRequest& req) {
		// Never receives it
	}
	void OnMsg(const plasma::client::OrderReplaceRequest& req) {
		// Never receives it
	}
	void OnMsg(const plasma::client::OrderStatusRequest& req) {
		// Never receives it
	}
	void OnMsg(const plasma::client::NonFillReport& rpt) {
	}
	void OnMsg(const plasma::client::FillReport& rpt) {
	}
	void OnMsg(const plasma::client::OrderCancelReject& rpt) {
	}
	void OnMsg(const plasma::client::DontKnowTrade& rpt) {
		// Never receives it
	}
};