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
#include "Casting.h"
#include "TimeSpan.h"

namespace catapult { namespace utils {

	/// Represents a block duration.
	class BlockSpan final {
	private:
		constexpr explicit BlockSpan(uint64_t minutes) : m_minutes(minutes)
		{}

	public:
		/// Creates a default (zero) block span.
		constexpr BlockSpan() : BlockSpan(0)
		{}

	public:
		/// Creates a block span from the given number of \a minutes.
		static constexpr BlockSpan FromMinutes(uint64_t minutes) {
			return BlockSpan(minutes);
		}

		/// Creates a block span from the given number of \a hours.
		static constexpr BlockSpan FromHours(uint64_t hours) {
			return FromMinutes(hours * 60);
		}

		/// Creates a block span from the given number of \a days.
		static constexpr BlockSpan FromDays(uint64_t days) {
			return FromHours(days * 24);
		}

	public:
		/// Gets the number of minutes.
		constexpr uint64_t minutes() const {
			return m_minutes;
		}

		/// Gets the number of hours.
		constexpr uint64_t hours() const {
			return m_minutes / 60;
		}

		/// Gets the number of days.
		constexpr uint64_t days() const {
			return hours() / 24;
		}

		/// Gets the approximate number of blocks given the generation target time (\a generationTargetTime).
		BlockDuration blocks(const TimeSpan& generationTargetTime) const {
			auto millisPerMinute = TimeSpan::FromMinutes(1).millis();
			if (m_minutes > std::numeric_limits<uint64_t>::max() / millisPerMinute)
				CATAPULT_THROW_RUNTIME_ERROR_1("overflow while calculating blocks from minutes", m_minutes);

			return BlockDuration(m_minutes * millisPerMinute / generationTargetTime.millis());
		}

	public:
		/// Returns \c true if this block span is equal to \a rhs.
		constexpr bool operator==(const BlockSpan& rhs) const {
			return m_minutes == rhs.m_minutes;
		}

		/// Returns \c true if this block span is not equal to \a rhs.
		constexpr bool operator!=(const BlockSpan& rhs) const {
			return !(*this == rhs);
		}

		/// Returns \c true if this block span is greater than or equal to \a rhs.
		constexpr bool operator>=(const BlockSpan& rhs) const {
			return m_minutes >= rhs.m_minutes;
		}

		/// Returns \c true if this block span is greater than \a rhs.
		constexpr bool operator>(const BlockSpan& rhs) const {
			return m_minutes > rhs.m_minutes;
		}

		/// Returns \c true if this block span is less than or equal to \a rhs.
		constexpr bool operator<=(const BlockSpan& rhs) const {
			return m_minutes <= rhs.m_minutes;
		}

		/// Returns \c true if this block span is less than \a rhs.
		constexpr bool operator<(const BlockSpan& rhs) const {
			return m_minutes < rhs.m_minutes;
		}

	private:
		uint64_t m_minutes;
	};

	/// Insertion operator for outputting \a blockSpan to \a out.
	std::ostream& operator<<(std::ostream& out, const BlockSpan& blockSpan);
}}
