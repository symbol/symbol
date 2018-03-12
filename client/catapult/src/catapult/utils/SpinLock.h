#pragma once
#include "catapult/preprocessor.h"
#include <atomic>
#include <mutex>
#include <thread>

namespace catapult { namespace utils {

	/// Simple spin lock implemented by using an atomic.
	/// Note that function naming allows this to be used as a Lockable type
	/// (http://en.cppreference.com/w/cpp/concept/Lockable).
	class SpinLock {
	public:
		/// Creates an unlocked lock.
		SpinLock() { unlock(); }

	public:
		/// Blocks until a lock can be obtained for the current execution agent.
		CATAPULT_INLINE
		void lock() {
			while (!try_lock())
				std::this_thread::yield();
		}

		/// Attempts to acquire the lock for the current execution agent without blocking.
		CATAPULT_INLINE
		bool try_lock() {
			return !m_isLocked.test_and_set(std::memory_order_acquire);
		}

		/// Releases the lock held by the execution agent. Throws no exceptions.
		CATAPULT_INLINE
		void unlock() noexcept {
			m_isLocked.clear(std::memory_order_release);
		}

	private:
		std::atomic_flag m_isLocked;
	};

	/// A spin lock guard.
	using SpinLockGuard = std::lock_guard<SpinLock>;
}}
