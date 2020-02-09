#pragma once
#include "ICallback.h"
#include "messages.h"
#include <sstream>
#include <map>

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
	map<uint32_t, EOrder*> _clt2Ord;
	map<uint32_t, EOrder*> _oid2Ord;
public:
	const ExecutionReport&		execRpt()	{ return exe; }
	const OrderCancelReject&	cxlRjt()	{ return rjt; }
	uint32_t				req_seq_no()	{ return req_id++; }
	uint32_t				rpt_seq_no()	{ return rpt_id++; }
	EOrder*						order()		{ return _clt2Ord[ClOrdId]; };
	uint8_t						id()		{ return ID; }

	void OnMsg(const NewOrderSingle& req) override {
		stringstream strm;
		strm << "\tMLS:\tNOS[" << req.clOrdId() << "]";
		std::cout << strm.str() << std::endl;

		auto oid = ord_id++;
		_clt2Ord[req.clOrdId()] = new EOrder(oid, req);
		_oid2Ord[oid] = _clt2Ord[req.clOrdId()];
		ClOrdId = req.clOrdId();
	}
	void OnMsg(const OrderCancelRequest& req) override {
		stringstream strm;
		strm << "\tMLS:\tOCR[(" << req.clOrdId() << "," << req.origClOrdId() << ")/" << req.orderId() << "]";
		std::cout << strm.str() << std::endl;

		auto oid = ord_id++;
		_clt2Ord[req.clOrdId()] = new EOrder(oid, req, *_clt2Ord[req.origClOrdId()]);
		_oid2Ord[oid] = _clt2Ord[req.clOrdId()];
		ClOrdId = req.clOrdId();
	}
	void OnMsg(const OrderReplaceRequest& req) override {
		stringstream strm;
		strm << "\tMLS:\tORR[(" << req.clOrdId() << "," << req.origClOrdId() << ")/" << req.orderId() << "]";
		std::cout << strm.str() << std::endl;

		auto oid = ord_id++;
		_clt2Ord[req.clOrdId()] = new EOrder(oid, req, *_clt2Ord[req.origClOrdId()]);
		_oid2Ord[oid] = _clt2Ord[req.clOrdId()];
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
