#include <TestSuite.h>
#include <TestSuiteN.h>

///////////////////////////////////////////////////////////////
//
TEST_F(TestSuite, SIZE) {
	std::cout << "sizeof(Order) " << sizeof(Order) << std::endl << std::endl;
	std::cout << "sizeof(OrderV2) " << sizeof(OrderV2) << std::endl << std::endl;
}
///////////////////////////////////////////////////////////////
//  NewOrderSingle	(X)
//  Reject			(X)
//	https://www.onixs.biz/fix-dictionary/4.4/app_dA.1.a.html
TEST_F(TestSuite, nos_rjt) {
	NewOrderReq idX(oms, gui, epa, 10000);
	idX.reject();
}
