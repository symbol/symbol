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

#include "TimeSynchronizationState.h"
#include "constants.h"
#include "catapult/utils/Casting.h"
#include "catapult/utils/NetworkTime.h"

namespace catapult { namespace timesync {

	TimeSynchronizationState::TimeSynchronizationState(const utils::TimeSpan& epochAdjustment, uint64_t clockAdjustmentThreshold)
			: m_epochAdjustment(epochAdjustment)
			, m_clockAdjustmentThreshold(clockAdjustmentThreshold)
			, m_offset(0)
			, m_nodeAge(0)
	{}

	TimeOffset TimeSynchronizationState::offset() const {
		return TimeOffset(m_offset);
	}

	uint64_t TimeSynchronizationState::absoluteOffset() const {
		return static_cast<uint64_t>(std::abs(m_offset));
	}

	TimeOffsetDirection TimeSynchronizationState::offsetDirection() const {
		return 0 > m_offset ? TimeOffsetDirection::Negative : TimeOffsetDirection::Positive;
	}

	NodeAge TimeSynchronizationState::nodeAge() const {
		return NodeAge(m_nodeAge);
	}

	Timestamp TimeSynchronizationState::networkTime() const {
		return Timestamp(utils::NetworkTime(m_epochAdjustment).now().unwrap() + static_cast<uint64_t>(m_offset));
	}

	void TimeSynchronizationState::update(TimeOffset offset) {
		if (m_clockAdjustmentThreshold < static_cast<uint64_t>(std::abs(offset.unwrap())))
			m_offset = m_offset + offset.unwrap();

		++m_nodeAge;
	}
}}
