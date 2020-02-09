#pragma once
#include "ICallback.h"

using namespace std;
using namespace plasma::client;

/////////////////////////////////////////////////////////////////////////
//
//
class MLS final : public plasma::ICallback 
{
	static const uint8_t ID = 7;
public:
	uint8_t id() { return ID; }
	void OnMsg(const NewOrderSingle& req) override {
		stringstream strm;
		strm << "\tMLS:\tNOS[" << req.clOrdId() << "]";
		std::cout << strm.str() << std::endl;

		//auto oid = ord_id++;
		//_clt2Ord[req.clOrdId()] = new EOrder(oid, req);
		//_oid2Ord[oid] = _clt2Ord[req.clOrdId()];
		//ClOrdId = req.clOrdId();
	}
	void OnMsg(const OrderCancelRequest& req) override {
		stringstream strm;
		strm << "\tMLS:\tOCR[(" << req.clOrdId() << "," << req.origClOrdId() << ")/" << req.orderId() << "]";
		std::cout << strm.str() << std::endl;
	}
	void OnMsg(const OrderReplaceRequest& req) override {
		stringstream strm;
		strm << "\tMLS:\tORR[(" << req.clOrdId() << "," << req.origClOrdId() << ")/" << req.orderId() << "]";
		std::cout << strm.str() << std::endl;
	}
	void OnMsg(const OrderStatusRequest& req) override {
		stringstream strm;
		strm << "\tMLS:\tOSR[(" << req.clOrdId() << ")/" << req.orderId() << "]";
	}
	void OnMsg(const ExecutionReport& rpt) override {
		stringstream strm;
		strm << "\tMLS:\tEXE[(" << ClientId(rpt.clOrdId()) << "," << ClientId(rpt.origClOrdId()) << ")-->" << rpt.orderId() << "] ";
		stringstream  sts;
		sts << ExecType::c_str(rpt.execType()) << " / " << OrdStatus::c_str(rpt.ordStatus());
		stringstream  sQty;
		sQty << "[" << rpt.qty() << "," << rpt.cumQty() << "," << rpt.leavesQty() << "]";
		printf("%20s %15s / %15s %20s\n", strm.str().c_str(), ExecType::c_str(rpt.execType()), OrdStatus::c_str(rpt.ordStatus()), sQty.str().c_str());
	}
	void OnMsg(const OrderCancelReject& rpt) override {
		stringstream strm;
		strm << "\tMLS:\tRJT[" << ClientId(rpt.clOrdId()) << "/" << ClientId(rpt.origClOrdId()) << "/" << rpt.orderId() << "] ";
		strm << OrdStatus::c_str(rpt.status());
		std::cout << strm.str() << std::endl;
	}
	void OnMsg(const DontKnowTrade& rpt) override {
		// Never receives it
	}
};
