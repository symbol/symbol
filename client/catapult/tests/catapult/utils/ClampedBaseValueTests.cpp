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

#include "catapult/utils/ClampedBaseValue.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS ClampedBaseValueTests

	namespace {
		struct TestValueRange {
			static constexpr int64_t Default_Value = 3;
			static constexpr int64_t Min_Value = -17;
			static constexpr int64_t Max_Value = 100;
		};

		using TestValue = ClampedBaseValue<int64_t, TestValueRange>;
	}

	TEST(TEST_CLASS, DefaultValueIsInitializedToRangeDefaultValue) {
		// Arrange:
		TestValue value;

		// Act + Assert:
		EXPECT_EQ(3, value.unwrap());
	}

	TEST(TEST_CLASS, ConstantsAreInitializedCorrectly) {
		EXPECT_EQ(TestValue(-17), TestValue::Min());
		EXPECT_EQ(TestValue(100), TestValue::Max());
	}

	TEST(TEST_CLASS, UnclampedTypeDoesNotClampValues) {
		EXPECT_EQ(-1000, TestValue::Unclamped(-1000).unwrap());
		EXPECT_EQ(1000, TestValue::Unclamped(1000).unwrap());
	}

	namespace {
		void AssertClampedValue(int64_t seed, int64_t expected) {
			// Act:
			TestValue value(seed);

			// Assert:
			EXPECT_EQ(expected, value.unwrap()) << "for seed " << seed;
		}
	}

	TEST(TEST_CLASS, ClampedValueCannotBeSetBelowMin) {
		std::vector<int64_t> deltas{ 1, 1000 };
		for (auto delta : deltas)
			AssertClampedValue(TestValueRange::Min_Value - delta, TestValueRange::Min_Value);
	}

	TEST(TEST_CLASS, ClampedValueCannotBeSetAboveMax) {
		std::vector<int64_t> deltas{ 1, 1000 };
		for (auto delta : deltas)
			AssertClampedValue(TestValueRange::Max_Value + delta, TestValueRange::Max_Value);
	}

	TEST(TEST_CLASS, ClampedValueCanBeSetInAllowableRange) {
		// Assert: none of the values get clamped
		std::vector<int64_t> values{ TestValueRange::Min_Value, -16, -8, 0, 50, 99, TestValueRange::Max_Value };
		for (auto value : values)
			AssertClampedValue(value, value);
	}

	// region copy construction / assignment

	TEST(TEST_CLASS, CanCopyConstructValues) {
		// Act:
		TestValue v1(5);
		TestValue v2 = v1;

		// ASSERT
		EXPECT_EQ(v2, v1);
		EXPECT_EQ(5, v1.unwrap());
		EXPECT_EQ(5, v2.unwrap());
	}

	TEST(TEST_CLASS, CanAssignValues) {
		// Act:
		TestValue v1(5);
		TestValue v2(7);
		v2 = v1;

		// ASSERT
		EXPECT_EQ(v2, v1);
		EXPECT_EQ(5, v1.unwrap());
		EXPECT_EQ(5, v2.unwrap());
	}

	// endregion

	// region addition and subtraction operators

	TEST(TEST_CLASS, CanAddClampedValues) {
		// Arrange:
		TestValue data1(-1);
		TestValue::Unclamped data2(TestValue::Max().unwrap() + 20);

		// Act:
		auto result = data1 + data2;

		// Assert: second operand is unclamped but result is clamped
		EXPECT_EQ(TestValue::Max(), result);
		EXPECT_LT(TestValue::Max().unwrap(), data2.unwrap());
	}

	TEST(TEST_CLASS, CanSubtractClampedValues) {
		// Arrange:
		TestValue data1(23);
		TestValue data2(70);

		// Act:
		auto result = data1 - data2;

		// Assert: result is unclamped
		EXPECT_EQ(-47, result.unwrap());
		EXPECT_GT(TestValue::Min().unwrap(), result.unwrap());
	}

	// endregion
}}
