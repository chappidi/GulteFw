#pragma once
#include <plasma_client/NewOrderSingle.h>
#include <plasma_client/OrderCancelRequest.h>
#include <plasma_client/OrderReplaceRequest.h>
#include <plasma_client/OrderStatusRequest.h>
#include <plasma_client/OrderCancelReject.h>
#include <plasma_client/ExecutionReport.h>
#include <plasma_client/DontKnowTrade.h>
#include <iostream>
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
namespace plasma {
	struct ICallback {
		virtual uint8_t id() = 0;
		virtual void OnMsg(const NewOrderSingle& req) = 0;
		virtual void OnMsg(const OrderCancelRequest& req) = 0;
		virtual void OnMsg(const OrderReplaceRequest& req) = 0;
		virtual void OnMsg(const OrderStatusRequest& req) = 0;
		virtual void OnMsg(const ExecutionReport& rpt) = 0;
		virtual void OnMsg(const OrderCancelReject& rpt) = 0;
		virtual void OnMsg(const DontKnowTrade& rpt) = 0;
	};
}