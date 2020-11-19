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
#include "catapult/types.h"
#include <chrono>
#include <iosfwd>
#include <stdint.h>

namespace catapult { namespace utils {

	/// Represents a time duration.
	class TimeSpan final {
	private:
		template<typename TRep, typename TPeriod>
		constexpr explicit TimeSpan(const std::chrono::duration<TRep, TPeriod>& duration)
				: m_millis(std::chrono::duration_cast<std::chrono::milliseconds>(duration))
		{}

	public:
		/// Creates a default (zero) time span.
		constexpr TimeSpan() : TimeSpan(std::chrono::milliseconds(0))
		{}

	private:
		template<typename TDuration>
		static constexpr TimeSpan FromRawDuration(uint64_t count) {
			return TimeSpan(TDuration(static_cast<typename TDuration::rep>(count)));
		}

	public:
		/// Creates a time span from the given number of \a hours.
		static constexpr TimeSpan FromHours(uint64_t hours) {
			return FromRawDuration<std::chrono::hours>(hours);
		}

		/// Creates a time span from the given number of \a minutes.
		static constexpr TimeSpan FromMinutes(uint64_t minutes) {
			return FromRawDuration<std::chrono::minutes>(minutes);
		}

		/// Creates a time span from the given number of \a seconds.
		static constexpr TimeSpan FromSeconds(uint64_t seconds) {
			return FromRawDuration<std::chrono::seconds>(seconds);
		}

		/// Creates a time span from the given number of \a milliseconds.
		static constexpr TimeSpan FromMilliseconds(uint64_t milliseconds) {
			return FromRawDuration<std::chrono::milliseconds>(milliseconds);
		}

		/// Creates a time span from the difference between \a start and \a end.
		static constexpr TimeSpan FromDifference(Timestamp end, Timestamp start) {
			return FromMilliseconds((end - start).unwrap());
		}

	private:
		template<typename TDuration>
		static constexpr uint64_t ConvertToDuration(const std::chrono::milliseconds& millis) {
			return static_cast<uint64_t>(std::chrono::duration_cast<TDuration>(millis).count());
		}

	public:
		/// Gets the number of hours.
		constexpr uint64_t hours() const {
			return ConvertToDuration<std::chrono::hours>(m_millis);
		}

		/// Gets the number of minutes.
		constexpr uint64_t minutes() const {
			return ConvertToDuration<std::chrono::minutes>(m_millis);
		}

		/// Gets the number of seconds.
		constexpr uint64_t seconds() const {
			return ConvertToDuration<std::chrono::seconds>(m_millis);
		}

		/// Gets the number of milliseconds.
		constexpr uint64_t millis() const {
			return static_cast<uint64_t>(m_millis.count());
		}

	public:
		/// Returns \c true if this time span is equal to \a rhs.
		constexpr bool operator==(const TimeSpan& rhs) const {
			return m_millis == rhs.m_millis;
		}

		/// Returns \c true if this time span is not equal to \a rhs.
		constexpr bool operator!=(const TimeSpan& rhs) const {
			return !(*this == rhs);
		}

		/// Returns \c true if this time span is greater than or equal to \a rhs.
		constexpr bool operator>=(const TimeSpan& rhs) const {
			return m_millis >= rhs.m_millis;
		}

		/// Returns \c true if this time span is greater than \a rhs.
		constexpr bool operator>(const TimeSpan& rhs) const {
			return m_millis > rhs.m_millis;
		}

		/// Returns \c true if this time span is less than or equal to \a rhs.
		constexpr bool operator<=(const TimeSpan& rhs) const {
			return m_millis <= rhs.m_millis;
		}

		/// Returns \c true if this time span is less than \a rhs.
		constexpr bool operator<(const TimeSpan& rhs) const {
			return m_millis < rhs.m_millis;
		}

	private:
		std::chrono::milliseconds m_millis;
	};

	/// Insertion operator for outputting \a timeSpan to \a out.
	std::ostream& operator<<(std::ostream& out, const TimeSpan& timeSpan);

	/// Adds \a timestamp and \a timeSpan resulting in new timestamp.
	constexpr Timestamp operator+(const Timestamp& timestamp, const TimeSpan& timeSpan) {
		return timestamp + Timestamp(timeSpan.millis());
	}

	/// Subtracts \a timeSpan from \a timestamp and returns the maximum of the difference and zero.
	constexpr Timestamp SubtractNonNegative(const Timestamp& timestamp, const TimeSpan& timeSpan) {
		return Timestamp(timestamp.unwrap() < timeSpan.millis() ? 0u : timestamp.unwrap() - timeSpan.millis());
	}
}}
