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
#include <chrono>
#include <thread>
#include <stdint.h>

namespace catapult { namespace utils {

	/// Maximum delay, in milliseconds, below which RetryWithBackoff yields instead of sleeping.
	/// \note Windows timer resolution is coarser, so it gets a higher threshold than other platforms.
#ifdef _WIN32
	constexpr uint32_t Retry_Yield_Threshold_Ms = 50;
#else
	constexpr uint32_t Retry_Yield_Threshold_Ms = 20;
#endif

	/// Runs \a operation up to \a numAttempts times, retrying only while \a shouldRetry returns \c true
	/// for the most recent result and further attempts remain. Waits between attempts with exponential
	/// backoff starting at \a baseDelayMs (doubling each attempt): a zero delay does not wait at all,
	/// a delay up to \c Retry_Yield_Threshold_Ms yields the thread, otherwise it sleeps for that long.
	/// \a onRetry is invoked with the zero-based attempt index, the result and the upcoming delay before
	/// every retry, for logging purposes only.
	/// \note \a shouldRetry must return \c false for a result that indicates success.
	template<typename TOperation, typename TShouldRetry, typename TOnRetry>
	auto RetryWithBackoff(
			TOperation operation,
			TShouldRetry shouldRetry,
			uint32_t numAttempts,
			TOnRetry onRetry,
			uint32_t baseDelayMs = 100) {
		auto result = operation();
		for (auto attempt = 0u; shouldRetry(result) && attempt + 1 < numAttempts; ++attempt) {
			auto delayMs = baseDelayMs << attempt;
			onRetry(attempt, result, delayMs);
			if (delayMs > Retry_Yield_Threshold_Ms)
				std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
			else if (delayMs > 0)
				std::this_thread::yield();

			result = operation();
		}

		return result;
	}
}}
