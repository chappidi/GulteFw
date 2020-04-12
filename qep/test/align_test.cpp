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

template<typename T>
constexpr bool isCacheLineAligned() {
	return alignof(T) % 64 == 0 || alignof(T) == 8 || alignog(T) == 16;
}
template<typename T>
struct alignas(64) CLAlign : T {

};

TEST_F(AlignTest, abc_abc) {
	struct S {
		char a; // size: 1, alignment: 1
		char b; // size: 1, alignment: 1
	}; // size: 2, alignment: 1

	// objects of type X must be allocated at 4-byte boundaries
	// because X.n must be allocated at 4-byte boundaries
	// because int's alignment requirement is (usually) 4
	struct X {
		int n;  // size: 4, alignment: 4
		char c; // size: 1, alignment: 1
		// three bytes padding
	}; // size: 8, alignment: 4 

	std::cout << "sizeof(S) = " << sizeof(S) << " alignof(S) = " << alignof(S) << '\n';
	std::cout << "sizeof(X) = " << sizeof(X) << " alignof(X) = " << alignof(X) << '\n';
	std::cout << "sizeof(CLAlign<X>) = " << sizeof(CLAlign<X>) << " alignof(CLAlign<X>) = " << alignof(CLAlign<X>) << '\n';
}

template <typename T>
void buf_read(void* buf, short sz)
{
	T* ptr = (T*)buf;
	T* ptrEnd = ptr + sz;

	while (ptr != ptrEnd) {
		*ptr++ = -*ptr;
	}
}

TEST_F(AlignTest, align_abc) {
	char* buf = new char[800000];
	{
		auto begin = __rdtsc();
		for (int i = 0; i < 1000000; ++i) {
			buf_read<long long>(buf, 200000);
		}
		auto end = __rdtsc();
		std::cout << "ALIGNED " << (end - begin) / cpu_freq << std::endl;
	}
	{
		auto begin = __rdtsc();
		for (int i = 0; i < 1000000; ++i) {
			buf_read<long long>(buf + 2, 200000);
		}
		auto end = __rdtsc();
		std::cout << "MIS ALIGNED (2X) " << (end - begin) / cpu_freq << std::endl;
	}
}