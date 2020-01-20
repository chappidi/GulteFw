#pragma once

#include <iostream>
#include <unordered_map>
#include "ICallback.h"
#include <sstream>
#include <cassert>
using namespace std;
using namespace plasma::client;

namespace plasma
{
	class OMS
	{
		unordered_map<uint8_t, ICallback*> _out_os;
	public:
		void OnMsg(const NewOrderSingle& req);
		void OnMsg(const OrderStatusRequest& req);
		void OnMsg(const OrderCancelRequest& req);
		void OnMsg(const OrderReplaceRequest& req);
		void OnMsg(const NonFillReport& rpt);
		void OnMsg(const FillReport& rpt);
		void OnMsg(const OrderCancelReject& rpt);
		void OnMsg(const DontKnowTrade& rpt);

		void OnLogin(ICallback& cb);
		void OnLogout(ICallback& cb);
	};
}
