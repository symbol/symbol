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

#include "catapult/utils/Casting.h"
#include "tests/TestHarness.h"
#include <functional>
#include <sstream>

namespace catapult { namespace utils {

#define TEST_CLASS CastingTests

	// region as_const

	TEST(TEST_CLASS, AsConstReturnsConstReferenceForConstType) {
		// Arrange:
		const int val = 7;

		// Act:
		auto& ref = as_const(val);

		// Assert:
		EXPECT_EQ(7, ref);
		EXPECT_TRUE(std::is_const_v<decltype(val)>);
		EXPECT_TRUE(std::is_const_v<std::remove_reference_t<decltype(ref)>>);
	}

	TEST(TEST_CLASS, AsConstReturnsConstReferenceForNonConstType) {
		// Arrange:
		int val = 7;

		// Act:
		auto& ref = as_const(val);

		// Assert:
		EXPECT_EQ(7, ref);
		EXPECT_FALSE(std::is_const_v<decltype(val)>);
		EXPECT_TRUE(std::is_const_v<std::remove_reference_t<decltype(ref)>>);
	}

	// endregion

	// region to_ratio

	TEST(TEST_CLASS, ToRatioReturnsExpectedValue) {
		EXPECT_EQ(2.0 / 3.0, to_ratio(2, 3));
		EXPECT_EQ(3.0 / 2.0, to_ratio(3, 2));
	}

	// endregion

	// region to_underlying_type

	namespace {
		std::string CreateUnderlyingTypeMessage(const char* typeName) {
			std::stringstream message;
			message << "(underlying type = " << typeName << ")";
			return message.str();
		}

		template<typename TUnderlyingType>
		void AssertToUnderlyingTypePreservesEnumValues(const char* typeName) {
			// Arrange:
			enum class TestEnum : TUnderlyingType { Zero = 0, Two = 2, Four = 4 };
			auto message = CreateUnderlyingTypeMessage(typeName);

			// Act: convert all values in TestEnum
			TUnderlyingType expected = 0;
			for (auto value : { TestEnum::Zero, TestEnum::Two, TestEnum::Four }) {
				auto rawValue = to_underlying_type(value);

				// Assert: the values are the same
				EXPECT_EQ(expected, rawValue) << "has same value " << message;
				expected = static_cast<TUnderlyingType>(expected + 2);
			}
		}

		template<typename TUnderlyingType>
		void AssertToUnderlyingTypePreservesEnumType(const char* typeName) {
			// Arrange:
			enum class TestEnum : TUnderlyingType { Foo };
			auto message = CreateUnderlyingTypeMessage(typeName);

			// Act:
			using ActualUnderlyingType = decltype(to_underlying_type(TestEnum::Foo));

			// Assert: the underlying types are the same
			EXPECT_EQ(sizeof(TUnderlyingType), sizeof(ActualUnderlyingType)) << "has same size " << message;
			auto areTypesSame = std::is_same_v<TUnderlyingType, ActualUnderlyingType>;
			EXPECT_TRUE(areTypesSame) << "has same type " << message;
		}
	}

	TEST(TEST_CLASS, ToUnderlyingTypePreservesEnumValues) {
		AssertToUnderlyingTypePreservesEnumValues<int8_t>("int8_t");
		AssertToUnderlyingTypePreservesEnumValues<uint64_t>("uint64_t");
	}

	TEST(TEST_CLASS, ToUnderlyingTypePreservesEnumType) {
		AssertToUnderlyingTypePreservesEnumType<int8_t>("int8_t");
		AssertToUnderlyingTypePreservesEnumType<uint64_t>("uint64_t");
	}

	// endregion

	// region checked_cast

	namespace {
		template<typename TSource, typename TDest>
		void AssertCheckedCast(TSource value, TDest expectedValue) {
			// Arrange:
			std::stringstream message;
			message << "casting " << static_cast<int64_t>(value);

			// Act:
			auto convertedValue = checked_cast<TSource, TDest>(value);

			// Assert:
			auto areTypesSame = std::is_same_v<TDest, decltype(convertedValue)>;
			EXPECT_TRUE(areTypesSame) << message.str();
			EXPECT_EQ(expectedValue, convertedValue) << message.str();
		}

		template<typename TSource, typename TDest>
		void AssertCheckedCastFails(TSource value) {
			// Arrange:
			std::stringstream message;
			message << "casting " << static_cast<int64_t>(value);
			action cast = [value]() { checked_cast<TSource, TDest>(value); };

			// Act + Assert:
			EXPECT_THROW(cast(), catapult_runtime_error) << message.str();
		}

