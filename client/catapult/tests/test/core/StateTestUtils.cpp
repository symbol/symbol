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

#include "StateTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	state::CatapultState CreateRandomCatapultState() {
		auto state = state::CatapultState();
		state.LastRecalculationHeight = test::GenerateRandomValue<model::ImportanceHeight>();
		state.LastFinalizedHeight = test::GenerateRandomValue<Height>();
		state.DynamicFeeMultiplier = test::GenerateRandomValue<BlockFeeMultiplier>();
		state.NumTotalTransactions = test::Random();
		return state;
	}

	state::CatapultState CreateDeterministicCatapultState() {
		auto state = state::CatapultState();
		state.LastRecalculationHeight = model::ImportanceHeight(12345);
		state.LastFinalizedHeight = Height(9876);
		state.DynamicFeeMultiplier = BlockFeeMultiplier(334455);
		state.NumTotalTransactions = 7654321;
		return state;
	}

	void AssertEqual(const state::CatapultState& expected, const state::CatapultState& actual, const std::string& message) {
		EXPECT_EQ(expected.LastRecalculationHeight, actual.LastRecalculationHeight) << message;
		EXPECT_EQ(expected.LastFinalizedHeight, actual.LastFinalizedHeight) << message;
		EXPECT_EQ(expected.DynamicFeeMultiplier, actual.DynamicFeeMultiplier) << message;
		EXPECT_EQ(expected.NumTotalTransactions, actual.NumTotalTransactions) << message;
	}
}}
