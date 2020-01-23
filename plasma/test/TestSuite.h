#pragma once
#include <gtest/gtest.h>
#include <plasma.h>
#include "GUI.h"
#include "EPA.h"

struct TestSuite : public testing::Test
{
	// declare variables
	plasma::OMS plasma;
	GUI clt;
	EPA epa;
	TestSuite() {
		plasma.OnLogin(clt);
		plasma.OnLogin(epa);
	}
	void VALIDATE_PENDING_NEW(const NewOrderSingle& nos, int idX, const NonFillReport& nfr) {
		EXPECT_TRUE(nfr.status() == OrdStatus::Pending_New);
		EXPECT_TRUE(nfr.origClOrdId() == 0 && nfr.clOrdId() == nos.clOrdId() && nfr.orderId() == idX);
		EXPECT_TRUE(nfr.leavesQty() == 0 && nfr.cumQty() == 0 && nfr.avgPx() == 0);
	}
	void VALIDATE_NEW(const NewOrderSingle& nos, int idX, const NonFillReport& nfr) {
		EXPECT_TRUE(nfr.status() == OrdStatus::New);
		EXPECT_TRUE(nfr.origClOrdId() == 0 && nfr.clOrdId() == nos.clOrdId() && nfr.orderId() == idX);
		EXPECT_TRUE(nfr.leavesQty() == 0 && nfr.cumQty() == 0 && nfr.avgPx() == 0);
	}
	void VALIDATE_REJECT(const NewOrderSingle& nos, int idX, const NonFillReport& nfr) {
		EXPECT_TRUE(nfr.status() == OrdStatus::Rejected);
		EXPECT_TRUE(nfr.origClOrdId() == 0 && nfr.clOrdId() == nos.clOrdId() && nfr.orderId() == idX);
		EXPECT_TRUE(nfr.leavesQty() == 0 && nfr.cumQty() == 0 && nfr.avgPx() == 0);
	}
	void VALIDATE_FILL(const NewOrderSingle& nos, int idX, double lastQty, double lastPx, const FillReport& fill) {
		EXPECT_TRUE(fill.status() == OrdStatus::Partially_Filled);
		EXPECT_TRUE(fill.clOrdId() == nos.clOrdId() && fill.orderId() == idX);
		EXPECT_TRUE(fill.lastPx() == lastPx && fill.lastQty() == lastQty);
	}
	void VALIDATE_FILLED(const NewOrderSingle& nos, int idX, double lastQty, double lastPx, const FillReport& fill) {
		EXPECT_TRUE(fill.status() == OrdStatus::Filled);
		EXPECT_TRUE(fill.clOrdId() == nos.clOrdId() && fill.orderId() == idX);
		EXPECT_TRUE(fill.lastPx() == lastPx && fill.lastQty() == lastQty);
	}
};