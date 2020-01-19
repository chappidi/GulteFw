#pragma once
#include <plasma_client/NewOrderSingle.h>
#include <plasma_client/OrderCancelRequest.h>
#include <plasma_client/OrderReplaceRequest.h>
#include <plasma_client/OrderStatusRequest.h>
#include <plasma_client/OrderCancelReject.h>
#include <plasma_client/NonFillReport.h>
#include <plasma_client/FillReport.h>
#include <plasma_client/DontKnowTrade.h>
namespace proxy {
	struct NewOrderSingle : public plasma::client::NewOrderSingle {
		NewOrderSingle(uint32_t unq_id, uint8_t tgt) {
			clOrdId(unq_id);
			target(tgt);
		}
	};
	struct OrderCancelRequest : public plasma::client::OrderCancelRequest {
		OrderCancelRequest(uint32_t unq_id, const NewOrderSingle& nos) {
			clOrdId(unq_id);
			origClOrdId(nos.clOrdId());

			symbol(nos.symbol());
			side(nos.side());
		}
	};
	struct OrderReplaceRequest : public plasma::client::OrderReplaceRequest
	{
		OrderReplaceRequest(uint32_t id, const NewOrderSingle& nos) {
			clOrdId(id);
			origClOrdId(nos.clOrdId());
			symbol(nos.symbol());
			side(nos.side());
		}
	};
	struct OrderStatusRequest : public plasma::client::OrderStatusRequest {
		OrderStatusRequest(const NewOrderSingle& nos) {
			clOrdId(nos.clOrdId());
			symbol(nos.symbol());
			side(nos.side());
			qty(nos.qty());
		}
		OrderStatusRequest(const OrderReplaceRequest& orr) {
			clOrdId(orr.clOrdId());
			symbol(orr.symbol());
			side(orr.side());
			qty(orr.qty());
		}
	};
}