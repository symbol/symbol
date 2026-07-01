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
#include <stdint.h>

namespace catapult { namespace utils {

	/// Runs \a operation up to \a numAttempts times, retrying only while \a isRetryable returns \c true
	/// for the most recent error and further attempts remain. \a onRetry is invoked with the zero-based
	/// attempt index and the error before every retry; it is expected to apply backoff.
	/// \note \a operation must return a default-constructed (falsy) error to signal success.
	template<typename TOperation, typename TIsRetryable, typename TOnRetry>
	auto RetryWithBackoff(TOperation operation, TIsRetryable isRetryable, uint32_t numAttempts, TOnRetry onRetry) {
		auto error = operation();
		for (auto attempt = 0u; error && isRetryable(error) && attempt + 1 < numAttempts; ++attempt) {
			onRetry(attempt, error);
			error = operation();
		}

		return error;
	}
}}
