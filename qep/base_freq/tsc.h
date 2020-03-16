#pragma once
#include <chrono>
#ifdef __linux__
#include <x86intrin.h>
#else
#include <intrin.h>
#endif
#include "cpu_info.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	time_since_epoch gives the duration since the beginning of the clock, not necessarily the Unix epoch.
//	steady_clock only guarantees to be monotonically increasing.
//	This means that steady_clock will always be moving forward and that it will never decrease.
//
//	There is no guarantee about steady_clock representing anything meaningful.
//	It can be the duration since the beginning of the program execution,
//		the duration that the computer has been turned on,
//		or pretty much anything as long as it continues to move forward.
//
//	In other words, steady_clock is not actually all that useful to tell real world time.
//	It is only useful to measure the passage of time.
//	It's uses include measuring the duration between two points: benchmarking, progress estimates, etc.
//
//	If you're looking for real world time, you should look into std::system_clock, a clock that represents the time of the system (i.e. the operating system's time).
//	It's great for telling time, but it's pretty pretty useless for measuring differentials
//		since it is not guaranteed to be monotonic and is almost certainly not given daylight saving time, users adjusting their clocks,
//		and other events that can alter real world time.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace tsc 
{
	///////////////////////////////////////////////////////////////////////////////////////	
	// clock counter begins at restart of computer
	struct steady_clock {
		using time_point = std::int64_t;
		static constexpr bool is_steady = true;

		inline static time_point tick()					{	return __rdtsc();		}
		inline static time_point tick(unsigned int& ui) {	return __rdtscp(&ui);	}
	};
	using high_resolution_clock = steady_clock;

	///////////////////////////////////////////////////////////////////////////////////////	
	// replacement for std::chrono::system_clock if it is faster
	struct system_clock {
		inline static auto now(uint64_t cpu_freq) {
			constexpr uint64_t NSF = 1000000000;
			thread_local auto ts = std::chrono::system_clock::now();
			thread_local auto tk = tsc::steady_clock::tick();

			auto tmp = tsc::steady_clock::tick();
			auto x = std::chrono::nanoseconds(((tmp - tk) * NSF) / cpu_freq);
			auto y = std::chrono::duration_cast<std::chrono::system_clock::duration>(x);

			//// adjust the drift every 100 sec
			if (y.count() > NSF) {
				tk = tsc::steady_clock::tick();
				ts = std::chrono::system_clock::now();
				return ts;
			}
			else {
				return ts + y;
			}
		}
	};
	///////////////////////////////////////////////////////////////////////////////////////
	// calculate the duration
	template<typename T>
	static inline auto duration_cast(double ticks, double cpu_factor) {
		using CF = typename T::period;
		return (CF::num * ticks) / (cpu_factor / CF::den);
	}

}