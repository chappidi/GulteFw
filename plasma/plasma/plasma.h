#pragma once
#include <plasma_client/NewOrderSingle.h>
#include <plasma_client/OrderCancelRequest.h>
#include <plasma_client/OrderReplaceRequest.h>
#include <plasma_client/OrderStatusRequest.h>
#include <plasma_client/OrderCancelReject.h>
#include <plasma_client/NonFillReport.h>
#include <plasma_client/FillReport.h>
#include <plasma_client/DontKnowTrade.h>
#include <iostream>
#include "ICallback.h"
#include <sstream>
#include <cassert>
using namespace std;
using namespace plasma::client;
struct ClientId {
	uint32_t _id;
	ClientId(uint32_t id) :_id(id) {	}
	inline uint8_t instance() const { return _id >> 24; }
	inline uint32_t seq_no() const { return _id & 0x00FFFFFF; }
};
static ostream& operator << (ostream& out, const ClientId& id) {
	out << (int)id.instance() << "." << id.seq_no();
	return out;
}
namespace plasma
{
	class OMS
	{
	public:
		void OnMsg(const NewOrderSingle& req) {
			stringstream strm;
			strm << "PLS:\t\tNOS[" << ClientId(req.clOrdId()) << "]";
			std::cout << strm.str() << std::endl;
		}
		void OnMsg(const OrderStatusRequest& req) {
			stringstream strm;
			strm << "PLS:\t\tOSR[" << ClientId(req.clOrdId()) << " / " << req.orderId() << "]";
			std::cout << strm.str() << std::endl;
		}
		void OnMsg(const OrderCancelRequest& req) {
			stringstream strm;
			strm << "PLS:\t\tOCR[" << ClientId(req.clOrdId()) << "/" << ClientId(req.origClOrdId()) << "/" << req.orderId() << "]";
			std::cout << strm.str() << std::endl;
		}
		void OnMsg(const OrderReplaceRequest& req) {
			stringstream strm;
			strm << "PLS:\t\tORR[" << ClientId(req.clOrdId()) << "/" << ClientId(req.origClOrdId()) << "/" << req.orderId() << "]";
			std::cout << strm.str() << std::endl;
		}
		void OnMsg(const NonFillReport& rpt) {
			stringstream strm;
			strm << "PLS:\t\tNTR[" << rpt.clOrdId() << "/" << rpt.origClOrdId() << "/" << rpt.orderId() << "]";
//			strm << toString(rpt._action) << " / " << OrdStatus::c_str(rpt.status());
			strm << "[" << rpt.qty() << "," << rpt.cumQty() << "," << rpt.leavesQty() << "]";
			std::cout << strm.str() << std::endl;
		}
		void OnMsg(const FillReport& rpt) {
			stringstream strm;
			strm << "PLS:\t\tTRD[" << rpt.clOrdId() << "/" << rpt.orderId() << "]";
			strm << OrdStatus::c_str(rpt.status());
			strm << "[" << rpt.qty() << "," << rpt.cumQty() << "," << rpt.leavesQty() << "]";
			std::cout << strm.str() << std::endl;
		}
		void OnMsg(const OrderCancelReject& rpt) {
			stringstream strm;
			strm << "PLS:\t\tRJT[" << rpt.clOrdId() << "/" << rpt.origClOrdId() << "/" << rpt.orderId() << "]";
			strm << OrdStatus::c_str(rpt.status());
			std::cout << strm.str() << std::endl;
		}
		void OnMsg(const DontKnowTrade& rpt) {
			stringstream strm;
			strm << "PLS:\t\tDKT[" << rpt.orderId() << "/" << rpt.execId() << "]";
			std::cout << strm.str() << std::endl;

		}
		void OnLogin(ICallback& cb) {
		}
		void OnLogout(ICallback& cb) {
		}
	};
}
