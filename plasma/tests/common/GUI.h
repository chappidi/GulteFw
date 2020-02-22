#pragma once
#include "ICallback.h"
#include "messages.h"
#include <sstream>
#include <iomanip> // required for std::setw
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
		stringstream stm;
		stm << "\tGUI:\tEXE[(" << ClientId(rpt.clOrdId()) << "," << std::setw(6) << ClientId(rpt.origClOrdId()) << ")-->" << rpt.orderId() << "] " 
			<< std::setw(16) << ExecType::c_str(rpt.execType()) << " / " << std::left << std::setw(16) << OrdStatus::c_str(rpt.ordStatus())
			<< " [" << std::setw(5) << rpt.qty() << " cq=" << std::setw(5) << rpt.cumQty() << " lq=" << std::setw(5) << rpt.leavesQty() << " wq=" << std::setw(5) << rpt.workingQty() << "]";
		std::cout << stm.str() << std::endl;
		execs[rpt.clOrdId()] = rpt;
	}
	void OnMsg(const OrderCancelReject& rpt) override {
		stringstream strm;
		strm << "\tGUI:\tRJT[(" << ClientId(rpt.clOrdId()) << "," << std::setw(6) << ClientId(rpt.origClOrdId()) << ")-->" << rpt.orderId() << "] "
			<< std::setw(16) << "" << " / " << std::left << std::setw(16) << OrdStatus::c_str(rpt.status());
		std::cout << strm.str() << std::endl;
		rjt = rpt;
	}
	void OnMsg(const DontKnowTrade& rpt) override {
		// Never receives it
	}

};