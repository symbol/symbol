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

#include "finalization/src/model/FinalizationRoundRange.h"
#include "tests/test/nodeps/Alignment.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS FinalizationRoundRangeTests

	// region constructor

	TEST(TEST_CLASS, CanCreateDefaultRange) {
		// Act:
		FinalizationRoundRange range;

		// Assert:
		EXPECT_EQ(FinalizationEpoch(0), range.Min.Epoch);
		EXPECT_EQ(FinalizationPoint(0), range.Min.Point);
		EXPECT_EQ(FinalizationEpoch(0), range.Max.Epoch);
		EXPECT_EQ(FinalizationPoint(0), range.Max.Point);
	}

	TEST(TEST_CLASS, CanCreateRangeAroundMinMax) {
		// Act:
		FinalizationRoundRange range({ FinalizationEpoch(7), FinalizationPoint(5) }, { FinalizationEpoch(11), FinalizationPoint(3) });

		// Assert:
		EXPECT_EQ(FinalizationEpoch(7), range.Min.Epoch);
		EXPECT_EQ(FinalizationPoint(5), range.Min.Point);
		EXPECT_EQ(FinalizationEpoch(11), range.Max.Epoch);
		EXPECT_EQ(FinalizationPoint(3), range.Max.Point);
	}

	// endregion

	// region size + alignment

#define FINALIZATION_ROUND_RANGE_FIELDS FIELD(Min) FIELD(Max)

	TEST(TEST_CLASS, FinalizationRoundRangeHasExpectedSize) {
		// Arrange:
		auto expectedSize = 0u;

#define FIELD(X) expectedSize += SizeOf32<decltype(FinalizationRoundRange::X)>();
		FINALIZATION_ROUND_RANGE_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(FinalizationRoundRange));
		EXPECT_EQ(16u, sizeof(FinalizationRoundRange));
	}

	TEST(TEST_CLASS, FinalizationRoundRangeHasProperAlignment) {
#define FIELD(X) EXPECT_ALIGNED(FinalizationRoundRange, X);
		FINALIZATION_ROUND_RANGE_FIELDS
#undef FIELD

		EXPECT_EQ(0u, sizeof(FinalizationRoundRange) % 8);
	}

#undef FINALIZATION_ROUND_RANGE_FIELDS

	// endregion

	// region equality operators

	namespace {
		FinalizationRoundRange CreateRange(uint32_t minEpoch, uint32_t minPoint, uint32_t maxEpoch, uint32_t maxPoint) {
			return FinalizationRoundRange(
					{ FinalizationEpoch(minEpoch), FinalizationPoint(minPoint) },
					{ FinalizationEpoch(maxEpoch), FinalizationPoint(maxPoint) });
		}

		std::unordered_set<std::string> GetEqualTags() {
			return { "default", "copy" };
		}

		std::unordered_map<std::string, FinalizationRoundRange> GenerateEqualityInstanceMap() {
			return {
				{ "default", CreateRange(7, 6, 11, 3) },
				{ "copy", CreateRange(7, 6, 11, 3) },
				{ "diff min", CreateRange(7, 7, 11, 3) },
				{ "diff max", CreateRange(7, 6, 10, 3) }
			};
		}
	}

	TEST(TEST_CLASS, OperatorEqualReturnsTrueOnlyForEqualValues) {
		test::AssertOperatorEqualReturnsTrueForEqualObjects("default", GenerateEqualityInstanceMap(), GetEqualTags());
	}

	TEST(TEST_CLASS, OperatorNotEqualReturnsTrueOnlyForUnequalValues) {
		test::AssertOperatorNotEqualReturnsTrueForUnequalObjects("default", GenerateEqualityInstanceMap(), GetEqualTags());
	}

	// endregion

	// region to string

	TEST(TEST_CLASS, CanOutput) {
		// Arrange:
		auto range = CreateRange(7, 6, 11, 3);

		// Act:
		auto str = test::ToString(range);

		// Assert:
		EXPECT_EQ("[(7, 6), (11, 3)]", str);
	}

	// endregion

	// region IsInRange

	TEST(TEST_CLASS, IsInRangeReturnsTrueWhenRoundIsInRange) {
		// Arrange:
		auto range = CreateRange(7, 6, 11, 3);

		// Act + Assert:
		EXPECT_TRUE(IsInRange(range, { FinalizationEpoch(7), FinalizationPoint(6) }));
		EXPECT_TRUE(IsInRange(range, { FinalizationEpoch(7), FinalizationPoint(7) }));
		EXPECT_TRUE(IsInRange(range, { FinalizationEpoch(7), FinalizationPoint(9) }));
		EXPECT_TRUE(IsInRange(range, { FinalizationEpoch(9), FinalizationPoint(0) }));
		EXPECT_TRUE(IsInRange(range, { FinalizationEpoch(9), FinalizationPoint(9) }));
		EXPECT_TRUE(IsInRange(range, { FinalizationEpoch(11), FinalizationPoint(2) }));
		EXPECT_TRUE(IsInRange(range, { FinalizationEpoch(11), FinalizationPoint(3) }));
	}

	TEST(TEST_CLASS, IsInRangeReturnsFalseWhenRoundIsNotInRange) {
		// Arrange:
		auto range = CreateRange(7, 6, 11, 3);

		// Act + Assert:
		EXPECT_FALSE(IsInRange(range, { FinalizationEpoch(5), FinalizationPoint(7) }));
		EXPECT_FALSE(IsInRange(range, { FinalizationEpoch(7), FinalizationPoint(0) }));
		EXPECT_FALSE(IsInRange(range, { FinalizationEpoch(7), FinalizationPoint(5) }));
		EXPECT_FALSE(IsInRange(range, { FinalizationEpoch(11), FinalizationPoint(4) }));
		EXPECT_FALSE(IsInRange(range, { FinalizationEpoch(11), FinalizationPoint(9) }));
		EXPECT_FALSE(IsInRange(range, { FinalizationEpoch(15), FinalizationPoint(4) }));
	}

	// endregion
}}
