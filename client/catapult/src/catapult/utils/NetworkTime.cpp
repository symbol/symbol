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

#include "NetworkTime.h"
#include "Casting.h"
#include <limits>

namespace catapult { namespace utils {

	Timestamp NetworkTime() {
		auto now = std::chrono::system_clock::now().time_since_epoch();
		auto nowMillis = std::chrono::duration_cast<std::chrono::milliseconds>(now);
		return Timestamp(static_cast<uint64_t>((nowMillis - Epoch_Time).count()));
	}

	Timestamp ToNetworkTime(const Timestamp& timestamp) {
		if (timestamp.unwrap() < static_cast<uint64_t>(Epoch_Time.count()))
			CATAPULT_THROW_INVALID_ARGUMENT_1("Unix timestamp must be after epoch time", timestamp);

		return Timestamp(static_cast<uint64_t>(timestamp.unwrap() - Epoch_Time.count()));
	}

	Timestamp ToUnixTime(const Timestamp& timestamp) {
		if (std::numeric_limits<uint64_t>::max() - Epoch_Time.count() < timestamp.unwrap())
			CATAPULT_THROW_INVALID_ARGUMENT_1("overflow detected in ToUnixTime", timestamp);

		return Timestamp(static_cast<uint64_t>(Epoch_Time.count() + timestamp.unwrap()));
	}
}}
