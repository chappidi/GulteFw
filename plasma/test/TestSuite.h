#pragma once
#include <plasma.h>
#include "GUI.h"
#include "EPA.h"
#include <gtest/gtest.h>

struct TestSuite : public testing::Test
{
	// declare variables
	plasma::OMS plasma;
	GUI clt;
	EPA epa;
	PROXY<NewOrderSingle>		nos;
	PROXY<OrderStatusRequest>	osr;
	PROXY<OrderCancelRequest>	ocr;
	int32_t						idX{ 0 };
	double						_execQty{ 0 };
	OrdStatus::Value			_ordStatus{ OrdStatus::NA };
	TestSuite() {
		// register
		plasma.OnLogin(clt);
		plasma.OnLogin(epa);

		// create requests
		nos = clt.get_nos(epa.id(), 10000);
		osr = clt.get_sts(nos);
		ocr = clt.get_cxl(idX, nos);
	}
};
