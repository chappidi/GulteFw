#pragma once
#include "ICallback.h"
#include "messages.h"
#include <map>

using namespace std;
using namespace plasma::client;

/////////////////////////////////////////////////////////////////////////
//
//
class EPA final : public ITarget, public plasma::ICallback
{
	static const uint8_t ID = 9;
	uint32_t rpt_id = (ID << 24) + 8001;
	int			ClOrdId = 0;
	uint32_t	ord_id = 9001;
	map<uint32_t, EOrder*> _clt2Ord;
	map<uint32_t, EOrder*> _oid2Ord;
public:
	EOrder* order()		{ return _clt2Ord[ClOrdId]; };
	uint32_t rpt_seq_no() { return rpt_id++; }
	uint8_t  id()		{ return ID; }

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
		strm << "\tEPA:\tOCR[(" << req.clOrdId() << "," << req.origClOrdId() << ")/" << req.orderId() << "]";
		std::cout << strm.str() << std::endl;

		auto oid = ord_id++;
		_clt2Ord[req.clOrdId()] = new EOrder(oid, req, *_clt2Ord[req.origClOrdId()]);
		_oid2Ord[oid] = _clt2Ord[req.clOrdId()];
		ClOrdId = req.clOrdId();
	}
	void OnMsg(const OrderReplaceRequest& req) override {
		stringstream strm;
		strm << "\tEPA:\tORR[(" << req.clOrdId() << "," << req.origClOrdId() << ")/" << req.orderId() << "]";
		std::cout << strm.str() << std::endl;

		auto oid = ord_id++;
		_clt2Ord[req.clOrdId()] = new EOrder(oid, req, *_clt2Ord[req.origClOrdId()]);
		_oid2Ord[oid] = _clt2Ord[req.clOrdId()];
		ClOrdId = req.clOrdId();
	}
	void OnMsg(const OrderStatusRequest& req) override {
		stringstream strm;
		strm << "\tEPA:\tOSR[(" << req.clOrdId() << ")/" << req.orderId() << "]";
//		std::cout << strm.str() << std::endl;
	}
	void OnMsg(const ExecutionReport& rpt) override {
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