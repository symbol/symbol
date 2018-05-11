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

#include "catapult/utils/BaseValue.h"
#include "catapult/types.h"
#include "tests/test/nodeps/Comparison.h"
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
		TestValue data;

		// Act + Assert:
		EXPECT_EQ(0u, data.unwrap());
	}

	TEST(TEST_CLASS, CanStoreValue) {
		// Arrange:
		TestValue data(123);

		// Act + Assert:
		EXPECT_EQ(123u, data.unwrap());
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
		TestValue data(123);
		TestValue newData(642);

		// Act:
		const auto& assignResult = (data = newData);

		// Assert:
		EXPECT_EQ(642u, newData.unwrap());
		EXPECT_EQ(642u, data.unwrap());
		EXPECT_EQ(&data, &assignResult);
	}

	TEST(TEST_CLASS, CanCopyConstruct) {
		// Act:
		TestValue data(123);
		TestValue newData(data);

		// Assert:
		EXPECT_EQ(123u, newData.unwrap());
		EXPECT_EQ(123u, data.unwrap());
	}

	TEST(TEST_CLASS, CanMoveAssign) {
		// Arrange:
		TestValue data(123);
		TestValue newData(642);

		// Act:
		const auto& assignResult = (newData = std::move(data));

		// Assert:
		EXPECT_EQ(123u, newData.unwrap());
		EXPECT_EQ(&newData, &assignResult);
	}

	TEST(TEST_CLASS, CanMoveConstruct) {
		// Act:
		TestValue data(123);
		TestValue newData(std::move(data));

		// Assert:
		EXPECT_EQ(123u, newData.unwrap());
	}

	// endregion

	// region unwrap

	TEST(TEST_CLASS, CanUnwrapAndRetrieveRawValue) {
		// Arrange:
		TestValue data(123);

		// Act:
		auto rawValue = data.unwrap();

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
		TestValue data1(123);

		// Act:
		auto str = test::ToString(data1);

		// Assert:
		EXPECT_EQ("123", str);
	}

	// endregion

	// region addition and subtraction operators

	TEST(TEST_CLASS, CanAddBaseValues) {
		// Arrange:
		TestValue data1(123);
		TestValue data2(234);

		// Act:
		auto data3 = data1 + data2;

		// Assert:
		EXPECT_EQ(TestValue(123), data1);
		EXPECT_EQ(TestValue(234), data2);
		EXPECT_EQ(TestValue(357), data3);
	}

	TEST(TEST_CLASS, CanSubtractBaseValues) {
		// Arrange:
		TestValue data1(543);
		TestValue data2(123);

		// Act:
		auto data3 = data1 - data2;

		// Assert:
		EXPECT_EQ(TestValue(543), data1);
		EXPECT_EQ(TestValue(123), data2);
		EXPECT_EQ(TestValue(420), data3);
	}

	// endregion

	// region type convertibility

	TEST(TEST_CLASS, CanAssignAliasedType) {
		// Assert:
		bool result = std::is_convertible<TestValue, AliasedValue>::value;
		EXPECT_TRUE(result);
	}

	TEST(TEST_CLASS, CannotAssignUsingDifferentType) {
		// Assert:
		bool result = std::is_convertible<TestValue, SameSizeValue>::value;
		EXPECT_FALSE(result);
	}

	namespace {
		template<typename... T>
		struct Set {};

		template<typename T, typename U>
		void Create(T, U) {
			bool convertible = std::is_convertible<T, U>::value;
			if (std::is_same<T, U>::value) {
				EXPECT_TRUE(convertible);
			} else {
				EXPECT_FALSE(convertible);
			}
		}

		template<typename ElemOuter, typename Head, typename... Tail>
		void DispatchInner(ElemOuter, Set<Head, Tail...>) {
			Create(ElemOuter{}, Head{});
			DispatchInner(ElemOuter{}, Set<Tail...>{});
		}

		template<typename ElemOuter, typename... Tail>
		void DispatchInner(ElemOuter, Set<>) {
			// finished iteration of inner loop
		}

		template<typename TSetInner, typename Head, typename... Tail>
		void DispatchOuter(Set<Head, Tail...>, TSetInner) {
			DispatchInner(Head{}, TSetInner{});
			DispatchOuter(Set<Tail...>{}, TSetInner{});
		}

		template<typename TSetInner, typename... Tail>
		void DispatchOuter(Set<>, TSetInner) {
			// finished iteration of outer loop
		}

		template<typename TSet>
		void AssertCannotConvertTypes() {
			DispatchOuter(TSet{}, TSet{});
		}
	}

	TEST(TEST_CLASS, CatapultTypesTests) {
		AssertCannotConvertTypes<Set<Timestamp, Amount, Height, Difficulty>>();
	}

	// endregion
}}
