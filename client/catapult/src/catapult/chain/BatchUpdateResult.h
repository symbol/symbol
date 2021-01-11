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
#include <iosfwd>
#include <stddef.h>

namespace catapult { namespace chain {

	/// Result of a batch update operation.
	struct BatchUpdateResult {
	public:
		/// Creates a default result.
		constexpr BatchUpdateResult() : BatchUpdateResult(0, 0, 0)
		{}

		/// Creates a result with initial values \a successCount, \a neutralCount and \a failureCount.
		constexpr BatchUpdateResult(size_t successCount, size_t neutralCount, size_t failureCount)
				: SuccessCount(successCount)
				, NeutralCount(neutralCount)
				, FailureCount(failureCount)
		{}

	public:
		/// Number of successful sub operations.
		size_t SuccessCount;

		/// Number of neutral sub operations.
		size_t NeutralCount;

		/// Number of failed sub operations.
		size_t FailureCount;

	public:
		/// Returns \c true if this result is equal to \a rhs.
		constexpr bool operator==(const BatchUpdateResult& rhs) const {
			return SuccessCount == rhs.SuccessCount && NeutralCount == rhs.NeutralCount && FailureCount == rhs.FailureCount;
		}

		/// Returns \c true if this result is not equal to \a rhs.
		constexpr bool operator!=(const BatchUpdateResult& rhs) const {
			return !(*this == rhs);
		}
	};

	/// Insertion operator for outputting \a result to \a out.
	std::ostream& operator<<(std::ostream& out, const BatchUpdateResult& result);
}}
