#include "catapult/utils/BaseValue.h"
#include "catapult/types.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {
	namespace {
		struct TestValue_tag {};
		using TestValue = BaseValue<uint64_t, TestValue_tag>;

		using AliasedValue = TestValue;

		struct SameSizeValue_tag {};
		using SameSizeValue = BaseValue<uint64_t, SameSizeValue_tag>;
	}

	TEST(BaseValueTests, DefaultValueIsZeroInitialized) {
		// Arrange:
		TestValue data;

		// Act + Assert:
		EXPECT_EQ(0u, data.unwrap());
	}

	TEST(BaseValueTests, CanStoreValue) {
		// Arrange:
		TestValue data(123);

		// Act + Assert:
		EXPECT_EQ(123u, data.unwrap());
	}

	TEST(ImmutableValueTests, CanStoreValueAsConstexpr) {
		// Act:
		constexpr TestValue Const_Data(123);

		// Assert:
		EXPECT_EQ(TestValue(123u), Const_Data);
	}

	TEST(BaseValueTests, CanCopyAssign) {
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

	TEST(BaseValueTests, CanCopyConstruct) {
		// Act:
		TestValue data(123);
		TestValue newData(data);

		// Assert:
		EXPECT_EQ(123u, newData.unwrap());
		EXPECT_EQ(123u, data.unwrap());
	}

	TEST(BaseValueTests, CanMoveAssign) {
		// Arrange:
		TestValue data(123);
		TestValue newData(642);

		// Act
		const auto& assignResult = (newData = std::move(data));

		// Assert:
		EXPECT_EQ(123u, newData.unwrap());
		EXPECT_EQ(&newData, &assignResult);
	}

	TEST(BaseValueTests, CanMoveConstruct) {
		// Act:
		TestValue data(123);
		TestValue newData(std::move(data));

		// Assert:
		EXPECT_EQ(123u, newData.unwrap());
	}

	TEST(BaseValueTests, CanUnwrapAndRetrieveRawValue) {
		// Arrange:
		TestValue data(123);

		// Act:
		auto rawValue = data.unwrap();

		// Assert:
		EXPECT_EQ(123u, rawValue);
	}

	// region comparison operators

	TEST(BaseValueTests, OperatorEqualReturnsTrueOnlyForEqualValues) {
		// Arrange:
		TestValue data1(123);
		TestValue data2(123);
		TestValue data3(642);

		EXPECT_TRUE(data1 == data2);
		EXPECT_FALSE(data1 == data3);
		EXPECT_FALSE(data3 == data1);
	}

	TEST(BaseValueTests, OperatorNotEqualReturnsTrueOnlyForUnequalValues) {
		// Arrange:
		TestValue data1(123);
		TestValue data2(123);
		TestValue data3(642);

		EXPECT_FALSE(data1 != data2);
		EXPECT_TRUE(data1 != data3);
		EXPECT_TRUE(data3 != data1);
	}

	TEST(BaseValueTests, OperatorLessThanReturnsTrueOnlyForSmallerValues) {
		// Arrange:
		TestValue data1(123);
		TestValue data2(123);
		TestValue data3(642);

		EXPECT_FALSE(data1 < data2);
		EXPECT_TRUE(data1 < data3);
		EXPECT_FALSE(data3 < data1);
	}

	TEST(BaseValueTests, OperatorLessThanOrEqualReturnsTrueOnlyForSmallerOrEqualValues) {
		// Arrange:
		TestValue data1(123);
		TestValue data2(123);
		TestValue data3(642);

		EXPECT_TRUE(data1 <= data2);
		EXPECT_TRUE(data1 <= data3);
		EXPECT_FALSE(data3 <= data1);
	}

	TEST(BaseValueTests, OperatorGreaterThanReturnsTrueOnlyForGreaterValues) {
		// Arrange:
		TestValue data1(123);
		TestValue data2(123);
		TestValue data3(642);

		EXPECT_FALSE(data1 > data2);
		EXPECT_FALSE(data1 > data3);
		EXPECT_TRUE(data3 > data1);
	}

	TEST(BaseValueTests, OperatorGreaterThanOrEqualReturnsTrueOnlyForGreaterOrEqualValues) {
		// Arrange:
		TestValue data1(123);
		TestValue data2(123);
		TestValue data3(642);

		EXPECT_TRUE(data1 >= data2);
		EXPECT_FALSE(data1 >= data3);
		EXPECT_TRUE(data3 >= data1);
	}

	// endregion

	// region addition and subtraction operators

	TEST(BaseValueTests, CanAddBaseValues) {
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

	TEST(BaseValueTests, CanSubtractBaseValues) {
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

	TEST(BaseValueTests, CanAssignAliasedType) {
		// Assert:
		bool result = std::is_convertible<TestValue, AliasedValue>::value;
		EXPECT_TRUE(result);
	}

	TEST(BaseValueTests, CannotAssignUsingDifferentType) {
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

	TEST(BaseValueTests, CatapultTypesTests) {
		AssertCannotConvertTypes<Set<Timestamp, Amount, Height, Difficulty>>();
	}
}}
