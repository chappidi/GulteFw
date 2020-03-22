#pragma once
#include <thread>

#ifdef __linux__
#include <pthread.h>
#else
#include <Windows.h>
#endif // __linux__

namespace qep {

	class xthread : public std::thread {
	public:
		template<typename ... Args>
		xthread(Args&&... args) : std::thread(std::forward<Args>(args)...) {
			// empty
		}
		void setpriority() {
#ifdef __linux__
				//pthread_getschedparam(native_handle(),);
#else
			//SetThreadPriority(native_handle(), THREAD_PRIORITY_HIGHEST);
#endif
			void setaffinity(int core) {
#ifdef __linux__
				cpu_set_t cpuset;
				CPU_ZERO(&cpuset);
				CPU_SET(core, &cpuset);
				int rc = pthread_setaffinity_np(native_handle(), sizeof(cpu_set_t), &cpuset);
#else
				auto mask = (static_cast<DWORD_PTR>(0) << core);
				SetThreadAffinityMask(native_handle(), mask);
#endif
			}

		}
	};
}
