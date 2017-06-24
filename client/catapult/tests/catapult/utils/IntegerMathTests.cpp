#include "catapult/utils/IntegerMath.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

	TEST(IntegerMathTests, GetNumBitsReturnsCorrectNumberOfBits) {
		// Assert:
		EXPECT_EQ(8u, GetNumBits<int8_t>());
		EXPECT_EQ(8u, GetNumBits<uint8_t>());

		EXPECT_EQ(32u, GetNumBits<int32_t>());
		EXPECT_EQ(32u, GetNumBits<uint32_t>());
	}

	TEST(IntegerMathTests, Log2ForNonZeroValuesReturnsHighBit) {
		// Assert:
		EXPECT_EQ(0u, Log2<uint8_t>(0x01));
		EXPECT_EQ(6u, Log2<uint8_t>(0x73));
		EXPECT_EQ(7u, Log2<uint8_t>(0x80));
		EXPECT_EQ(7u, Log2<uint8_t>(0xFF));

		EXPECT_EQ(0u, Log2<uint32_t>(0x00000001));
		EXPECT_EQ(22u, Log2<uint32_t>(0x00500321));
		EXPECT_EQ(31u, Log2<uint32_t>(0x80000000));
		EXPECT_EQ(31u, Log2<uint32_t>(0xFFFFFFFF));
	}

	TEST(IntegerMathTests, Log2ForZeroValuesReturnsMax) {
		// Assert:
		EXPECT_EQ(0xFFu, Log2<uint8_t>(0));
		EXPECT_EQ(0xFFFFFFFFu, Log2<uint32_t>(0));
	}

	TEST(IntegerMathTests, Pow2ReturnsCorrectValueForInRangeResults) {
		// Assert:
		EXPECT_EQ(0x01u, Pow2<uint8_t>(0));
		EXPECT_EQ(0x04u, Pow2<uint8_t>(2));
		EXPECT_EQ(0x80u, Pow2<uint8_t>(7));

		EXPECT_EQ(0x00000001u, Pow2<uint32_t>(0));
		EXPECT_EQ(0x00020000u, Pow2<uint32_t>(17));
		EXPECT_EQ(0x80000000u, Pow2<uint32_t>(31));
	}

	TEST(IntegerMathTests, Pow2ReturnsZeroForOutOfRangeResults) {
		// Assert:
		EXPECT_EQ(0u, Pow2<uint8_t>(8));
		EXPECT_EQ(0u, Pow2<uint8_t>(18));

		EXPECT_EQ(0u, Pow2<uint32_t>(32));
		EXPECT_EQ(0u, Pow2<uint32_t>(70));
	}

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

	TEST(IntegerMathTests, DivideAndGetRemainderUpdatesValueAndReturnsRemainder) {
		// Assert:
		AssertDivideAndGetRemainderForType<uint32_t>();
		AssertDivideAndGetRemainderForType<uint16_t>();
	}
}}
