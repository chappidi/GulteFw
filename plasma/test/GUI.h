#pragma once
#include "messages.h"
#include "ICallback.h"
#include <map>
using namespace std;

class Source {
protected:
	uint32_t req_id = (ID << 24) + 7001;
	map<uint32_t, uint32_t> _clt2ord;
public:
	static const uint8_t ID = 5;
	// utility functions
	auto get_nos(uint8_t tgt, double_t qty)
	{
		proxy::NewOrderSingle req(req_id++, tgt);
		req.symbol(9999);
		req.side(plasma::client::Side::BUY);
		req.qty(qty);
		return req;
	}
	auto get_cxl(uint32_t clOrdId, const proxy::NewOrderSingle& nos)
	{
		proxy::OrderCancelRequest req(req_id++, nos);
		req.orderId(clOrdId);
		return req;
	}
	auto get_rpl(uint32_t clOrdId, double_t qty, const proxy::NewOrderSingle& nos)
	{
		proxy::OrderReplaceRequest req(req_id++, nos);
		req.orderId(clOrdId);
		req.qty(qty);
		return req;
	}
	auto get_sts(const proxy::NewOrderSingle& nos)
	{
		proxy::OrderStatusRequest req(nos);
		auto itr = _clt2ord.find(nos.clOrdId());
		req.orderId((itr != _clt2ord.end()) ? itr->second : 0);
		return req;
	}
	auto get_sts(const proxy::OrderReplaceRequest& orr)
	{
		proxy::OrderStatusRequest req(orr);
		auto itr = _clt2ord.find(orr.clOrdId());
		req.orderId((itr != _clt2ord.end()) ? itr->second : 0);
		return req;
	}
};

class GUI final : public plasma::ICallback, public Source
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