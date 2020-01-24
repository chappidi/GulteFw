#pragma once
#include "messages.h"
#include "ICallback.h"
#include <iostream>
#include <sstream>
#include <iomanip>
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
		PROXY<NewOrderSingle> req;
		req.clOrdId(req_id++);
		req.target(tgt);
		req.symbol(9999);
		req.side(plasma::client::Side::BUY);
		req.qty(qty);
		return req;
	}
	auto get_cxl(uint32_t clOrdId, const NewOrderSingle& nos)
	{
		PROXY<OrderCancelRequest> req;
		req.clOrdId(req_id++);
		req.origClOrdId(nos.clOrdId());
		req.symbol(nos.symbol());
		req.side(nos.side());
		req.orderId(clOrdId);
		return req;
	}
	auto get_rpl(uint32_t clOrdId, double_t qty, const NewOrderSingle& nos)
	{
		PROXY<OrderReplaceRequest> req;
		req.clOrdId(req_id++);
		req.origClOrdId(nos.clOrdId());
		req.symbol(nos.symbol());
		req.side(nos.side());
		req.orderId(clOrdId);
		req.qty(qty);
		return req;
	}
	auto get_sts(const NewOrderSingle& nos)
	{
		PROXY<OrderStatusRequest> req;
		req.clOrdId(nos.clOrdId());
		req.symbol(nos.symbol());
		req.side(nos.side());
		req.qty(nos.qty());
		auto itr = _clt2ord.find(nos.clOrdId());
		req.orderId((itr != _clt2ord.end()) ? itr->second : 0);
		return req;
	}
	auto get_sts(const OrderReplaceRequest& orr)
	{
		PROXY<OrderStatusRequest> req;
		req.clOrdId(orr.clOrdId());
		req.symbol(orr.symbol());
		req.side(orr.side());
		req.qty(orr.qty());
		auto itr = _clt2ord.find(orr.clOrdId());
		req.orderId((itr != _clt2ord.end()) ? itr->second : 0);
		return req;
	}
};

class GUI final : public plasma::ICallback, public Source
{
public:
	PROXY<ExecutionReport>	exe;
	PROXY<OrderCancelReject> rjt;
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
	void OnMsg(const ExecutionReport& rpt) override {
		stringstream strm;
		strm << "\tGUI:\tEXE[(" << ClientId(rpt.clOrdId()) << "," << ClientId(rpt.origClOrdId()) << ")-->" << rpt.orderId() << "] ";
		stringstream  sts;
		sts << ExecType::c_str(rpt.execType()) << " / " << OrdStatus::c_str(rpt.ordStatus());
		stringstream  sQty;
		sQty << "[" << rpt.qty() << "," << rpt.cumQty() << "," << rpt.leavesQty() << "]";
		//		std::cout << strm.str() << sQty.str() << std::endl;
		printf("%20s %30s %20s\n", strm.str().c_str(), sts.str().c_str(), sQty.str().c_str());
		exe = rpt;
	}
	void OnMsg(const OrderCancelReject& rpt) override {
		stringstream strm;
		strm << "\tGUI:\tRJT[" << ClientId(rpt.clOrdId()) << "/" << ClientId(rpt.origClOrdId()) << "/" << rpt.orderId() << "] ";
		strm << OrdStatus::c_str(rpt.status());
		std::cout << strm.str() << std::endl;
		rjt = rpt;
	}
	void OnMsg(const DontKnowTrade& rpt) override {
		// Never receives it
	}
};