#pragma once
#include "ICallback.h"
#include "messages.h"
#include "Sink.h"

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
	Sink		_snk;
public:
	uint8_t  id()		{ return ID; }
	Sink& sink()		{ return _snk; }
	uint32_t clOrdId() { return ClOrdId; }

	void OnMsg(const NewOrderSingle& req) override {
		stringstream strm;
		strm << "\tEPA:\tNOS[" << req.clOrdId() << "]";
		std::cout << strm.str() << std::endl;

		auto oid = ord_id++;
		_snk._clt2Ord[req.clOrdId()] = new EOrder(oid, req);
		_snk._oid2Ord[oid] = _snk._clt2Ord[req.clOrdId()];
		ClOrdId = req.clOrdId();
	}
	void OnMsg(const OrderCancelRequest& req) override {
		stringstream strm;
		strm << "\tEPA:\tOCR[(" << req.clOrdId() << "," << req.origClOrdId() << ")/" << req.orderId() << "]";
		std::cout << strm.str() << std::endl;

		auto oid = ord_id++;
		_snk._clt2Ord[req.clOrdId()] = new EOrder(oid, req, *_snk._clt2Ord[req.origClOrdId()]);
		_snk._oid2Ord[oid] = _snk._clt2Ord[req.clOrdId()];
		ClOrdId = req.clOrdId();
	}
	void OnMsg(const OrderReplaceRequest& req) override {
		stringstream strm;
		strm << "\tEPA:\tORR[(" << req.clOrdId() << "," << req.origClOrdId() << ")/" << req.orderId() << "]";
		std::cout << strm.str() << std::endl;

		auto oid = ord_id++;
		_snk._clt2Ord[req.clOrdId()] = new EOrder(oid, req, *_snk._clt2Ord[req.origClOrdId()]);
		_snk._oid2Ord[oid] = _snk._clt2Ord[req.clOrdId()];
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