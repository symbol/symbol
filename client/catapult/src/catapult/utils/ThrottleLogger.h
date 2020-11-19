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
#include "Logging.h"
#include "SpinLock.h"
#include <chrono>

namespace catapult { namespace utils {

	/// Simple throttle logger.
	/// \note Instances need to be threadsafe due to usage in CATAPULT_LOG_THROTTLE.
	class ThrottleLogger {
	private:
		using Clock = std::chrono::steady_clock;

	public:
		/// Constructs a logger with the specified throttle in milliseconds (\a throttleMillis).
		explicit ThrottleLogger(uint64_t throttleMillis)
				: m_throttleMillis(throttleMillis)
				, m_counter(0)
		{}

	public:
		/// Gets the total number of log attempts.
		uint32_t counter() const {
			return m_counter;
		}

	public:
		/// Returns \c false if a log should be output.
		bool isThrottled() {
			++m_counter;

			SpinLockGuard guard(m_lock);
			auto now = Clock::now();
			if (Clock::time_point() != m_last && millis(now) < m_throttleMillis)
				return true;

			m_last = now;
			return false;
		}

	private:
		uint64_t millis(const Clock::time_point& now) const {
			auto elapsedDuration = now - m_last;
			return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(elapsedDuration).count());
		}

	private:
		uint64_t m_throttleMillis;
		std::atomic<uint32_t> m_counter; // atomic to allow increment / access outside of m_lock
		Clock::time_point m_last; // protected by m_lock
		utils::SpinLock m_lock;
	};

/// Logs a throttled message at \a LEVEL at most every \a THROTTLE_MILLIS milliseconds.
/// \note \a THROTTLE_MILLIS must be a compile time constant or behavior is undefined.
#define CATAPULT_LOG_THROTTLE(LEVEL, THROTTLE_MILLIS) \
	static utils::ThrottleLogger BOOST_LOG_UNIQUE_IDENTIFIER_NAME_INTERNAL(throttle_logger, __LINE__)(THROTTLE_MILLIS); \
	if (!BOOST_LOG_UNIQUE_IDENTIFIER_NAME_INTERNAL(throttle_logger, __LINE__).isThrottled()) \
		CATAPULT_LOG(LEVEL) << "[" << BOOST_LOG_UNIQUE_IDENTIFIER_NAME_INTERNAL(throttle_logger, __LINE__).counter() << " log count] "
}}
