#include <gtest/gtest.h>
#include <boost/fiber/all.hpp>
#include <boost/fiber/numa/topology.hpp>

struct FiberTest : public testing::Test
{
};
TEST_F(FiberTest, numa_factor)
{
	std::vector<boost::fibers::numa::node> topo = boost::fibers::numa::topology();
	for (auto n : topo) {
		std::cout << "node: " << n.id << "|";
		std::cout << "cpus: ";
		for (auto cpu_id : n.logical_cpus) {
			std::cout << cpu_id << " ";
		}
		std::cout << "| distance: ";
		for (auto d : n.distance) {
			std::cout << d << " ";
		}
		std::cout << std::endl;
	}
	std::cout << "NUMA" << std::endl;
}
TEST_F(FiberTest, thread_test) {
	const double cpu_freq = 2994370145;
	auto cb = []() {
		const int LOOP_SIZE = 5000000;
		for (int i = 0; i < LOOP_SIZE; ++i) {
			std::this_thread::yield();
		}
	};
	constexpr int COUNT = 100;
	std::thread thrds[COUNT];
	auto begin = __rdtsc();
	for (int itr = 0; itr < COUNT; itr++) {
		thrds[itr] = std::thread(cb);
	}
	for (int itr = 0; itr < COUNT; itr++) {
		thrds[itr].join();
	}
	auto end = __rdtsc();
	std::cout << std::thread::hardware_concurrency() << "\tconcurrent threads are supported" << std::endl;
	
	std::cout << "THREAD " << (end - begin)/ cpu_freq << std::endl;
}
TEST_F(FiberTest, fiber_test) {
	const double cpu_freq = 2994370145;
	auto cb = []() {
		const int LOOP_SIZE = 5000000;
		for (int i = 0; i < LOOP_SIZE; ++i) {
			boost::this_fiber::yield();
		}
	};
	constexpr int COUNT = 100;
	boost::fibers::fiber fbr[COUNT];
	auto begin = __rdtsc();
	for (int itr = 0; itr < COUNT; itr++) {
		fbr[itr] = boost::fibers::fiber(cb);
	}
	for (int itr = 0; itr < COUNT; itr++) {
		fbr[itr].join();
	}
	auto end = __rdtsc();
	std::cout << std::thread::hardware_concurrency() << "\tconcurrent threads are supported" << std::endl;

	std::cout << "FIBER " << (end - begin) / cpu_freq << std::endl;
}