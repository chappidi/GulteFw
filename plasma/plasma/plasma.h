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
using namespace plasma::client;
namespace plasma
{
	class OMS
	{
	public:
		void OnMsg(const NewOrderSingle& req) {
		}
		void OnMsg(const OrderStatusRequest& req) {
		}
		void OnMsg(const OrderCancelRequest& req) {
		}
		void OnMsg(const OrderReplaceRequest& req) {
		}
		void OnMsg(const NonFillReport& rpt) {
		}
		void OnMsg(const FillReport& rpt) {
		}
		void OnMsg(const OrderCancelReject& rpt) {
		}
		void OnMsg(const DontKnowTrade& rpt) {

		}
		void OnLogin(ICallback& cb) {
		}
		void OnLogout(ICallback& cb) {
		}
	};
}
