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

#include "VotingSet.h"
#include "catapult/exceptions.h"

namespace catapult { namespace model {

	namespace {
		void CheckParameters(FinalizationEpoch epoch, uint64_t grouping) {
			if (0 == grouping)
				CATAPULT_THROW_INVALID_ARGUMENT("grouping zero is not supported");

			if (FinalizationEpoch() == epoch)
				CATAPULT_THROW_INVALID_ARGUMENT("epoch zero is not supported");
		}

		void CheckParameters(Height height, uint64_t grouping) {
			if (0 == grouping)
				CATAPULT_THROW_INVALID_ARGUMENT("grouping zero is not supported");

			if (Height() == height)
				CATAPULT_THROW_INVALID_ARGUMENT("height zero is not supported");
		}
	}

	Height CalculateVotingSetStartHeight(FinalizationEpoch epoch, uint64_t grouping) {
		CheckParameters(epoch, grouping);
		return epoch <= FinalizationEpoch(2)
				? Height(epoch.unwrap())
				: Height((epoch.unwrap() - 2) * grouping + 1);
	}

	Height CalculateVotingSetEndHeight(FinalizationEpoch epoch, uint64_t grouping) {
		CheckParameters(epoch, grouping);
		return FinalizationEpoch(1) == epoch
				? Height(1)
				: Height((epoch.unwrap() - 1) * grouping);
	}

	FinalizationEpoch CalculateFinalizationEpochForHeight(Height height, uint64_t grouping) {
		CheckParameters(height, grouping);
		if (height <= Height(1))
			return FinalizationEpoch(1);

		uint32_t adjustment = 0 == height.unwrap() % grouping ? 0 : 1;
		return FinalizationEpoch(1 + adjustment + static_cast<FinalizationEpoch::ValueType>(height.unwrap() / grouping));
	}
}}
