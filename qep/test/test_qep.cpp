#include <gtest/gtest.h>
#include <tsc.h>

struct TestQEP : public testing::Test
{

};

std::ostream& operator << (std::ostream& os, std::chrono::time_point<std::chrono::system_clock>& tp) {
	using namespace std::chrono;
	std::time_t t = system_clock::to_time_t(tp);
	auto tm = *std::localtime(&t);
	//	std::cout << std::ctime(&t) << std::endl; // for example : Tue Sep 27 14:21:13 2011
	auto us = duration_cast<std::chrono::microseconds>(tp.time_since_epoch()).count() % 1'000'000;
	os << std::put_time(&tm, "%F %T.") << us << std::put_time(&tm, " %Z\n") << std::endl;
	return os;
}
/*
	std::chrono::system_clock		starting point is the 1.1.1970, the so called UNIX-epoch.  (resolution 100 nanosec)
	std::chrono::steady_clock		starting point is the boot of your PC. ( resolution in nanosecs)
	tsc::steady_clock				starting point is the boot iof your PC. ( resolution cpu cycles)
	std::chrono::steady_clock	=	tsc::steady_clock / (cpu freq in GHzs)
*/
///////////////////////////////////////////////////////////
// Calculate the cost of calling std::chrono::system::clock
// sytem clock resolution: (1 / 100th nanosec)
TEST_F(TestQEP, system_clock) {
	using namespace std::chrono;
	// warmup
	system_clock::now();
	std::this_thread::sleep_for(2s);
	// snap
	auto start = tsc::steady_clock::tick();
	auto tp = system_clock::now();
	auto end = tsc::steady_clock::tick();
	// cost calculation
	std::cout << "cost " << (end - start) << " ticks" << std::endl << std::endl;
	std::cout << "raw:" << tp.time_since_epoch().count() << std::endl;
	std::cout << "ns :" << duration_cast<nanoseconds>(tp.time_since_epoch()).count() << std::endl;
	std::cout << "us :" << duration_cast<microseconds>(tp.time_since_epoch()).count() << std::endl;
	std::cout << "ms :" << duration_cast<milliseconds>(tp.time_since_epoch()).count() << std::endl;
	std::cout << "tt :" << system_clock::to_time_t(tp) << std::endl;
	std::cout << "sec:" << duration_cast<seconds>(tp.time_since_epoch()).count() << std::endl;
	// print time stamp upto microsec resolution
	std::cout << tp << std::endl;
}
///////////////////////////////////////////////////////////
//
TEST_F(TestQEP, PRINT) {
	using namespace std::chrono;
	auto csc = steady_clock::now();
	auto tsc = tsc::steady_clock::tick();
	auto ctp = system_clock::now(); 

	std::cout << steady_clock::period::num << " / " << steady_clock::period::den << std::endl;
	std::cout << system_clock::period::num << " / " << system_clock::period::den << std::endl;

	std::cout << csc.time_since_epoch().count() << " / " << tsc << " / " << ctp.time_since_epoch().count() << std::endl;
}
///////////////////////////////////////////////////////////
//
TEST_F(TestQEP, TSC) {
	auto test_single_sys = []() {
		auto start = tsc::steady_clock::tick();
		auto tmp = std::chrono::system_clock::now();
		auto end = tsc::steady_clock::tick();
		return (end - start);
	};
	auto test_single_hrc = []() {
		auto start = tsc::steady_clock::tick();
		auto tmp = std::chrono::steady_clock::now();
		auto end = tsc::steady_clock::tick();
		return (end - start);
	};
	auto tsc_single_sys = []() {
		auto start = tsc::steady_clock::tick();
		auto tmp = tsc::system_clock::now();
		auto end = tsc::steady_clock::tick();
		return (end - start);
	};

	auto test_single_tsc = []() {
		auto start = tsc::steady_clock::tick();
		auto tmp = tsc::steady_clock::tick();
		auto end = tsc::steady_clock::tick();
		return (end - start);
	};

	for (int itr = 0; itr < 10; itr++) {
		auto x2 = test_single_hrc();
		auto x4 = tsc_single_sys();
		auto x1 = test_single_sys();
		auto x3 = test_single_tsc();
		std::cout << "test " << x1 << " / " << x2 << " / " << x3 << " / " << x4 << std::endl;
	}
}