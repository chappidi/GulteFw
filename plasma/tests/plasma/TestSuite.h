#pragma once
#include <GUI.h>
#include <MLS.h>
#include <EPA.h>
#include <plasma.h>
#include <gtest/gtest.h>
#include <ReqState.h>

/////////////////////////////////////////////////////////////////////////
//
//
struct TestSuite : public testing::Test
{
	const string reason = "";
	plasma::OMS oms;
	GUI gui;
	EPA epa;
	// register
	TestSuite() { 
		oms.OnLogin(gui); 
		oms.OnLogin(epa); 
	}
	auto new_order(double qty) {
		return NewOrderReq(oms, gui, epa, qty);
	}
};
