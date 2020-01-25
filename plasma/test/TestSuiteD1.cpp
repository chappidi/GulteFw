#include <plasma.h>
#include "GUI.h"
#include "EPA.h"
#include <gtest/gtest.h>

struct TestSuiteD1 : public testing::Test
{
	// declare variables
	plasma::OMS plasma;
	GUI clt;
	EPA epa;
	TestSuiteD1() {
		plasma.OnLogin(clt);
		plasma.OnLogin(epa);
	}
	///////////////////////////////////////////////////////////////
	//  NewOrderSingle	(X)
	//  Ack				(X)
	//  Partial Fill	(X)
	//  Replace Request	(Y,X)
	//	Pending Replace	(Y,X)
	//  Partial Fill	(X)
	//	Replaced		(Y,X)
	//  Partial Fill	(Y)
	//  Replace Request	(Z,Y)
	//	Pending Replace	(Z,Y)
	//  Partial Fill	(Y)
	//	Replaced		(Z,Y)
	//  Partial Fill	(Z)
	// https://www.onixs.biz/fix-dictionary/4.4/app_dD.1.a.html

};