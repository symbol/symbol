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

#include "AccountStateCacheTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	state::BalanceHistory CreateBalanceHistory(const std::vector<std::pair<Height, Amount>>& balancePairs) {
		state::BalanceHistory history;
		for (const auto& pair : balancePairs)
			history.add(pair.first, pair.second);

		// Sanity: all pairs were added
		EXPECT_EQ(balancePairs.size(), history.size());
		return history;
	}

	cache::AddressBalanceHistoryMap GenerateBalanceHistories(const AddressBalanceHistorySeeds& seeds) {
		cache::AddressBalanceHistoryMap map;

		for (const auto& seed : seeds)
			map.emplace(Address{ { seed.first } }, CreateBalanceHistory(seed.second));

		return map;
	}

	void AssertEqual(const cache::AddressBalanceHistoryMap& expected, const cache::AddressBalanceHistoryMap& actual) {
		EXPECT_EQ(expected.size(), actual.size());

		for (const auto& pair : expected) {
			auto actualIter = actual.find(pair.first);
			if (actual.cend() == actualIter) {
				EXPECT_NE(actual.cend(), actualIter) << pair.first << " in expected but not in actual";
				continue;
			}

			const auto& expectedBalanceHistory = pair.second;
			const auto& actualBalanceHistory = actualIter->second;
			EXPECT_EQ(expectedBalanceHistory.heights(), actualBalanceHistory.heights()) << "address = " << pair.first;

			for (auto height : expectedBalanceHistory.heights()) {
				EXPECT_EQ(expectedBalanceHistory.balance(height), actualBalanceHistory.balance(height))
						<< "address = " << pair.first << ", height = " << height;
			}
		}
	}
}}
