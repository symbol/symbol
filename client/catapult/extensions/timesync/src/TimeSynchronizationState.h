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
#include "types.h"
#include "catapult/utils/TimeSpan.h"
#include <atomic>

namespace catapult { namespace timesync {

	/// Direction of a time offset.
	enum class TimeOffsetDirection : uint8_t {
		/// Time offset is in positive direction.
		Positive = 0,

		/// Time offset is in negative direction.
		Negative = 1
	};

	/// Time synchronization state.
	class TimeSynchronizationState {
	public:
		/// Creates a time synchronization state with an epoch adjustment relative to unix timestamp epoch of \a epochAdjustment
		/// that only updates internal offset values when magnitude of change is greater than \a clockAdjustmentThreshold.
		TimeSynchronizationState(const utils::TimeSpan& epochAdjustment, uint64_t clockAdjustmentThreshold);

	public:
		/// Gets the offset.
		TimeOffset offset() const;

		/// Gets the absolute value of the current offset.
		uint64_t absoluteOffset() const;

		/// Gets the offset direction.
		TimeOffsetDirection offsetDirection() const;

		/// Gets the node age.
		NodeAge nodeAge() const;

	public:
		/// Gets the network time.
		Timestamp networkTime() const;

		/// Updates the current offset using \a offset.
		void update(TimeOffset offset);

	private:
		utils::TimeSpan m_epochAdjustment;
		uint64_t m_clockAdjustmentThreshold;
		std::atomic<int64_t> m_offset;
		std::atomic<int64_t> m_nodeAge;
	};
}}
