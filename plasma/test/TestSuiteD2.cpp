#include <plasma.h>
#include "GUI.h"
#include "EPA.h"
#include <gtest/gtest.h>

struct TestSuiteD2 : public testing::Test
{
	// declare variables
	plasma::OMS plasma;
	GUI clt;
	EPA epa;
	TestSuiteD2() {
		plasma.OnLogin(clt);
		plasma.OnLogin(epa);
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//  Ack				(X)
	//  Partial Fill	(X)
	//  Replace Request	(Y,X)
	//  Replace Request	(Z,Y)
	//	Pending Replace	(Y,X)
	//	Replaced		(Y,X)
	//	Pending Replace	(Z,Y)
	//	Replaced		(Z,Y)
	//  Partial Fill	(Z)
	// https://www.onixs.biz/fix-dictionary/4.4/app_dD.2.a.html
};