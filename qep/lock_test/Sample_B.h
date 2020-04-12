#pragma once
#include "state.h"
#include <future>
#include <map>

/*
	uses atomic index and buff.
	No use of locking with spin_lock

	1. Increment index 
	2. later add to cache
*/

template<typename T>
struct SAMPLE_B {
	alignas(64) std::atomic<int> index{ 0 };
	std::map<int, int> buf;
//	alignas(64) std::map<int, int> buf;
	int* lt = new int[LOOP_SIZE];

	inline STATE write() noexcept {
		STATE info;
		// loop limit
		while (index.load(std::memory_order_relaxed) < LOOP_SIZE) {
			info.loop++;
			//1. Increment index
			auto prev = index.fetch_add(1, std::memory_order_acquire);
			// 2. Add to cache the index
			prev++;
			buf.try_emplace(prev, prev);
			lt[prev] = prev;
		}
		return info;
	}
	inline STATE read() noexcept {
		STATE info;
		while (index.load() < LOOP_SIZE) {
			info.loop++;
			// skip the zero value
			if (index.load() == 0) continue;

			// find
			auto itr = buf.find(index.load());
			// check if key exists
			if (itr == buf.end())
			{
				/*	This happens because
					1. index is atomic and the change is reflected before data is inserted into buf
					2. The buf changes are still in l1 cache and not pushed out immediately
				*/
				if (info.errors_x == 0) { info.msg << std::endl << "\tNOT FOUND " << index; }
				info.errors_x++;
			}
			// check if correct value
			else if (itr->second != index)
			{
				/*	
					buf[index] = index.load() is not atomic operation
					The buf changes are still in L1 cache and not pushed out immediately
				*/
				if (info.errors_y == 0) { info.msg << std::endl << "\tWRONG VAL " << index << " : " << buf[index]; }
				info.errors_y++;
			}
			else {
				info.success++;
			}
		}
		return info;
	}

	static void test() {
		constexpr int COUNT = 1;
		SAMPLE_B<T> data;
		std::future<STATE> tsks[COUNT][2];

		auto begin = __rdtsc();
		for (int itr = 0; itr < COUNT; itr++) {
			tsks[itr][1] = std::async(std::launch::async, &SAMPLE_B::read, &data);
			tsks[itr][0] = std::async(std::launch::async, &SAMPLE_B::write, &data);
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
		auto end = __rdtsc();

		std::cout << "WRITER LOOPS " << ws_t.loop << std::endl;
		std::cout << "READER LOOPS " << rs_t.loop << std::endl;
		auto ttl = rs_t.errors_x + rs_t.errors_y + rs_t.success;
		std::cout << "ERROR COUNT  " << rs_t.errors_x << " / " << rs_t.errors_y << " / " << rs_t.success << std::endl;
		std::cout << "\t\t" << ttl << std::endl;
		std::cout << "ERROR MSG" << rs_t.msg.str() << std::endl;
		std::cout << "COUNT " << data.buf.size() << " / " << data.index << std::endl;

		for (int i = 0; i < LOOP_SIZE; i++)
		{
			if (data.lt[i] != i) {
				std::cout << "ERROR in LIST" << std::endl;
				exit(0);
			}
		}
		if (data.buf.size() != data.index) {
			int tmp{ 1 };
			for (auto itr = data.buf.begin(); itr != data.buf.end(); ++itr) {
				if (itr->first != tmp) {
					std::cout << "BREAK " << itr->first << " / " << tmp <<  std::endl;
					exit(0);
				}
				tmp++;
			}
			std::cout << "MIN\t" << data.buf.begin()->first << " / " << data.buf.rbegin()->first << std::endl;
		}
	}
};
