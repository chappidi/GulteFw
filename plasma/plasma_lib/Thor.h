#pragma once
#pragma once
#include "ICallback.h"
#include "OrderV2.h"
#include <iostream>
#include <sstream>
#include <cassert>
#include <vector>
#include <map>
#include <unordered_map>
using namespace std;
using namespace plasma::client;

namespace plasma
{

	class OMS_V2 final : public ICallback
	{
		struct XYZ {
			ICallback* _cb;
			map<uint32_t, uint32_t > _clnt2oms;
		};
		unordered_map<uint8_t, XYZ> _out_os;
		vector<OrderV2*> _orders;
		OrderV2* lookup(uint32_t orderId, uint32_t clOrdId);
		void get_status(ExecutionReport& rpt, const OrderV2& req);
	public:
		uint8_t id() { return 0; }
		OMS_V2() {
			// dummy order.
			_orders.push_back(nullptr);
		}
		void OnMsg(const NewOrderSingle& req);
		void OnMsg(const OrderCancelRequest& req);
		void OnMsg(const OrderReplaceRequest& req);
		void OnMsg(const OrderStatusRequest& req);

		void OnMsg(const ExecutionReport& rpt);
		void OnMsg(const OrderCancelReject& rpt);
		void OnMsg(const DontKnowTrade& rpt);

		void OnLogin(ICallback& cb);
		void OnLogout(ICallback& cb);
	};
}
