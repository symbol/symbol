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

#include "NetworkTime.h"
#include "Casting.h"
#include <limits>

namespace catapult { namespace utils {

	NetworkTime::NetworkTime(const utils::TimeSpan& epochAdjustment) : m_epochAdjustment(epochAdjustment)
	{}

	Timestamp NetworkTime::now() const {
		auto nowMillis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
		return Timestamp(static_cast<uint64_t>(nowMillis.count()) - m_epochAdjustment.millis());
	}

	Timestamp NetworkTime::toNetworkTime(const Timestamp& timestamp) const {
		if (timestamp.unwrap() < m_epochAdjustment.millis())
			CATAPULT_THROW_INVALID_ARGUMENT_1("Unix timestamp must be after epoch time", timestamp);

		return Timestamp(static_cast<uint64_t>(timestamp.unwrap() - m_epochAdjustment.millis()));
	}

	Timestamp NetworkTime::toUnixTime(const Timestamp& timestamp) const {
		if (std::numeric_limits<uint64_t>::max() - m_epochAdjustment.millis() < timestamp.unwrap())
			CATAPULT_THROW_INVALID_ARGUMENT_1("overflow detected in ToUnixTime", timestamp);

		return Timestamp(static_cast<uint64_t>(m_epochAdjustment.millis() + timestamp.unwrap()));
	}
}}
