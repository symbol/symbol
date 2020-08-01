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

#include "catapult/utils/IntegerMath.h"
#include "tests/test/nodeps/Random.h"
#include "tests/TestHarness.h"
#include <cmath>

namespace catapult { namespace utils {

#define TEST_CLASS IntegerMathTests

	// region CheckedAdd

	namespace {
		template<typename T>
		struct CheckedAddExample {
			T Value;
			T Delta;
			T ExpectedValue;
		};

		template<typename T>
		void AssertCheckedAddCanAdd(const std::vector<CheckedAddExample<T>>& examples) {
			// Arrange:
			for (const auto& example : examples) {
				auto value = example.Value;

				// Act:
				auto isAddSuccess = CheckedAdd(value, example.Delta);

				// Assert:
				EXPECT_TRUE(isAddSuccess) << HexFormat(example.Value);
				EXPECT_EQ(example.ExpectedValue, value) << HexFormat(example.Value);
			}
		}

		template<typename T>
		void AssertCheckedAddCannotAdd(const std::vector<CheckedAddExample<T>>& examples) {
			// Arrange:
			for (const auto& example : examples) {
				auto value = example.Value;

				// Act:
				auto isAddSuccess = CheckedAdd(value, example.Delta);

				// Assert:
				EXPECT_FALSE(isAddSuccess) << HexFormat(example.Value);
				EXPECT_EQ(example.Value, value) << HexFormat(example.Value);
			}
		}

		struct TwoByteValue_tag {};
		using TwoByteValue = BaseValue<uint16_t, TwoByteValue_tag>;
	}

