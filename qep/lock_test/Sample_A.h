#pragma once
#include "state.h"
#include "spin_lock.h"
#include <future>
#include <map>

/*
	Perfect implementation,.
	Uses SpinLock
	SAMPLE_A<spin_lock>::test()
*/

template<typename T>
struct SAMPLE_A {
	inline static T lock_;
	alignas(64) int index{ 0 };
	alignas(64) std::map<int, int> buf;

	STATE write() noexcept {
		STATE info;
		while (index < LOOP_SIZE) {
			//lock_guard constr to destructor makes the changes atomic
			std::lock_guard<T> grd(lock_);
			if (index < LOOP_SIZE) {
				index++;
				buf[index] = index;
				info.loop++;
			}
		}
		return info;
	}
	STATE read() noexcept {
		STATE info;
		while (index < LOOP_SIZE) {
			std::lock_guard<T> grd(lock_);
			auto itr = buf.find(index);
			if (itr == buf.end()) {

				if (info.errors_x == 0)
					info.msg << "NO FOUND " << index << std::endl;
				info.errors_x++;
			}
			else if (itr->second != index) {
				if (info.errors_y == 0)
					info.msg << "WRONG VAL " << index <<  " : " << buf[index] << std::endl;
				info.errors_y++;
			}
			info.loop++;
		}
		return info;
	}

	static void test() {
		constexpr int COUNT = 3;
		SAMPLE_A<T> data;
		std::future<STATE> tsks[COUNT][2];

		for (int itr = 0; itr < COUNT; itr++) {
			tsks[itr][0] = std::async(std::launch::async, &SAMPLE_A::write, &data);
			tsks[itr][1] = std::async(std::launch::async, &SAMPLE_A::read, &data);
		}

		STATE ws_t;
		STATE rs_t;
		for (int itr = 0; itr < COUNT; itr++) {
			auto ws = tsks[itr][0].get();
			auto rs = tsks[itr][1].get();
			std::cout << "[" << itr << "] WR:" << ws.loop << " RD:" << rs.loop << std::endl;
			ws_t += ws;
			rs_t += rs;
		}

		std::cout << "WRITER LOOPS " << ws_t.loop << std::endl;
		std::cout << "READER LOOPS " << rs_t.loop << std::endl;
		std::cout << "ERROR MSG" << rs_t.msg.str() << std::endl;
		std::cout << "COUNT " << data.buf.size() << " / " << data.index << std::endl;

	}
};

