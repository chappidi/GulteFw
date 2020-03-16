#include <gtest/gtest.h>
#include <tsc.h>
#include <thread>

struct SpeedTest : public testing::Test
{
};
///////////////////////////////////////////////////////////
// If the machine sleeps __rdtsc  will not increment
// whereas std::chrono::steady_clock::now() will increment
TEST_F(SpeedTest, cpu_factor) 
{
	{
		// this will not work because if u let the machine sleep it skews the ratio 
		// && at start std::chrono::steady_clock::now() !=  tsc::steady_clock::tick() != 0
		auto x1 = std::chrono::steady_clock::now();
		double x2 = tsc::steady_clock::tick();
		std::cout << (uint64_t)x2 << " / " << x1.time_since_epoch().count() << std::endl;
		std::cout << "CPU FREQ " << x2 / x1.time_since_epoch().count() << std::endl;
	}
	std::cout << "CPU FREQ " << (uint64_t)tsc::cpu_info::intel() << std::endl;
	std::cout << std::endl;
	for (int i = 0; i < 10; i++) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		std::cout << "CPU FREQ " << (uint64_t)tsc::cpu_info::freq() << std::endl;
	}
}
///////////////////////////////////////////////////////////
//
TEST_F(SpeedTest, speed_system_clock) {
	const int LOOP_SIZE = 5000000;
	std::cout << "Running for 5 seconds..." << std::endl;

	auto begin = std::chrono::system_clock::now();
	auto last = begin;

	long duration = 0;
	long counter = 0;
	long unique = 0;
	while (duration < 5000000) {
		auto end = std::chrono::system_clock::now();
		if (last != end) {
			unique++;
			last = end;
		}
		counter++;
		duration = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
	}
	auto end = std::chrono::system_clock::now();
	duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
	std::cout << "iterations :" << counter
		<< "\n\tunique values :" << unique
		<< "\n\tratio :" << (counter * 1.0)/unique
		<< "\n\ttime taken (ms): " << duration << std::endl;
}
///////////////////////////////////////////////////////////
//
TEST_F(SpeedTest, speed_tsc_system_clock) {
	// get cpu freq
	std::this_thread::sleep_for(std::chrono::seconds(1));
//	const uint64_t cpu_freq = 2994370145;
	auto cpu_freq = tsc::cpu_info::freq();

	const int LOOP_SIZE = 5000000;
	std::cout << "Running for 5 seconds..." << std::endl;

	// ticks in cpu cycles
	auto begin = tsc::system_clock::now(cpu_freq);
	auto last = begin;

	long duration = 0;
	long counter = 0;
	long unique = 0;
	while (duration < 5000000) {
		// ticks in cpu cycles
		auto end = tsc::system_clock::now(cpu_freq);
		if (last != end) {
			unique++;
			last = end;
		}
		counter++;
		duration = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
	}
	auto end = tsc::system_clock::now(cpu_freq);
	duration = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
	std::cout << "iterations :" << counter
		<< "\n\tunique values :" << unique
		<< "\n\tratio :" << (counter * 1.0) / unique
		<< "\n\ttime taken (ms): " << duration << std::endl;
}

///////////////////////////////////////////////////////////
//
TEST_F(SpeedTest, speed_steady_clock) {
	const int LOOP_SIZE = 5000000;
	std::cout << "Running for 5 seconds..." << std::endl;

	auto begin = std::chrono::steady_clock::now();
	auto last = begin;

	long duration = 0;
	long counter = 0;
	long unique = 0;
	while (duration < 5000000) {
		auto end = std::chrono::steady_clock::now();
		if (last != end) {
			unique++;
			last = end;
		}
		counter++;
		duration = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
	}
	auto end = std::chrono::steady_clock::now();
	duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
	std::cout << "iterations :" << counter
		<< "\n\tunique values :" << unique
		<< "\n\tratio :" << (counter * 1.0) / unique
		<< "\n\ttime taken (ms): " << duration << std::endl;
}
///////////////////////////////////////////////////////////
//
TEST_F(SpeedTest, speed_tsc_steady_clock) {
	// get cpu freq
	std::this_thread::sleep_for(std::chrono::seconds(1));
	auto cpu_freq = tsc::cpu_info::freq();

	const int LOOP_SIZE = 5000000;
	std::cout << "Running for 5 seconds..." << std::endl;

	// ticks in cpu cycles
	auto begin = tsc::steady_clock::tick();
	auto last = begin;

	long long duration = 0;
	long counter = 0;
	long unique = 0;
	while (duration < 5000000) {
		// ticks in cpu cycles
		auto end = tsc::steady_clock::tick();
		if (last != end) {
			unique++;
			last = end;
		}
		counter++;
		duration = tsc::duration_cast<std::chrono::microseconds>(end - begin, cpu_freq);
	}
	auto end = tsc::steady_clock::tick();
	duration = tsc::duration_cast<std::chrono::microseconds>(end - begin, cpu_freq);
	//std::cout << (end - begin) << " / " << cpu_freq << std::endl;
	//using CF = typename std::chrono::nanoseconds::period;
	//std::cout << CF::num << " / " << CF::den << std::endl;

	std::cout << "iterations :" << counter
		<< "\n\tunique values :" << unique
		<< "\n\tratio :" << (counter * 1.0) / unique
		<< "\n\ttime taken (ms): " << duration << std::endl;
}
