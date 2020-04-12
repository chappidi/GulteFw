#include <gtest/gtest.h>
#include <tsc.h>
#include <thread>
#include <atomic>

struct ExceptionTest : public testing::Test
{
	const double cpu_freq = 2994370145;
};

struct ABC {
	void write(char* l) {
		if (l == nullptr)
			throw std::exception();
	}
	void writex(char* l) {
		if (l != nullptr) {

		}
	}
};
TEST_F(ExceptionTest, test_1) {
	constexpr int LOOP_SIZE = 10000000;
	auto cb = [LOOP_SIZE]() {
		ABC abc;
		for (int i = 0; i < LOOP_SIZE; ++i) {
			abc.writex("D");
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

	std::cout << "THREAD " << (end - begin) << std::endl;

}
TEST_F(ExceptionTest, test_2) {
	constexpr int LOOP_SIZE = 10000000;
	auto cb = [LOOP_SIZE]() {
		ABC abc;
		try {
			for (int i = 0; i < LOOP_SIZE; ++i) {
				abc.write("D");
			}
		}
		catch (std::exception & ex) {

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
	std::cout << "THREAD " << (end - begin) << std::endl;
}

TEST_F(ExceptionTest, test_3) {
	constexpr int LOOP_SIZE = 10000000;
	auto cb = [LOOP_SIZE]() {
		ABC abc;
		for (int i = 0; i < LOOP_SIZE; ++i) {
			try {
				abc.write("D");
			}
			catch (std::exception & ex) {

			}
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
	std::cout << "THREAD " << (end - begin) << std::endl;
}
