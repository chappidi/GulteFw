#pragma once
#include <GUI.h>
#include <MLS.h>
#include <EPA.h>
#include <Thor.h>
#include <gtest/gtest.h>
#include <ReqState.h>

/////////////////////////////////////////////////////////////////////////
//
class ParentNewOrderReq : public NewOrderReq {
	ITarget& epa;
public:
	ParentNewOrderReq(plasma::ICallback& pls, ISource& sr, ITarget& ep, ITarget& tg, double qty)
		:NewOrderReq(pls, sr, tg, qty), epa(ep)
	{

	}
	void fill(double qty, double px = 99.98) {
		auto ch = slice_order(epa, qty);
		ch.accept();
		ch.fill(qty, px);
		ch.done();
	}
};
/////////////////////////////////////////////////////////////////////////
//
//
struct TestSuiteV2 : public testing::Test
{
	plasma::OMS_V2 oms;
	GUI gui;
	MLS mls;
	EPA epa;
	// register
	TestSuiteV2() { 
		oms.OnLogin(gui); 
		oms.OnLogin(mls); 
		oms.OnLogin(epa); 
	}
	auto new_order(ITarget& tgt, double qty) {
		return ParentNewOrderReq(oms, gui, epa, tgt, qty);
	}
};
