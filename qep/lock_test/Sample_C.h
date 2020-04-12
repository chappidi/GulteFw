#pragma once
#include "state.h"
#include <future>
#include <map>

/*
	uses atomic index and buff.
	No use of locking with spin_lock

	Identical to Sample_B but operation seq is reversed

	1. later add to buffer
	2. Increment index
*/

template<typename T>
struct SAMPLE_C {
	alignas(64) std::atomic<int> index{ 0 };
	alignas(64) std::map<int, int> buf;

	STATE write() noexcept {
		STATE info;
		while (index.load() < LOOP_SIZE) {
			// add to cache
			buf[index + 1] = index + 1;
			// increment index
			index.fetch_add(1);
			info.loop++;
		}
		return info;
	}
	STATE read() noexcept {
		STATE info;
		while (index.load() < LOOP_SIZE) {
			// find
			auto itr = buf.find(index.load());
			// check if key exists
			if (itr == buf.end())
			{
				/*
				This happens because
					1. index is atomic and the change is reflected before data is inserted into buf
					2. The buf changes are still in l1 cache and not pushed out immediately
				*/
				if (info.errors_x == 0)
					info.msg << std::endl << "\tNOT FOUND " << index;
				info.errors_x++;
			}
			// check if correct value
			else if (itr->second != index)
			{
				/*
					buf[index] = index.load() is not atomic operation
					The buf changes are still in L1 cache and not pushed out immediately
				*/
				if (info.errors_y == 0)
					info.msg << std::endl << "\tWRONG VAL " << index << " : " << buf[index];
				info.errors_y++;
			}
			info.loop++;
		}
		return info;
	}

	static void test() {
		constexpr int COUNT = 1;
		SAMPLE_C<T> data;
		std::future<STATE> tsks[COUNT][2];

		for (int itr = 0; itr < COUNT; itr++) {
			tsks[itr][0] = std::async(std::launch::async, &SAMPLE_C::write, &data);
			tsks[itr][1] = std::async(std::launch::async, &SAMPLE_C::read, &data);
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
		std::cout << "ERROR COUNT " << rs_t.errors_x << " / " << rs_t.errors_y << std::endl;
		std::cout << "ERROR MSG" << rs_t.msg.str() << std::endl;
		std::cout << "COUNT " << data.buf.size() << " / " << data.index << std::endl;
	}
};