	TEST(TEST_CLASS, CheckedAddCanAddValuesBelowMax_Uint32) {
		AssertCheckedAddCanAdd<uint32_t>({
			{ 0xFFFF'FFFE, 0x0000'0000, 0xFFFF'FFFE },
			{ 0xFFFF'FFFD, 0x0000'0001, 0xFFFF'FFFE },
			{ 0x0000'0000, 0xFFFF'FFFE, 0xFFFF'FFFE },
			{ 0xABCD'9876, 0x0000'1230, 0xABCD'AAA6 },
			{ 0xFFFF'0000, 0x0000'FFFE, 0xFFFF'FFFE }
		});
	}

	TEST(TEST_CLASS, CheckedAddCanAddValuesBelowMax_Uint16) {
		AssertCheckedAddCanAdd<uint16_t>({
			{ 0xFFFE, 0x0000, 0xFFFE },
			{ 0xFFFD, 0x0001, 0xFFFE },
			{ 0x0000, 0xFFFE, 0xFFFE },
			{ 0xABCD, 0x1111, 0xBCDE },
			{ 0xFF00, 0x00FE, 0xFFFE }
		});
	}

	TEST(TEST_CLASS, CheckedAddCanAddValuesBelowMax_BaseValue) {
		AssertCheckedAddCanAdd<TwoByteValue>({
			{ TwoByteValue(0xFFFE), TwoByteValue(0x0000), TwoByteValue(0xFFFE) },
			{ TwoByteValue(0xFFFD), TwoByteValue(0x0001), TwoByteValue(0xFFFE) },
			{ TwoByteValue(0x0000), TwoByteValue(0xFFFE), TwoByteValue(0xFFFE) },
			{ TwoByteValue(0xABCD), TwoByteValue(0x1111), TwoByteValue(0xBCDE) },
			{ TwoByteValue(0xFF00), TwoByteValue(0x00FE), TwoByteValue(0xFFFE) }
		});
	}

	TEST(TEST_CLASS, CheckedAddCanAddValuesUpToMax_Uint32) {
		AssertCheckedAddCanAdd<uint32_t>({
			{ 0xFFFF'FFFF, 0x0000'0000, 0xFFFF'FFFF },
			{ 0xFFFF'FFFE, 0x0000'0001, 0xFFFF'FFFF },
			{ 0x0000'0000, 0xFFFF'FFFF, 0xFFFF'FFFF },
			{ 0xABCD'9876, 0x5432'6789, 0xFFFF'FFFF },
			{ 0xFFFF'0000, 0x0000'FFFF, 0xFFFF'FFFF }
		});
	}

	TEST(TEST_CLASS, CheckedAddCanAddValuesUpToMax_Uint16) {
		AssertCheckedAddCanAdd<uint16_t>({
			{ 0xFFFF, 0x0000, 0xFFFF },
			{ 0xFFFE, 0x0001, 0xFFFF },
			{ 0x0000, 0xFFFF, 0xFFFF },
			{ 0xABCD, 0x5432, 0xFFFF },
			{ 0xFF00, 0x00FF, 0xFFFF }
		});
	}

	TEST(TEST_CLASS, CheckedAddCanAddValuesUpToMax_BaseValue) {
		AssertCheckedAddCanAdd<TwoByteValue>({
			{ TwoByteValue(0xFFFF), TwoByteValue(0x0000), TwoByteValue(0xFFFF) },
			{ TwoByteValue(0xFFFE), TwoByteValue(0x0001), TwoByteValue(0xFFFF) },
			{ TwoByteValue(0x0000), TwoByteValue(0xFFFF), TwoByteValue(0xFFFF) },
			{ TwoByteValue(0xABCD), TwoByteValue(0x5432), TwoByteValue(0xFFFF) },
			{ TwoByteValue(0xFF00), TwoByteValue(0x00FF), TwoByteValue(0xFFFF) }
		});
	}

	TEST(TEST_CLASS, CheckedAddCannotAddValuesAboveMax_Uint32) {
		// Arrange: third value is ignored
		AssertCheckedAddCannotAdd<uint32_t>({
			{ 0xFFFF'FFFF, 0x0000'0001, 0 },
			{ 0x0000'0001, 0xFFFF'FFFF, 0 },
			{ 0xABCD'9876, 0x5432'678A, 0 },
			{ 0xFFFF'0001, 0x0000'FFFF, 0 },
			{ 0xFFFF'FFFE, 0xFFFF'FFFE, 0 }
		});
	}

	TEST(TEST_CLASS, CheckedAddCannotAddValuesAboveMax_Uint16) {
		// Arrange: third value is ignored
		AssertCheckedAddCannotAdd<uint16_t>({
			{ 0xFFFF, 0x0001, 0 },
			{ 0x0001, 0xFFFF, 0 },
			{ 0xABCD, 0x5433, 0 },
			{ 0xFF01, 0x00FF, 0 },
			{ 0xFFFE, 0xFFFE, 0 }
		});
	}

	TEST(TEST_CLASS, CheckedAddCannotAddValuesAboveMax_BaseValue) {
		// Arrange: third value is ignored
		AssertCheckedAddCannotAdd<TwoByteValue>({
			{ TwoByteValue(0xFFFF), TwoByteValue(0x0001), TwoByteValue() },
			{ TwoByteValue(0x0001), TwoByteValue(0xFFFF), TwoByteValue() },
			{ TwoByteValue(0xABCD), TwoByteValue(0x5433), TwoByteValue() },
			{ TwoByteValue(0xFF01), TwoByteValue(0x00FF), TwoByteValue() },
			{ TwoByteValue(0xFFFE), TwoByteValue(0xFFFE), TwoByteValue() }
		});
	}

	// endregion

	// region GetPaddingSize

	TEST(TEST_CLASS, GetPaddingSizeCalculatesPaddingSizeCorrectly) {
		EXPECT_EQ(0u, GetPaddingSize(40u, 8));
		EXPECT_EQ(7u, GetPaddingSize(49u, 8));
		EXPECT_EQ(4u, GetPaddingSize(52u, 8));

		EXPECT_EQ(2u, GetPaddingSize(40u, 7));
		EXPECT_EQ(0u, GetPaddingSize(49u, 7));
		EXPECT_EQ(4u, GetPaddingSize(52u, 7));

		EXPECT_EQ(0u, GetPaddingSize(40u, 1));
		EXPECT_EQ(0u, GetPaddingSize(49u, 1));
		EXPECT_EQ(0u, GetPaddingSize(52u, 1));
	}

	// endregion

	// region GetNumBits

	TEST(TEST_CLASS, GetNumBitsReturnsCorrectNumberOfBits) {
		EXPECT_EQ(8, GetNumBits<int8_t>());
		EXPECT_EQ(8u, GetNumBits<uint8_t>());

		EXPECT_EQ(32, GetNumBits<int32_t>());
		EXPECT_EQ(32u, GetNumBits<uint32_t>());
	}

	// endregion

	// region Log2

	TEST(TEST_CLASS, Log2ForNonzeroValuesReturnsHighBit) {
		EXPECT_EQ(0u, Log2<uint8_t>(0x01));
		EXPECT_EQ(6u, Log2<uint8_t>(0x73));
		EXPECT_EQ(7u, Log2<uint8_t>(0x80));
		EXPECT_EQ(7u, Log2<uint8_t>(0xFF));

		EXPECT_EQ(0u, Log2<uint32_t>(0x00000001));
		EXPECT_EQ(22u, Log2<uint32_t>(0x00500321));
		EXPECT_EQ(31u, Log2<uint32_t>(0x80000000));
		EXPECT_EQ(31u, Log2<uint32_t>(0xFFFFFFFF));
	}

	TEST(TEST_CLASS, Log2ForZeroValuesReturnsMax) {
		EXPECT_EQ(0xFFu, Log2<uint8_t>(0));
		EXPECT_EQ(0xFFFFFFFFu, Log2<uint32_t>(0));
	}

	// endregion

	// region Log2TimesPowerOfTwo

	TEST(TEST_CLASS, Log2TimesPowerOfTwoThrowsForZero) {
		EXPECT_THROW(Log2TimesPowerOfTwo(0, 0), catapult_invalid_argument);
		EXPECT_THROW(Log2TimesPowerOfTwo(0, 1), catapult_invalid_argument);
		EXPECT_THROW(Log2TimesPowerOfTwo(0, 10), catapult_invalid_argument);
		EXPECT_THROW(Log2TimesPowerOfTwo(0, 54), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, Log2TimesPowerOfTwoIsExactForPowersOfTwo) {
		// Arrange:
		constexpr auto Exponent = 10u;
		constexpr auto Two_To_Ten = 1ull << Exponent;

		for (auto i = 0u; i < 32; ++i) {
			// Act:
			auto result = Log2TimesPowerOfTwo(1ull << i, Exponent);

			// Assert:
			EXPECT_EQ(i * Two_To_Ten, result);
		}
	}

	TEST(TEST_CLASS, Log2TimesPowerOfTwoIsAGoodApproximationForExactValue) {
		// Arrange:
		constexpr auto Exponent = 54u;
		constexpr auto Two_To_Fifty_Four = 1ull << Exponent;

		uint32_t mask = 0;
		for (auto i = 0u; i < 32; ++i) {
			for (auto j = 0; j < 100; ++j) {
				auto value = (1ull << i) + (test::Random() & mask);

				// Act:
				auto result = Log2TimesPowerOfTwo(value, Exponent);
				auto expected = std::log2(value) * Two_To_Fifty_Four;
				auto ratio = 0.0 == expected && 0 == result ? 1.0 : static_cast<double>(result) / expected;

				// Assert:
				auto message = "for value " + std::to_string(value);
				EXPECT_LT(0.999999, ratio) << message;
				EXPECT_GT(1.000001, ratio) << message;
			}

			mask = (mask << 1) + 1;
		}
	}

	TEST(TEST_CLASS, Log2TimesPowerOfTwoCanIterateMoreThanThirtyOneTimesWithCorrectResult) {
		// Act:
		auto result = Log2TimesPowerOfTwo(0xF7F6F5F4, 54);

		// Assert:
		EXPECT_EQ(0x07FD0E2FCCDB25E2u, result);
	}

	// endregion

	// region FixedPointPowerOfTwo

	namespace {
		auto FixedPointToDouble(int32_t value) {
			return static_cast<double>(value) / 65'536;
		}
	}

	TEST(TEST_CLASS, FixedPointPowerOfTwoIsAGoodApproximationForExactValue) {
		// Arrange:
		for (auto value = -(6 << 16); value < 15 << 16; value += 0xFF) {
			// Act:
			auto result = FixedPointToDouble(static_cast<int32_t>(FixedPointPowerOfTwo(value)));
			auto exactResult = std::pow(2, FixedPointToDouble(value));
			auto relativeError = (exactResult - result) / exactResult;

			// Assert:
			EXPECT_LT(relativeError, 0.001)
					<< "for value (" << FixedPointToDouble(value)
					<< "), result (" << result
					<< "), exact result (" << exactResult << ")";
		}
	}

	// endregion

	// region DivideAndGetRemainder

	namespace {
		template<typename T>
		void AssertDivideAndGetRemainder(T seed, T divisor, T expectedResult, T expectedRemainder) {
			// Act:
			auto remainder = DivideAndGetRemainder(seed, divisor);

			// Assert:
			auto message = std::to_string(seed) + " / " + std::to_string(divisor);
			EXPECT_EQ(expectedResult, seed) << message;
			EXPECT_EQ(expectedRemainder, remainder) << message;
		}

		template<typename T>
		void AssertDivideAndGetRemainderForType() {
			// Assert:
			AssertDivideAndGetRemainder<T>(4096, 1000, 4, 96);
			AssertDivideAndGetRemainder<T>(17, 100, 0, 17);
			AssertDivideAndGetRemainder<T>(242, 121, 2, 0);
			AssertDivideAndGetRemainder<T>(0, 100, 0, 0);
		}
	}

	TEST(TEST_CLASS, DivideAndGetRemainderUpdatesValueAndReturnsRemainder) {
		AssertDivideAndGetRemainderForType<uint32_t>();
		AssertDivideAndGetRemainderForType<uint16_t>();
	}

	// endregion

	// region IsPowerMultiple

	TEST(TEST_CLASS, IsPowerMultipleReturnsFalseWhenMultipleIsLess) {
		EXPECT_FALSE(IsPowerMultiple<uint64_t>(200, 199, 10));
		EXPECT_FALSE(IsPowerMultiple<uint32_t>(200, 20, 10));
		EXPECT_FALSE(IsPowerMultiple<uint16_t>(200, 2, 10));
	}

	TEST(TEST_CLASS, IsPowerMultipleReturnsFalseWhenMultipleIsNotMultiple) {
		EXPECT_FALSE(IsPowerMultiple<uint64_t>(200, 201, 10));
		EXPECT_FALSE(IsPowerMultiple<uint32_t>(200, 399, 10));
		EXPECT_FALSE(IsPowerMultiple<uint16_t>(200, 650, 10));
	}

	TEST(TEST_CLASS, IsPowerMultipleReturnsFalseWhenMultipleIsNotPowerMultiple) {
		EXPECT_FALSE(IsPowerMultiple<uint64_t>(200, 200 * 2, 10));
		EXPECT_FALSE(IsPowerMultiple<uint32_t>(200, 200 * 20, 10));
		EXPECT_FALSE(IsPowerMultiple<uint16_t>(200, 200 * 7, 10));
	}

	TEST(TEST_CLASS, IsPowerMultipleReturnsTrueWhenMultipleIsEqual) {
		EXPECT_TRUE(IsPowerMultiple<uint64_t>(200, 200, 10));
		EXPECT_TRUE(IsPowerMultiple<uint32_t>(300, 300, 20));
		EXPECT_TRUE(IsPowerMultiple<uint16_t>(400, 400, 30));
	}

	TEST(TEST_CLASS, IsPowerMultipleReturnsTrueWhenMultipleIsPowerMultiple) {
		EXPECT_TRUE(IsPowerMultiple<uint64_t>(200, 200 * 100, 10));
		EXPECT_TRUE(IsPowerMultiple<uint32_t>(300, 300 * 20, 20));
		EXPECT_TRUE(IsPowerMultiple<uint16_t>(400, 400 * 8, 2));
	}

	// endregion
}}
