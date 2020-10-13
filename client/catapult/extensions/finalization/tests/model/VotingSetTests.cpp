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

#include "finalization/src/model/VotingSet.h"
#include "catapult/model/HeightGrouping.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS VotingSetTests

	// region CalculateVotingSetStartHeight

	TEST(TEST_CLASS, CalculateVotingSetStartHeight_DoesNotSupportZeroGrouping) {
		EXPECT_THROW(CalculateVotingSetStartHeight(FinalizationEpoch(1), 0), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CalculateVotingSetStartHeight_DoesNotSupportZeroEpoch) {
		EXPECT_THROW(CalculateVotingSetStartHeight(FinalizationEpoch(), 20), catapult_invalid_argument);

		EXPECT_THROW(CalculateVotingSetStartHeight(FinalizationEpoch(), 50), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CalculateVotingSetStartHeight_SupportsEpochOne) {
		EXPECT_EQ(Height(1), CalculateVotingSetStartHeight(FinalizationEpoch(1), 20));

		EXPECT_EQ(Height(1), CalculateVotingSetStartHeight(FinalizationEpoch(1), 50));
	}

	TEST(TEST_CLASS, CalculateVotingSetStartHeight_SupportsEpochsGreaterThanOne) {
		EXPECT_EQ(Height(2), CalculateVotingSetStartHeight(FinalizationEpoch(2), 20));
		EXPECT_EQ(Height(21), CalculateVotingSetStartHeight(FinalizationEpoch(3), 20));
		EXPECT_EQ(Height(141), CalculateVotingSetStartHeight(FinalizationEpoch(9), 20));

		EXPECT_EQ(Height(2), CalculateVotingSetStartHeight(FinalizationEpoch(2), 50));
		EXPECT_EQ(Height(51), CalculateVotingSetStartHeight(FinalizationEpoch(3), 50));
		EXPECT_EQ(Height(351), CalculateVotingSetStartHeight(FinalizationEpoch(9), 50));
	}

	// endregion

	// region CalculateVotingSetEndHeight

	TEST(TEST_CLASS, CalculateVotingSetEndHeight_DoesNotSupportZeroGrouping) {
		EXPECT_THROW(CalculateVotingSetEndHeight(FinalizationEpoch(1), 0), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CalculateVotingSetEndHeight_DoesNotSupportZeroEpoch) {
		EXPECT_THROW(CalculateVotingSetEndHeight(FinalizationEpoch(), 20), catapult_invalid_argument);

		EXPECT_THROW(CalculateVotingSetEndHeight(FinalizationEpoch(), 50), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CalculateVotingSetEndHeight_SupportsEpochOne) {
		EXPECT_EQ(Height(1), CalculateVotingSetEndHeight(FinalizationEpoch(1), 20));

		EXPECT_EQ(Height(1), CalculateVotingSetEndHeight(FinalizationEpoch(1), 50));
	}

	TEST(TEST_CLASS, CalculateVotingSetEndHeight_SupportsEpochsGreaterThanOne) {
		EXPECT_EQ(Height(20), CalculateVotingSetEndHeight(FinalizationEpoch(2), 20));
		EXPECT_EQ(Height(40), CalculateVotingSetEndHeight(FinalizationEpoch(3), 20));
		EXPECT_EQ(Height(160), CalculateVotingSetEndHeight(FinalizationEpoch(9), 20));

		EXPECT_EQ(Height(50), CalculateVotingSetEndHeight(FinalizationEpoch(2), 50));
		EXPECT_EQ(Height(100), CalculateVotingSetEndHeight(FinalizationEpoch(3), 50));
		EXPECT_EQ(Height(400), CalculateVotingSetEndHeight(FinalizationEpoch(9), 50));
	}

	// endregion

	// region CalculateFinalizationEpochForHeight

	TEST(TEST_CLASS, CalculateFinalizationEpochForHeight_DoesNotSupportZeroGrouping) {
		EXPECT_THROW(CalculateFinalizationEpochForHeight(Height(1), 0), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CalculateFinalizationEpochForHeight_DoesNotSupportZeroHeight) {
		EXPECT_THROW(CalculateFinalizationEpochForHeight(Height(), 20), catapult_invalid_argument);

		EXPECT_THROW(CalculateFinalizationEpochForHeight(Height(), 50), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CalculateFinalizationEpochForHeight_SupportsHeightOne) {
		EXPECT_EQ(FinalizationEpoch(1), CalculateFinalizationEpochForHeight(Height(1), 20));

		EXPECT_EQ(FinalizationEpoch(1), CalculateFinalizationEpochForHeight(Height(1), 50));
	}

	TEST(TEST_CLASS, CalculateFinalizationEpochForHeight_SupportsEpochsGreaterThanOne) {
		EXPECT_EQ(FinalizationEpoch(2), CalculateFinalizationEpochForHeight(Height(2), 20));
		EXPECT_EQ(FinalizationEpoch(2), CalculateFinalizationEpochForHeight(Height(17), 20));
		EXPECT_EQ(FinalizationEpoch(2), CalculateFinalizationEpochForHeight(Height(20), 20));
		EXPECT_EQ(FinalizationEpoch(3), CalculateFinalizationEpochForHeight(Height(21), 20));

		EXPECT_EQ(FinalizationEpoch(2), CalculateFinalizationEpochForHeight(Height(2), 50));
		EXPECT_EQ(FinalizationEpoch(2), CalculateFinalizationEpochForHeight(Height(17), 50));
		EXPECT_EQ(FinalizationEpoch(2), CalculateFinalizationEpochForHeight(Height(50), 50));
		EXPECT_EQ(FinalizationEpoch(3), CalculateFinalizationEpochForHeight(Height(51), 50));
	}

	// endregion

	// region consistency

	TEST(TEST_CLASS, VotingSetStartAndEndHeightsAreConsistent) {
		// Arrange:
		constexpr auto Grouping = 50u;
		for (auto i = 2u; i <= 100; ++i) {
			// Act:
			auto previousEndHeight = CalculateVotingSetEndHeight(FinalizationEpoch(i - 1), Grouping);
			auto startHeight = CalculateVotingSetStartHeight(FinalizationEpoch(i), Grouping);

			// Assert:
			EXPECT_EQ(previousEndHeight + Height(1), startHeight) << i;
		}
	}

	TEST(TEST_CLASS, VotingSetStartAndEndHeightsAreConsistentWithGroupedHeight) {
		// Arrange:
		constexpr auto Grouping = 50u;
		for (auto i = 2u; i <= 100; ++i) {
			// Act:
			auto previousEndHeight = CalculateVotingSetEndHeight(FinalizationEpoch(i - 1), Grouping);
			auto startHeight = CalculateVotingSetStartHeight(FinalizationEpoch(i), Grouping);
			auto groupedStartHeight = CalculateGroupedHeight<Height>(startHeight, Grouping);

			// Assert:
			EXPECT_EQ(previousEndHeight, groupedStartHeight) << i;
		}
	}

	TEST(TEST_CLASS, CanGetConsistentEpochFromVotingSetStartHeight) {
		// Arrange:
		constexpr auto Grouping = 50u;
		for (auto i = 2u; i <= 100; ++i) {
			// Act:
			auto startHeight = CalculateVotingSetStartHeight(FinalizationEpoch(i), Grouping);
			auto epoch = CalculateFinalizationEpochForHeight(startHeight, Grouping);

			// Assert:
			EXPECT_EQ(FinalizationEpoch(i), epoch) << i;
		}
	}

	TEST(TEST_CLASS, CanGetConsistentEpochFromVotingSetEndHeight) {
		// Arrange:
		constexpr auto Grouping = 50u;
		for (auto i = 2u; i <= 100; ++i) {
			// Act:
			auto endHeight = CalculateVotingSetEndHeight(FinalizationEpoch(i), Grouping);
			auto epoch = CalculateFinalizationEpochForHeight(endHeight, Grouping);

			// Assert:
			EXPECT_EQ(FinalizationEpoch(i), epoch) << i;
		}
	}

	// endregion
}}
