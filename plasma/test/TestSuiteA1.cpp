#include <gtest/gtest.h>
#include <plasma.h>
#include "GUI.h"
#include "EPA.h"
struct TestSuiteA1 : public testing::Test {

	// declare variables
	plasma::OMS plasma;
	GUI clt;
	EPA epa;
	TestSuiteA1() {
		plasma.OnLogin(clt);
		plasma.OnLogin(epa);
	}
};

TEST_F(TestSuiteA1, test_nos_rjt) {
	auto nos = clt.get_nos(epa.id(), 10000);
	std::cout << nos << std::endl;
	EXPECT_EQ(nos.symbol(), 9999);
}

TEST_F(TestSuiteA1, test_nos_pnd_new_rjt) {
	auto nos = clt.get_nos(epa.id(), 10000);
	std::cout << nos << std::endl;
	auto sts = clt.get_sts(nos);
	std::cout << sts << std::endl;
	EXPECT_EQ(nos.clOrdId(), sts.clOrdId());
	EXPECT_EQ(nos.symbol(), 9999);
}
