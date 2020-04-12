#include <gtest/gtest.h>
#include <tsc.h>
#include <thread>
#include <atomic>

struct StackTest : public testing::Test
{
	const double cpu_freq = 2994370145;
};

struct ABC {
	char buf[100000];
	void write(char c) {
		for (int i = 0; i < 10000; i++)
			buf[i] = c;
	}
	void writex(char c) {
		char buf[100000];
		for (int i = 0; i < 10000; i++)
			buf[i] = c;
	}
};

TEST_F(StackTest, test_1) {
	constexpr int LOOP_SIZE = 10000000;
	auto cb = [LOOP_SIZE]() {
		ABC abc;
		for (int i = 0; i < LOOP_SIZE; ++i) {
			abc.writex((char)(i % 26 + 65));
		}
	};
	constexpr int COUNT = 10;
	std::thread thrds[COUNT];
	auto begin = __rdtsc();
	for (int itr = 0; itr < COUNT; itr++) {
		thrds[itr] = std::thread(cb);
	}
	for (int itr = 0; itr < COUNT; itr++) {
		thrds[itr].join();
	}
	auto end = __rdtsc();

	std::cout << "THREAD " << (end - begin)/ cpu_freq << std::endl;

}
TEST_F(StackTest, test_2) {
	constexpr int LOOP_SIZE = 10000000;
	auto cb = [LOOP_SIZE]() {
		ABC abc;
		for (int i = 0; i < LOOP_SIZE; ++i) {
			abc.write((char)(i % 26 + 65));
		}
	};
	constexpr int COUNT = 10;
	std::thread thrds[COUNT];
	auto begin = __rdtsc();
	for (int itr = 0; itr < COUNT; itr++) {
		thrds[itr] = std::thread(cb);
	}
	for (int itr = 0; itr < COUNT; itr++) {
		thrds[itr].join();
	}
	auto end = __rdtsc();

	std::cout << "THREAD " << (end - begin)/ cpu_freq << std::endl;

}
