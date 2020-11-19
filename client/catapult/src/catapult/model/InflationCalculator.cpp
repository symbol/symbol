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

#include "InflationCalculator.h"
#include "catapult/exceptions.h"

namespace catapult { namespace model {

	size_t InflationCalculator::size() const {
		return m_inflationMap.size();
	}

	bool InflationCalculator::contains(Height height, Amount amount) const {
		auto iter = m_inflationMap.find(height);
		return m_inflationMap.cend() != iter && iter->second == amount;
	}

	Amount InflationCalculator::getSpotAmount(Height height) const {
		auto amount = Amount();
		for (const auto& pair : m_inflationMap) {
			if (height < pair.first)
				break;

			amount = pair.second;
		}

		return amount;
	}

	Amount InflationCalculator::getCumulativeAmount(Height height) const {
		if (Height() == height)
			return Amount();

		Amount totalAmount;
		Amount currentAmount;
		Height currentHeight(1);
		for (const auto& pair : m_inflationMap) {
			auto numBlocks = ((height < pair.first ? height : pair.first) - currentHeight).unwrap();
			totalAmount = totalAmount + Amount(currentAmount.unwrap() * numBlocks);
			currentHeight = pair.first;
			currentAmount = pair.second;
			if (height <= pair.first)
				break;
		}

		if (currentHeight < height)
			totalAmount = totalAmount + Amount(currentAmount.unwrap() * (height - currentHeight).unwrap());

		return totalAmount;
	}

	std::pair<Amount, bool> InflationCalculator::sumAll() const {
		auto currentPair = std::make_pair(Height(1), Amount());
		uint64_t totalAmountRaw = 0;
		auto maxAmount = std::numeric_limits<uint64_t>::max();
		for (const auto& pair : m_inflationMap) {
			auto numBlocks = (pair.first - currentPair.first).unwrap();
			if (0 != numBlocks && currentPair.second.unwrap() > maxAmount / numBlocks)
				return std::make_pair(Amount(), false);

			auto summand = currentPair.second.unwrap() * numBlocks;
			if (totalAmountRaw > maxAmount - summand)
				return std::make_pair(Amount(), false);

			totalAmountRaw += summand;
			currentPair = pair;
		}

		auto isValid = Amount() == currentPair.second;
		return std::make_pair(Amount(isValid ? totalAmountRaw : 0), isValid);
	}

	void InflationCalculator::add(Height height, Amount amount) {
		if (Height() == height || (!m_inflationMap.empty() && (--m_inflationMap.cend())->first >= height))
			CATAPULT_THROW_INVALID_ARGUMENT_1("cannot add inflation entry (height)", height);

		m_inflationMap.emplace(height, amount);
	}
}}
