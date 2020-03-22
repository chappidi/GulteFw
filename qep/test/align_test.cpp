#include <gtest/gtest.h>
#include <tsc.h>
#include <thread>
#include <atomic>

struct AlignTest : public testing::Test
{
	const double cpu_freq = 2994370145;
};
namespace NON_ATOMIC {
	struct ABCD {
		int k;
		int x;
		int y;
		int z;
		int d;
	};

	struct XYZ {
		alignas(64) int k;
		alignas(64) int x;
		alignas(64) int y;
		alignas(64) int z;
		alignas(64) int d;
	};
}
namespace ATOMIC {
	struct ABCD {
		std::atomic<int> k;
		std::atomic<int> x;
		std::atomic<int> y;
		std::atomic<int> z;
		std::atomic<int> d;
	};

	struct XYZ {
		alignas(64) std::atomic<int> k;
		alignas(64) std::atomic<int> x;
		alignas(64) std::atomic<int> y;
		alignas(64) std::atomic<int> z;
		alignas(64) std::atomic<int> d;
	};
}

TEST_F(AlignTest, align_struct) 
{

	std::cout << "sizeof(ABCD)" << sizeof(ATOMIC::ABCD) << std::endl;
	std::cout << "sizeof(XYZ)" << sizeof(ATOMIC::XYZ) << std::endl;
}
TEST_F(AlignTest, non_perf_test) {

	NON_ATOMIC::ABCD val;
	auto cb = [](int& v) {
		const int LOOP_SIZE = 50000000;
		for (int i = 0; i < LOOP_SIZE; ++i) {
			v++;
		}
	};
	constexpr int COUNT = 5;
	std::thread thrds[COUNT];
	auto begin = __rdtsc();
	thrds[0] = std::thread(cb, std::ref(val.k));
	thrds[1] = std::thread(cb, std::ref(val.x));
	thrds[2] = std::thread(cb, std::ref(val.y));
	thrds[3] = std::thread(cb, std::ref(val.z));
	thrds[4] = std::thread(cb, std::ref(val.d));
	for (int itr = 0; itr < COUNT; itr++) {
		thrds[itr].join();
	}
	auto end = __rdtsc();
	std::cout << "THREAD " << (end - begin) / cpu_freq << std::endl;
}

TEST_F(AlignTest, non_perf_test_2) {

	NON_ATOMIC::XYZ val;
	auto cb = [](int& v) {
		const int LOOP_SIZE = 50000000;
		for (int i = 0; i < LOOP_SIZE; ++i) {
			v++;
		}
	};
	constexpr int COUNT = 5;
	std::thread thrds[COUNT];
	auto begin = __rdtsc();
	thrds[0] = std::thread(cb, std::ref(val.k));
	thrds[1] = std::thread(cb, std::ref(val.x));
	thrds[2] = std::thread(cb, std::ref(val.y));
	thrds[3] = std::thread(cb, std::ref(val.z));
	thrds[4] = std::thread(cb, std::ref(val.d));
	for (int itr = 0; itr < COUNT; itr++) {
		thrds[itr].join();
	}
	auto end = __rdtsc();
	std::cout << "THREAD " << (end - begin) / cpu_freq << std::endl;
}
TEST_F(AlignTest, perf_test) {

	ATOMIC::ABCD val;
	auto cb = [](std::atomic<int>& v) {
		const int LOOP_SIZE = 50000000;
		for (int i = 0; i < LOOP_SIZE; ++i) {
			v++;
		}
	};
	constexpr int COUNT = 5;
	std::thread thrds[COUNT];
	auto begin = __rdtsc();
	thrds[0] = std::thread(cb, std::ref(val.k));
	thrds[1] = std::thread(cb, std::ref(val.x));
	thrds[2] = std::thread(cb, std::ref(val.y));
	thrds[3] = std::thread(cb, std::ref(val.z));
	thrds[4] = std::thread(cb, std::ref(val.d));
	for (int itr = 0; itr < COUNT; itr++) {
		thrds[itr].join();
	}
	auto end = __rdtsc();
	std::cout << "THREAD " << (end - begin) / cpu_freq << std::endl;
}

TEST_F(AlignTest, perf_test_2) {

	ATOMIC::XYZ val;
	auto cb = [](std::atomic<int>& v) {
		const int LOOP_SIZE = 50000000;
		for (int i = 0; i < LOOP_SIZE; ++i) {
			v++;
		}
	};
	constexpr int COUNT = 5;
	std::thread thrds[COUNT];
	auto begin = __rdtsc();
	thrds[0] = std::thread(cb, std::ref(val.k));
	thrds[1] = std::thread(cb, std::ref(val.x));
	thrds[2] = std::thread(cb, std::ref(val.y));
	thrds[3] = std::thread(cb, std::ref(val.z));
	thrds[4] = std::thread(cb, std::ref(val.d));
	for (int itr = 0; itr < COUNT; itr++) {
		thrds[itr].join();
	}
	auto end = __rdtsc();
	std::cout << "THREAD " << (end - begin) / cpu_freq << std::endl;
}
TEST_F(AlignTest, perf_test_3) {

	ATOMIC::XYZ val;
	auto cb = [](std::atomic<int>& v) {
		const int LOOP_SIZE = 50000000;
		for (int i = 0; i < LOOP_SIZE; ++i) {
			v.fetch_add(1, std::memory_order_relaxed);
		}
	};
	constexpr int COUNT = 5;
	std::thread thrds[COUNT];
	auto begin = __rdtsc();
	thrds[0] = std::thread(cb, std::ref(val.k));
	thrds[1] = std::thread(cb, std::ref(val.x));
	thrds[2] = std::thread(cb, std::ref(val.y));
	thrds[3] = std::thread(cb, std::ref(val.z));
	thrds[4] = std::thread(cb, std::ref(val.d));
	for (int itr = 0; itr < COUNT; itr++) {
		thrds[itr].join();
	}
	auto end = __rdtsc();
	std::cout << "THREAD " << (end - begin) / cpu_freq << std::endl;
}