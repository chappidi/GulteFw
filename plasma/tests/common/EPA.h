#pragma once
#include "ICallback.h"

using namespace std;
using namespace plasma::client;

/////////////////////////////////////////////////////////////////////////
//
//
class EPA final : public plasma::ICallback 
{
	static const uint8_t ID = 9;
public:
	uint8_t id() { return ID; }
	void OnMsg(const NewOrderSingle& req) override {
		stringstream strm;
		strm << "\tEPA:\tNOS[" << req.clOrdId() << "]";
		std::cout << strm.str() << std::endl;

		//auto oid = ord_id++;
		//_clt2Ord[req.clOrdId()] = new EOrder(oid, req);
		//_oid2Ord[oid] = _clt2Ord[req.clOrdId()];
		//ClOrdId = req.clOrdId();
	}
	void OnMsg(const OrderCancelRequest& req) override {
		stringstream strm;
		strm << "\tEPA:\tOCR[(" << req.clOrdId() << "," << req.origClOrdId() << ")/" << req.orderId() << "]";
		std::cout << strm.str() << std::endl;

		//auto oid = ord_id++;
		//_clt2Ord[req.clOrdId()] = new EOrder(oid, req, *_clt2Ord[req.origClOrdId()]);
		//_oid2Ord[oid] = _clt2Ord[req.clOrdId()];
		//ClOrdId = req.clOrdId();
	}
	void OnMsg(const OrderReplaceRequest& req) override {
		stringstream strm;
		strm << "\tEPA:\tORR[(" << req.clOrdId() << "," << req.origClOrdId() << ")/" << req.orderId() << "]";
		std::cout << strm.str() << std::endl;

		//auto oid = ord_id++;
		//_clt2Ord[req.clOrdId()] = new EOrder(oid, req, *_clt2Ord[req.origClOrdId()]);
		//_oid2Ord[oid] = _clt2Ord[req.clOrdId()];
		//ClOrdId = req.clOrdId();
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