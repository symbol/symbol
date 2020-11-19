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
#include "BlockStatistic.h"
#include <algorithm>

namespace catapult { namespace state {

	/// Calculates the dynamic fee multiplier over the specified range [\a beginIter, \a endIter)
	/// given the number of historical statistics (\a count) and default multiplier value (\a defaultFeeMultiplier).
	template<typename TIterator>
	BlockFeeMultiplier CalculateDynamicFeeMultiplier(
			const TIterator& beginIter,
			const TIterator& endIter,
			size_t count,
			BlockFeeMultiplier defaultFeeMultiplier) {
		if (0 == count)
			return defaultFeeMultiplier;

		std::vector<BlockFeeMultiplier> feeMultipliers;
		feeMultipliers.reserve(count);

		for (auto iter = beginIter; endIter != iter && feeMultipliers.size() < count; ++iter) {
			if (BlockFeeMultiplier() != iter->FeeMultiplier)
				feeMultipliers.push_back(iter->FeeMultiplier);
		}

		while (feeMultipliers.size() < count)
			feeMultipliers.push_back(defaultFeeMultiplier);

		auto feeMultipliersMedianIter = feeMultipliers.begin() + count / 2;
		std::nth_element(feeMultipliers.begin(), feeMultipliersMedianIter, feeMultipliers.end());
		return *feeMultipliersMedianIter;
	}
}}
