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
#include "catapult/utils/TimeSpan.h"
#include <cstdint>

namespace catapult { namespace timesync { namespace filters {

	/// Start value for the maximum tolerated deviation in ms.
	constexpr auto Tolerated_Deviation_Start = utils::TimeSpan::FromMinutes(120);

	/// Minimum value for the maximum tolerated deviation in ms.
	constexpr auto Tolerated_Deviation_Minimum = utils::TimeSpan::FromMinutes(1);

	/// Value that indicates after which round the decay starts.
	constexpr int64_t Start_Decay_After_Round = 5;

	/// Value that indicates the speed of the decay.
	constexpr double Decay_Strength = 0.3;

	/// Value that indicates the percentage of the samples is discarded.
	constexpr double Alpha = 0.4;

	/// Maximum time in ms that a response to a time sync request may take.
	constexpr int64_t Tolerated_Duration_Maximum = 1000;
}}}
