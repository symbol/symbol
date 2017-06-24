#include "catapult/utils/ImmutableValue.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

	TEST(ImmutableValueTests, CanCreateAsConstexpr) {
		// Act:
		constexpr ImmutableValue<uint32_t> Const_Value(78u);

		// Assert:
		EXPECT_EQ(78u, Const_Value);
	}

	TEST(ImmutableValueTests, CanCreateAroundMutableValue) {
		// Act:
		ImmutableValue<uint32_t> value(78u);

		// Assert:
		EXPECT_EQ(78u, value);
	}

	TEST(ImmutableValueTests, CanMoveConstruct) {
		// Act:
		ImmutableValue<uint32_t> value(78u);
		ImmutableValue<uint32_t> value2(std::move(value));

		// Assert:
		EXPECT_EQ(78u, value2);
	}

	TEST(ImmutableValueTests, CanMoveAssign) {
		// Act:
		ImmutableValue<uint32_t> value(78u);
		ImmutableValue<uint32_t> value2(12u);
		const auto& assignResult = (value2 = std::move(value));

		// Assert:
		EXPECT_EQ(78u, value2);
		EXPECT_EQ(&value2, &assignResult);
	}

	TEST(ImmutableValueTests, CanCastToMutableValue) {
		// Arrange:
		ImmutableValue<uint32_t> value(78u);

		// Act:
		auto rawValue = static_cast<uint32_t>(78u);

		// Assert:
		EXPECT_EQ(78u, rawValue);
	}
}}
