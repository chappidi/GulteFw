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
class GUI final : public ISource, public plasma::ICallback
{
	static const uint8_t ID = 5;
	uint32_t req_id = (ID << 24) + 7001;
	map<uint32_t, PROXY<ExecutionReport>> execs;
	PROXY<OrderCancelReject> rjt;
public:
	PROXY<NewOrderSingle> get_nos(uint8_t tgt, double_t qty, uint32_t prnt)
	{
		PROXY<NewOrderSingle> req;
		// fill in the details
		req.parent(prnt)
			.clOrdId(req_seq_no())
			.target(tgt)
			.symbol(9999)
			.side(plasma::client::Side::BUY)
			.qty(qty);

		return req;
	}
	uint32_t req_seq_no()	{ return req_id++; }
	uint8_t  id()			{ return ID; }
	const ExecutionReport& execRpt(uint32_t clOrdId) { 
		return execs[clOrdId]; 
	}
	const OrderCancelReject& cxlRjt() { return rjt; }

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
		sQty << "[" << rpt.qty() << ", cq=" << rpt.cumQty() << ", lq=" << rpt.leavesQty() << ", wq=" << rpt.workingQty() << "]";
		//		std::cout << strm.str() << sQty.str() << std::endl;
		printf("%20s %15s / %15s %20s\n", strm.str().c_str(), ExecType::c_str(rpt.execType()), OrdStatus::c_str(rpt.ordStatus()), sQty.str().c_str());
		execs[rpt.clOrdId()] = rpt;
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