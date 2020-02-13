#pragma once
#include <plasma_client/NewOrderSingle.h>
#include <plasma_client/OrderCancelRequest.h>
#include <plasma_client/OrderReplaceRequest.h>
#include <plasma_client/OrderStatusRequest.h>

using namespace plasma::client;
/////////////////////////////////////////////////////////////////////////
// translate functions
/*
static OrderStatusRequest& operator << (OrderStatusRequest& osr, const NewOrderSingle& req)
{
	osr.clOrdId(req.clOrdId())
		.symbol(req.symbol())
		.side(req.side())
		.qty(req.qty());
	return osr;
}
*/
static OrderStatusRequest& operator << (OrderStatusRequest& osr, const OrderReplaceRequest& req)
{
	osr.clOrdId(req.clOrdId())
		.symbol(req.symbol())
		.side(req.side())
		.qty(req.qty());
	return osr;
}
/*
static OrderStatusRequest& operator << (OrderStatusRequest& osr, const OrderCancelRequest& req)
{
	osr.clOrdId(req.clOrdId())
		.symbol(req.symbol())
		.side(req.side())
		.qty(req.qty());
	return osr;
}
*/