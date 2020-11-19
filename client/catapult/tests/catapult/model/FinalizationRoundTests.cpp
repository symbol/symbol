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

#include "catapult/model/FinalizationRound.h"
#include "tests/test/nodeps/Alignment.h"
#include "tests/test/nodeps/Comparison.h"

namespace catapult { namespace model {

#define TEST_CLASS FinalizationRoundTests

	// region size + alignment

#define FINALIZATION_ROUND_FIELDS FIELD(Epoch) FIELD(Point)

	TEST(TEST_CLASS, FinalizationRoundHasExpectedSize) {
		// Arrange:
		auto expectedSize = 0u;

#define FIELD(X) expectedSize += SizeOf32<decltype(FinalizationRound::X)>();
		FINALIZATION_ROUND_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(FinalizationRound));
		EXPECT_EQ(8u, sizeof(FinalizationRound));
	}

	TEST(TEST_CLASS, FinalizationRoundHasProperAlignment) {
#define FIELD(X) EXPECT_ALIGNED(FinalizationRound, X);
		FINALIZATION_ROUND_FIELDS
#undef FIELD

		EXPECT_EQ(0u, sizeof(FinalizationRound) % 8);
	}

#undef FINALIZATION_ROUND_FIELDS

	// endregion

	// region comparison operators

	namespace {
		std::vector<FinalizationRound> GenerateIncreasingFinalizationRoundValues() {
			return {
				{ FinalizationEpoch(7), FinalizationPoint(5) },
				{ FinalizationEpoch(7), FinalizationPoint(10) },
				{ FinalizationEpoch(7), FinalizationPoint(11) },
				{ FinalizationEpoch(8), FinalizationPoint(11) }
			};
		}
	}

	DEFINE_EQUALITY_AND_COMPARISON_TESTS(TEST_CLASS, GenerateIncreasingFinalizationRoundValues())

	// endregion

	// region arithmetic operators

	TEST(TEST_CLASS, CanAddPointToRound) {
		// Arrange:
		auto round = model::FinalizationRound{ FinalizationEpoch(21), FinalizationPoint(10) };

		// Act + Assert:
		EXPECT_EQ(FinalizationRound({ FinalizationEpoch(21), FinalizationPoint(11) }), round + FinalizationPoint(1));
		EXPECT_EQ(FinalizationRound({ FinalizationEpoch(21), FinalizationPoint(19) }), round + FinalizationPoint(9));
	}

	TEST(TEST_CLASS, CanSubtractPointFromRound) {
		// Arrange:
		auto round = model::FinalizationRound{ FinalizationEpoch(21), FinalizationPoint(10) };

		// Act + Assert:
		EXPECT_EQ(FinalizationRound({ FinalizationEpoch(21), FinalizationPoint(9) }), round - FinalizationPoint(1));
		EXPECT_EQ(FinalizationRound({ FinalizationEpoch(21), FinalizationPoint(1) }), round - FinalizationPoint(9));
	}

	// endregion

	// region to string

	TEST(TEST_CLASS, CanOutput) {
		// Arrange:
		auto round = FinalizationRound{ FinalizationEpoch(7), FinalizationPoint(11) };

		// Act:
		auto str = test::ToString(round);

		// Assert:
		EXPECT_EQ("(7, 11)", str);
	}

	// endregion
}}
