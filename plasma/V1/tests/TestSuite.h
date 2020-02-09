#pragma once
#include <plasma.h>
#include "GUI.h"
#include "MLS.h"
#include "EPA.h"
#include <gtest/gtest.h>
#include <stack>

struct TestSuite : public testing::Test
{
	plasma::OMS oms;
	GUI gui;
	EPA epa;
	// register
	TestSuite() { oms.OnLogin(gui); oms.OnLogin(epa); }
};
