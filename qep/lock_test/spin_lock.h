#pragma once
#include <atomic>

struct no_lock
{
	inline void lock() {}
	inline bool try_lock() {}
	inline void unlock() {}
};

class spin_lock
{
	std::atomic_flag m_lock;
public:
	spin_lock() { m_lock.clear(); }
	spin_lock(const spin_lock&) = delete;
	~spin_lock() = default;

	inline void lock() {
		while (m_lock.test_and_set(std::memory_order_acquire))
			;
	}
	inline bool try_lock() {
		return !m_lock.test_and_set(std::memory_order_acquire);
	}
	inline void unlock() {
		m_lock.clear(std::memory_order_release);
	}
};