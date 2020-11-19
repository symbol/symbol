/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#pragma once
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
		SpinLock() {
			unlock();
		}

	public:
		/// Blocks until a lock can be obtained for the current execution agent.
		inline void lock() {
			while (!try_lock())
				std::this_thread::yield();
		}

		/// Attempts to acquire the lock for the current execution agent without blocking.
		inline bool try_lock() {
			return !m_isLocked.test_and_set(std::memory_order_acquire);
		}

		/// Releases the lock held by the execution agent. Throws no exceptions.
		inline void unlock() noexcept {
			m_isLocked.clear(std::memory_order_release);
		}

	private:
		std::atomic_flag m_isLocked;
	};

	/// Spin lock guard.
	using SpinLockGuard = std::lock_guard<SpinLock>;
}}