		const auto UInt8_Min = std::numeric_limits<uint8_t>::min();
		const auto UInt8_Max = std::numeric_limits<uint8_t>::max();
		const auto Int8_Min = std::numeric_limits<int8_t>::min();
		const auto Int8_Max = std::numeric_limits<int8_t>::max();

		const auto UInt16_Max = std::numeric_limits<uint16_t>::max();
		const auto Int16_Max = std::numeric_limits<int16_t>::max();
		const auto Int16_Min = std::numeric_limits<int16_t>::min();
	}

	TEST(TEST_CLASS, CheckedCastChecksUnsignedToSignedConversions) {
		// Assert: can convert within bounds [UInt8_Min, Int8_Max]
		AssertCheckedCast<uint8_t, int8_t>(UInt8_Min, static_cast<int8_t>(UInt8_Min)); // min
		AssertCheckedCast<uint8_t, int8_t>(static_cast<uint8_t>(Int8_Max / 2), Int8_Max / 2); // min < x < max
		AssertCheckedCast<uint8_t, int8_t>(static_cast<uint8_t>(Int8_Max), Int8_Max); // max

		// Assert: cannot convert outside of bounds
		AssertCheckedCastFails<uint8_t, int8_t>(static_cast<uint8_t>(Int8_Min));
		AssertCheckedCastFails<uint8_t, int8_t>(Int8_Max / 2 * 3);
		AssertCheckedCastFails<uint8_t, int8_t>(UInt8_Max);
	}

	TEST(TEST_CLASS, CheckedCastChecksSignedToUnsignedConversions) {
		// Assert: can convert within bounds [UInt8_Min, Int8_Max]
		//         (static casts are needed for signed / unsigned adjustments)
		AssertCheckedCast<int8_t, uint8_t>(static_cast<int8_t>(UInt8_Min), UInt8_Min); // min
		AssertCheckedCast<int8_t, uint8_t>(Int8_Max / 2, static_cast<uint8_t>(Int8_Max / 2)); // min < x < max
		AssertCheckedCast<int8_t, uint8_t>(Int8_Max, static_cast<uint8_t>(Int8_Max)); // max

		// Assert: cannot convert outside of bounds
		AssertCheckedCastFails<int8_t, uint8_t>(Int8_Min);
		AssertCheckedCastFails<int8_t, uint8_t>(static_cast<int8_t>(Int8_Max / 2 * 3));
		AssertCheckedCastFails<int8_t, uint8_t>(static_cast<int8_t>(UInt8_Max));
	}

	TEST(TEST_CLASS, CheckedCastChecksUnsignedToUnsignedConversions) {
		// Assert: can convert within bounds [UInt8_Min, UInt8_Max]
		AssertCheckedCast<uint16_t, uint8_t>(UInt8_Min, UInt8_Min); // min
		AssertCheckedCast<uint16_t, uint8_t>(UInt8_Max / 2, UInt8_Max / 2); // min < x < max
		AssertCheckedCast<uint16_t, uint8_t>(UInt8_Max, UInt8_Max); // max

		// Assert: cannot convert outside of bounds
		AssertCheckedCastFails<uint16_t, uint8_t>(UInt8_Max + 1);
		AssertCheckedCastFails<uint16_t, uint8_t>(UInt16_Max / 2);
		AssertCheckedCastFails<uint16_t, uint8_t>(UInt16_Max);
	}

	TEST(TEST_CLASS, CheckedCastChecksSignedToSignedConversions) {
		// Assert: can convert within bounds [Int8_Min, Int8_Max]
		AssertCheckedCast<int16_t, int8_t>(Int8_Min, Int8_Min); // min
		AssertCheckedCast<int16_t, int8_t>(Int8_Min / 2, Int8_Min / 2); // min < y < 0
		AssertCheckedCast<int16_t, int8_t>(0, 0); // y < 0 < x
		AssertCheckedCast<int16_t, int8_t>(Int8_Max / 2, Int8_Max / 2); // 0 < x < max
		AssertCheckedCast<int16_t, int8_t>(Int8_Max, Int8_Max); // max

		// Assert: cannot convert outside of bounds
		AssertCheckedCastFails<int16_t, int8_t>(Int16_Min);
		AssertCheckedCastFails<int16_t, int8_t>(Int16_Min / 2);
		AssertCheckedCastFails<int16_t, int8_t>(Int8_Min - 1);
		AssertCheckedCastFails<int16_t, int8_t>(Int8_Max + 1);
		AssertCheckedCastFails<int16_t, int8_t>(Int16_Max / 2);
		AssertCheckedCastFails<int16_t, int8_t>(Int16_Max);
	}

	// endregion
}}
