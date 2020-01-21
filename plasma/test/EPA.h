#pragma once
#include "messages.h"
#include "ICallback.h"
#include"EOrder.h"
#include <iostream>
#include <sstream>
#include <map>
using namespace std;

/////////////////////////////////////////////////////////////////////////
//
//
struct Sink {
	static const uint8_t ID = 9;

	map<uint32_t, EOrder*> _clt2Ord;
	map<uint32_t, EOrder*> _oid2Ord;
	uint32_t rpt_id = (ID << 24) + 8001;
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
/////////////////////////////////////////////////////////////////////////
//
//
class EPA final : public plasma::ICallback, public Sink
{
	uint32_t ord_id = 9001;
public:
	int ClOrdId = 0;
	uint8_t id() { return ID; }
	void OnMsg(const NewOrderSingle& req) override {
		stringstream strm;
		strm << "\tEPA:\tNOS[" << req.clOrdId() << "]";
		std::cout << strm.str() << std::endl;
		auto oid = ord_id++;
		_clt2Ord[req.clOrdId()] = new EOrder(oid, req);
		_oid2Ord[oid] = _clt2Ord[req.clOrdId()];
		ClOrdId = req.clOrdId();
	}
	void OnMsg(const OrderCancelRequest& req) override {
		stringstream strm;
		strm << "\tEPA:\tOCR[" << req.clOrdId() << "/" << req.origClOrdId() << "/" << req.orderId() << "]";
		std::cout << strm.str() << std::endl;
		auto oid = ord_id++;
		_clt2Ord[req.clOrdId()] = new EOrder(oid, req);
		_oid2Ord[oid] = _clt2Ord[req.clOrdId()];
		ClOrdId = req.clOrdId();
	}
	void OnMsg(const OrderReplaceRequest& req) override {
		stringstream strm;
		strm << "\tEPA:\tORR[" << req.clOrdId() << "/" << req.origClOrdId() << "/" << req.orderId() << "]";
		std::cout << strm.str() << std::endl;
		auto oid = ord_id++;
		_clt2Ord[req.clOrdId()] = new EOrder(oid, req);
		_oid2Ord[oid] = _clt2Ord[req.clOrdId()];
		ClOrdId = req.clOrdId();
	}
	void OnMsg(const OrderStatusRequest& req) override {
		stringstream strm;
		strm << "\tEPA:\tOSR[" << req.clOrdId() << "/ NA /" << req.orderId() << "]";
		std::cout << strm.str() << std::endl;
		ClOrdId = req.clOrdId();
	}
	void OnMsg(const NonFillReport& rpt) override {
		// EPA sends it. does not receive it 
	}
	void OnMsg(const FillReport& rpt) override {
		// EPA sends it. does not receive it 
	}
	void OnMsg(const OrderCancelReject& rpt) override {
		// EPA sends it. does not receive it 
	}
	void OnMsg(const DontKnowTrade& rpt) override {
		stringstream strm;
		strm << "\tEPA\tDKT[" << rpt.orderId() << "/" << rpt.execId() << "]";
		std::cout << strm.str() << std::endl;
		// Just Log it. Nothing to do
	}
};