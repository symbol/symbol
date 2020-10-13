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

namespace catapult { namespace model {

	/// Calculates the voting set start height for \a epoch given \a grouping.
	/// \note This is the first height that is included in the specified epoch.
	Height CalculateVotingSetStartHeight(FinalizationEpoch epoch, uint64_t grouping);

	/// Calculates the voting set end height for \a epoch given \a grouping.
	/// \note This is the last height that is included in the specified epoch.
	/// \note This is the grouped height that should be used with the next epoch.
	Height CalculateVotingSetEndHeight(FinalizationEpoch epoch, uint64_t grouping);

	/// Calculates the finalization epoch for \a height given \a grouping.
	FinalizationEpoch CalculateFinalizationEpochForHeight(Height height, uint64_t grouping);
}}
