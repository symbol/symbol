/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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
#include "Casting.h"
#include "TimeSpan.h"

namespace catapult { namespace utils {

	/// Represents a block duration.
	class BlockSpan final {
	private:
		constexpr explicit BlockSpan(uint64_t hours) : m_hours(hours)
		{}

	public:
		/// Creates a default (zero) block span.
		constexpr BlockSpan() : BlockSpan(0)
		{}

	public:
		/// Creates a block span from the given number of \a hours.
		static constexpr BlockSpan FromHours(uint64_t hours) {
			return BlockSpan(hours);
		}

		/// Creates a block span from the given number of \a days.
		static constexpr BlockSpan FromDays(uint64_t days) {
			return BlockSpan(days * 24);
		}

	public:
		/// Returns the number of hours.
		constexpr uint64_t hours() const {
			return m_hours;
		}

		/// Returns the number of days.
		constexpr uint64_t days() const {
			return m_hours / 24;
		}

		/// Returns the approximate number of blocks given the generation target time (\a generationTargetTime).
		BlockDuration blocks(const TimeSpan& generationTargetTime) const {
			auto millisPerHour = TimeSpan::FromHours(1).millis();
			if (m_hours > std::numeric_limits<uint64_t>::max() / millisPerHour)
				CATAPULT_THROW_RUNTIME_ERROR_1("overflow while calculating blocks from hours", m_hours);

			return BlockDuration(m_hours * millisPerHour / generationTargetTime.millis());
		}

	public:
		/// Returns \c true if this block span is equal to \a rhs.
		constexpr bool operator==(const BlockSpan& rhs) const {
			return m_hours == rhs.m_hours;
		}

		/// Returns \c true if this block span is not equal to \a rhs.
		constexpr bool operator!=(const BlockSpan& rhs) const {
			return !(*this == rhs);
		}

		/// Returns \c true if this block span is greater than or equal to \a rhs.
		constexpr bool operator>=(const BlockSpan& rhs) const {
			return m_hours >= rhs.m_hours;
		}

		/// Returns \c true if this block span is greater than \a rhs.
		constexpr bool operator>(const BlockSpan& rhs) const {
			return m_hours > rhs.m_hours;
		}

		/// Returns \c true if this block span is less than or equal to \a rhs.
		constexpr bool operator<=(const BlockSpan& rhs) const {
			return m_hours <= rhs.m_hours;
		}

		/// Returns \c true if this block span is less than \a rhs.
		constexpr bool operator<(const BlockSpan& rhs) const {
			return m_hours < rhs.m_hours;
		}

	private:
		uint64_t m_hours;
	};

	/// Insertion operator for outputting \a blockSpan to \a out.
	std::ostream& operator<<(std::ostream& out, const BlockSpan& blockSpan);
}}
