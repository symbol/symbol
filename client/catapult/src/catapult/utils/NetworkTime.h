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
#include "catapult/types.h"
#include <chrono>

namespace catapult { namespace utils {

	// Note: a network timestamp is represented as the number of milliseconds since the Epoch_Time.
	//       a unix timestamp is represented as the number of milliseconds since 1970-01-01 00:00:00 UTC.

	/// Represents the number of milliseconds between 1970-01-01 00:00:00 UTC and 2016-04-01 00:00:00 UTC.
	constexpr auto Epoch_Time = std::chrono::duration<int64_t, std::milli>(1459468800000);

	/// Gets the network time, i.e. the number of milliseconds since Epoch_Time.
	Timestamp NetworkTime();

	/// Given a unix \a timestamp, returns the corresponding network timestamp
	/// that the unix timestamp represents.
	Timestamp ToNetworkTime(const Timestamp& timestamp);

	/// Given a network \a timestamp, returns the corresponding unix timestamp
	/// that the network timestamp represents.
	Timestamp ToUnixTime(const Timestamp& timestamp);
}}
