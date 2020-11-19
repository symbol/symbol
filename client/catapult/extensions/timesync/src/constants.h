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
#include <cstdint>

namespace catapult { namespace timesync {

	/// Start value for the coupling of clocks.
	constexpr double Coupling_Start = 1.0;

	/// Minimum value for the coupling of clocks.
	constexpr double Coupling_Minimum = 0.1;

	/// Value that indicates the round after which the decay starts.
	constexpr uint64_t Start_Coupling_Decay_After_Round = 5;

	/// Value that indicates the speed of the coupling decay.
	constexpr double Coupling_Decay_Strength = 0.3;

	/// Value that indicates how large the change in network time must be in order to update the node's network time.
	/// \note This constant is used to prevent slow shifts in network time. The unit of this constant is milliseconds.
	constexpr uint64_t Clock_Adjustment_Threshold = 75;

	/// Minimum offset from remote to local node to trigger logging a warning.
	constexpr int64_t Warning_Threshold_Millis = 100;
}}
