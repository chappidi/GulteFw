#include <gtest/gtest.h>
#define _ENABLE_ATOMIC_ALIGNMENT_FIX
#include <tsc.h>
#include <thread>
#include <atomic>

struct LockTest : public testing::Test
{
	const double cpu_freq = 2994370145;
};
struct A { long x; };
struct B { long x; long y; };
struct C { long x; long y; long z; };
TEST_F(LockTest, race_test_1) {
	std::atomic<A> a;
	std::atomic<B> b;
	std::atomic<C> c;
	std::cout << "A " << a.is_lock_free() << std::endl;
	std::cout << "B " << b.is_lock_free() << std::endl;
	std::cout << "C " << c.is_lock_free() << std::endl;
}
/*
std::memory_order_relaxed  :: no guarantees
std::memory_order_acquire	:: after barrier operations stays after barrier
std::memory_order_release   :: before barrier operations stay before barrier

std::memory_order_seq_cst	:: highest and slowest (across multiple atomic variable)


	memory_order_relaxed,
	memory_order_consume,
	memory_order_acquire,
	memory_order_release,
	memory_order_acq_rel,
	memory_order_seq_cst
*/
TEST_F(LockTest, test_2) {
	constexpr int LOOP_SIZE = 10000000;
	alignas(64) int count{ 0 };
	char* queue = new char[LOOP_SIZE];
	alignas(64) std::atomic<int> front{ 0 };
	auto cb = [LOOP_SIZE, &queue, &count](std::atomic<int>& v) {
		while (v.load() < LOOP_SIZE) {
			int idx = v.fetch_add(1);
			if (idx < LOOP_SIZE) {
				queue[idx] = idx % 10;
				count++;
			}
		}
	};
	constexpr int COUNT = 10;
	std::thread thrds[COUNT];
	auto begin = __rdtsc();
	for (int itr = 0; itr < COUNT; itr++) {
		thrds[itr] = std::thread(cb, std::ref(front));
	}
	for (int itr = 0; itr < COUNT; itr++) {
		thrds[itr].join();
	}
	auto end = __rdtsc();
	int error{ 0 };
	for (int itr = 0; itr < LOOP_SIZE; itr++) {
		if (queue[itr] != itr % 10)
			error++;
	}
	std::cout << "THREAD " << (end - begin) / cpu_freq << std::endl;
	std::cout << "COUNT " << error << " / " << count << std::endl;
}

