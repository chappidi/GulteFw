#pragma once

using namespace plasma::client;
namespace plasma {
	struct ICallback {
		virtual uint8_t id() = 0;
		virtual void OnMsg(const NewOrderSingle& req) = 0;
		virtual void OnMsg(const OrderCancelRequest& req) = 0;
		virtual void OnMsg(const OrderReplaceRequest& req) = 0;
		virtual void OnMsg(const OrderStatusRequest& req) = 0;
//		virtual void OnMsg(const ExecutionReport& rpt) = 0;
		virtual void OnMsg(const NonFillReport& rpt) = 0;
		virtual void OnMsg(const FillReport& rpt) = 0;
		virtual void OnMsg(const OrderCancelReject& rpt) = 0;
		virtual void OnMsg(const DontKnowTrade& rpt) = 0;
	};
}