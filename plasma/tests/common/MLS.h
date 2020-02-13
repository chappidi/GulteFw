#pragma once
#include "ICallback.h"
#include "messages.h"
#include <sstream>
#include "Sink.h"

using namespace std;
using namespace plasma::client;

/////////////////////////////////////////////////////////////////////////
//
//
class MLS final : public ISource, public ITarget, public plasma::ICallback
{
	static const uint8_t ID = 7;
	uint32_t rpt_id = (ID << 24) + 9001;
	uint32_t req_id = (ID << 24) + 3001;
	PROXY<ExecutionReport>	exe;
	PROXY<OrderCancelReject> rjt;
	int			ClOrdId = 0;
	uint32_t	ord_id = 2001;
	Sink		_snk;
public:
	const ExecutionReport&		execRpt(uint32_t clOrdId)	{ return exe; }
	const OrderCancelReject&	cxlRjt()	{ return rjt; }
	uint32_t				req_seq_no()	{ return req_id++; }
	uint32_t					clOrdId()	{ return ClOrdId; }
	uint8_t						id()		{ return ID; }
	Sink&						sink()		{ return _snk; }

	void OnMsg(const NewOrderSingle& req) override {
		stringstream strm;
		strm << "\tMLS:\tNOS[" << req.clOrdId() << "]";
		std::cout << strm.str() << std::endl;

		auto oid = ord_id++;
		_snk._clt2Ord[req.clOrdId()] = new EOrder(oid, req);
		_snk._oid2Ord[oid] = _snk._clt2Ord[req.clOrdId()];
		ClOrdId = req.clOrdId();
	}
	void OnMsg(const OrderCancelRequest& req) override {
		stringstream strm;
		strm << "\tMLS:\tOCR[(" << req.clOrdId() << "," << req.origClOrdId() << ")/" << req.orderId() << "]";
		std::cout << strm.str() << std::endl;

		auto oid = ord_id++;
		_snk._clt2Ord[req.clOrdId()] = new EOrder(oid, req, *_snk._clt2Ord[req.origClOrdId()]);
		_snk._oid2Ord[oid] = _snk._clt2Ord[req.clOrdId()];
		ClOrdId = req.clOrdId();
	}
	void OnMsg(const OrderReplaceRequest& req) override {
		stringstream strm;
		strm << "\tMLS:\tORR[(" << req.clOrdId() << "," << req.origClOrdId() << ")/" << req.orderId() << "]";
		std::cout << strm.str() << std::endl;

		auto oid = ord_id++;
		_snk._clt2Ord[req.clOrdId()] = new EOrder(oid, req, *_snk._clt2Ord[req.origClOrdId()]);
		_snk._oid2Ord[oid] = _snk._clt2Ord[req.clOrdId()];
		ClOrdId = req.clOrdId();
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
		exe = rpt;
	}
	void OnMsg(const OrderCancelReject& rpt) override {
		stringstream strm;
		strm << "\tMLS:\tRJT[" << ClientId(rpt.clOrdId()) << "/" << ClientId(rpt.origClOrdId()) << "/" << rpt.orderId() << "] ";
		strm << OrdStatus::c_str(rpt.status());
		std::cout << strm.str() << std::endl;
		rjt = rpt;
	}
	void OnMsg(const DontKnowTrade& rpt) override {
		// Never receives it
	}
};
