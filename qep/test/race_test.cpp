#include <gtest/gtest.h>
#include <tsc.h>
#include <thread>
#include <atomic>

struct RaceTest : public testing::Test
{
	const double cpu_freq = 2994370145;
};

TEST_F(RaceTest, race_test_1) {

	constexpr int LOOP_SIZE = 10000000;
	int val = 0;
	auto cb = [LOOP_SIZE](int& v) {
		for (int i = 0; i < LOOP_SIZE; ++i) {
			v++;
		}
	};
	constexpr int COUNT = 5;
	std::thread thrds[COUNT];
	auto begin = __rdtsc();
	for (int itr = 0; itr < COUNT; itr++) {
		thrds[itr] = std::thread(cb, std::ref(val));
	}
	for (int itr = 0; itr < COUNT; itr++) {
		thrds[itr].join();
	}
	auto end = __rdtsc();
	std::cout << "COUNT " << LOOP_SIZE *5  << " / " << val << std::endl;
	std::cout << "THREAD " << (end - begin) / cpu_freq << std::endl;
}
TEST_F(RaceTest, race_test_2) {

	constexpr int LOOP_SIZE = 10000000;
	std::atomic<int> val{ 0 };
	auto cb = [LOOP_SIZE](std::atomic<int>& v) {
		for (int i = 0; i < LOOP_SIZE; ++i) {
			v++;
		}
	};
	constexpr int COUNT = 5;
	std::thread thrds[COUNT];
	auto begin = __rdtsc();
	for (int itr = 0; itr < COUNT; itr++) {
		thrds[itr] = std::thread(cb, std::ref(val));
	}
	for (int itr = 0; itr < COUNT; itr++) {
		thrds[itr].join();
	}
	auto end = __rdtsc();
	std::cout << "COUNT " << LOOP_SIZE * 5 << " / " << val << std::endl;
	std::cout << "THREAD " << (end - begin) / cpu_freq << std::endl;
}
TEST_F(RaceTest, race_test_3) {

	constexpr int LOOP_SIZE = 10000000;
	std::atomic<int> val{ 0 };
	auto cb = [LOOP_SIZE](std::atomic<int>& v) {
		for (int i = 0; i < LOOP_SIZE; ++i) {
			v += 1;
		}
	};
	constexpr int COUNT = 5;
	std::thread thrds[COUNT];
	auto begin = __rdtsc();
	for (int itr = 0; itr < COUNT; itr++) {
		thrds[itr] = std::thread(cb, std::ref(val));
	}
	for (int itr = 0; itr < COUNT; itr++) {
		thrds[itr].join();
	}
	auto end = __rdtsc();
	std::cout << "COUNT " << LOOP_SIZE * 5 << " / " << val << std::endl;
	std::cout << "THREAD " << (end - begin) / cpu_freq << std::endl;
}
TEST_F(RaceTest, race_test_4) {

	constexpr int LOOP_SIZE = 10000000;
	std::atomic<int> val{ 0 };
	auto cb = [LOOP_SIZE](std::atomic<int>& v) {
		for (int i = 0; i < LOOP_SIZE; ++i) {
			v = v + 1;
		}
	};
	constexpr int COUNT = 5;
	std::thread thrds[COUNT];
	auto begin = __rdtsc();
	for (int itr = 0; itr < COUNT; itr++) {
		thrds[itr] = std::thread(cb, std::ref(val));
	}
	for (int itr = 0; itr < COUNT; itr++) {
		thrds[itr].join();
	}
	auto end = __rdtsc();
	std::cout << "COUNT " << LOOP_SIZE * 5 << " / " << val << std::endl;
	std::cout << "THREAD " << (end - begin) / cpu_freq << std::endl;
}

