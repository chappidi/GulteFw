#pragma once
#include <chrono>
//#include <atomic>
//#include <future>
namespace tsc
{
	struct cpu_info final {
		using time_point = std::chrono::time_point<std::chrono::steady_clock>;
		// variables
		inline static const time_point	hrc_start = std::chrono::steady_clock::now();
		inline static const uint64_t	tsc_start = __rdtsc();

		///////////////////////////////////////////////////////
		// This works for intel only
		inline static auto intel() {
			int cpuInfo[4] = { 0, 0, 0, 0 };
			__cpuid(cpuInfo, 0x16);
			return cpuInfo[0];
		}
		///////////////////////////////////////////////////////
		// cpu base freq in hz
		inline static auto freq() {
			uint64_t frq = intel();
			if (frq == 0) {
				auto hrc_end = std::chrono::steady_clock::now();
				auto tsc_end = __rdtsc();
				// elapsed tsc / elapsed time = cpu freq
				auto delta = std::chrono::duration<double>(hrc_end - hrc_start).count();
				frq = (tsc_end - tsc_start) / delta;
			}
			return frq;
		}
	};


/*
	struct freq_calculator final {
		using time_point = std::chrono::time_point<std::chrono::steady_clock>;
		// variables
		const time_point hrc_start = std::chrono::steady_clock::now();
		const std::uint64_t tsc_start = __rdtsc();
		std::atomic<double> freq{ 1.0 };
		std::future<void> _task;

		freq_calculator() {
			calibrate();
			_task = std::async(std::launch::async, [&] {
				std::uint64_t interval = 8;
				for (int itr = 0; itr < 25; ++itr) {
					interval <<= 1;
					std::this_thread::sleep_for(std::chrono::microseconds(interval));
					calibrate();
				}
				});
		}
		void calibrate() {
			// snapshot end
			auto hrc_end = std::chrono::steady_clock::now();
			auto tsc_end = __rdtsc();
			// elapsed tsc / elapsed time = cpu freq
			auto delta = std::chrono::duration<double>(hrc_end - hrc_start).count();
			freq = (tsc_end - tsc_start) / delta;
		}
	};
	// global
//	static freq_calculator _cpu;

	template<typename T>
	static inline auto duration(uint64_t ticks) {
		using CF = typename T::period;
		return (CF::num * 1.0 * ticks) / (_cpu.freq / CF::den);
	}
	static auto drift() {
		// snapshot end
		auto hrc_end = std::chrono::steady_clock::now();
		auto tsc_end = __rdtsc();
		auto x = std::chrono::duration_cast<std::chrono::nanoseconds>(hrc_end - _cpu.hrc_start).count();
		auto y = tsc::duration<std::chrono::nanoseconds>(tsc_end - _cpu.tsc_start);
		return std::abs(y - x);
	}
*/
}