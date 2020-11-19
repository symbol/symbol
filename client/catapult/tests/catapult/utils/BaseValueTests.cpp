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

#include "catapult/utils/BaseValue.h"
#include "catapult/types.h"
#include "tests/test/nodeps/Comparison.h"
#include "tests/test/nodeps/Convertibility.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS BaseValueTests

	namespace {
		struct TestValue_tag {};
		using TestValue = BaseValue<uint64_t, TestValue_tag>;

		using AliasedValue = TestValue;

		struct SameSizeValue_tag {};
		using SameSizeValue = BaseValue<uint64_t, SameSizeValue_tag>;
	}

	// region constructor

	TEST(TEST_CLASS, DefaultValueIsZeroInitialized) {
		// Arrange:
		TestValue value;

		// Act + Assert:
		EXPECT_EQ(0u, value.unwrap());
	}

	TEST(TEST_CLASS, CanStoreValue) {
		// Arrange:
		TestValue value(123);

		// Act + Assert:
		EXPECT_EQ(123u, value.unwrap());
	}

	TEST(TEST_CLASS, CanStoreValueAsConstexpr) {
		// Act:
		constexpr TestValue Const_Data(123);

		// Assert:
		EXPECT_EQ(TestValue(123), Const_Data);
	}

	// endregion

	// region copy + assign

	TEST(TEST_CLASS, CanCopyAssign) {
		// Arrange:
		TestValue value(123);
		TestValue newValue(642);

		// Act:
		const auto& assignResult = (value = newValue);

		// Assert:
		EXPECT_EQ(642u, newValue.unwrap());
		EXPECT_EQ(642u, value.unwrap());
		EXPECT_EQ(&value, &assignResult);
	}

	TEST(TEST_CLASS, CanCopyConstruct) {
		// Act:
		TestValue value(123);
		TestValue newValue(value);

		// Assert:
		EXPECT_EQ(123u, newValue.unwrap());
		EXPECT_EQ(123u, value.unwrap());
	}

	TEST(TEST_CLASS, CanMoveAssign) {
		// Arrange:
		TestValue value(123);
		TestValue newValue(642);

		// Act:
		const auto& assignResult = (newValue = std::move(value));

		// Assert:
		EXPECT_EQ(123u, newValue.unwrap());
		EXPECT_EQ(&newValue, &assignResult);
	}

	TEST(TEST_CLASS, CanMoveConstruct) {
		// Act:
		TestValue value(123);
		TestValue newValue(std::move(value));

		// Assert:
		EXPECT_EQ(123u, newValue.unwrap());
	}

	// endregion

	// region unwrap

	TEST(TEST_CLASS, CanUnwrapAndRetrieveRawValue) {
		// Arrange:
		TestValue value(123);

		// Act:
		auto rawValue = value.unwrap();

		// Assert:
		EXPECT_EQ(123u, rawValue);
	}

	// endregion

	// region comparison operators

	namespace {
		std::vector<TestValue> GenerateIncreasingValues() {
			return { TestValue(123), TestValue(642), TestValue(989) };
		}
	}

	DEFINE_EQUALITY_AND_COMPARISON_TESTS(TEST_CLASS, GenerateIncreasingValues())

	// endregion

	// region to string

	TEST(TEST_CLASS, CanOutputBaseValue) {
		// Arrange:
		TestValue value1(123);

		// Act:
		auto str = test::ToString(value1);

		// Assert:
		EXPECT_EQ("123", str);
	}

	// endregion

	// region addition and subtraction operators

	TEST(TEST_CLASS, CanAddBaseValues) {
		// Arrange:
		TestValue value1(123);
		TestValue value2(234);

		// Act:
		auto value3 = value1 + value2;

		// Assert:
		EXPECT_EQ(TestValue(123), value1);
		EXPECT_EQ(TestValue(234), value2);
		EXPECT_EQ(TestValue(357), value3);
	}

	TEST(TEST_CLASS, CanSubtractBaseValues) {
		// Arrange:
		TestValue value1(543);
		TestValue value2(123);

		// Act:
		auto value3 = value1 - value2;

		// Assert:
		EXPECT_EQ(TestValue(543), value1);
		EXPECT_EQ(TestValue(123), value2);
		EXPECT_EQ(TestValue(420), value3);
	}

	// endregion

	// region type convertibility

	TEST(TEST_CLASS, CanAssignAliasedType) {
		auto isConvertible = std::is_convertible_v<TestValue, AliasedValue>;
		EXPECT_TRUE(isConvertible);
	}

	TEST(TEST_CLASS, CannotAssignUsingDifferentType) {
		auto isConvertible = std::is_convertible_v<TestValue, SameSizeValue>;
		EXPECT_FALSE(isConvertible);
	}

	TEST(TEST_CLASS, CatapultTypesTests) {
		test::TypeConvertibilityTests::AssertCannotConvertTypes<Timestamp, Amount, Height, Difficulty>();
	}

	// endregion
}}
