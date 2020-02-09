#pragma once
#include <GUI.h>
#include <MLS.h>
#include <EPA.h>
#include <plasma.h>
#include <Thor.h>
#include <gtest/gtest.h>

/////////////////////////////////////////////////////////////////////////
//
//
struct TestSuite : public testing::Test
{
	plasma::OMS oms;
	GUI gui;
	EPA epa;
	// register
	TestSuite() { 
		oms.OnLogin(gui); 
		oms.OnLogin(epa); 
	}
	auto new_order(double qty) {
		return 0;
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
		return 0;
	}
};
