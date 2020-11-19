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

#include "src/state/AccountRestrictionUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS AccountRestrictionUtilsTests

	TEST(TEST_CLASS, CanConvertPrimitiveTypeToVector) {
		// Act:
		auto vector = ToVector(static_cast<uint16_t>(0x1234));

		// Assert:
		auto expectedVector = std::vector<uint8_t>{ 0x34, 0x12 };
		EXPECT_EQ(expectedVector, vector);
	}

	TEST(TEST_CLASS, CanConvertBaseTypeToVector) {
		// Act:
		auto vector = ToVector(Amount(0x9A78563412));

		// Assert:
		auto expectedVector = std::vector<uint8_t>{ 0x12, 0x34, 0x56, 0x78, 0x9A, 0x00, 0x00, 0x00 };
		EXPECT_EQ(expectedVector, vector);
	}

	TEST(TEST_CLASS, CanConvertEmptyArrayToVector) {
		// Act:
		auto vector = ToVector(std::array<uint8_t, 0>{});

		// Assert:
		auto expectedVector = std::vector<uint8_t>{};
		EXPECT_EQ(expectedVector, vector);
	}

	TEST(TEST_CLASS, CanConvertArrayWithValuesToVector) {
		// Act:
		auto vector = ToVector(std::array<uint8_t, 5>{ { 0x12, 0x34, 0x56, 0x78, 0x9A } });

		// Assert:
		auto expectedVector = std::vector<uint8_t>{ 0x12, 0x34, 0x56, 0x78, 0x9A };
		EXPECT_EQ(expectedVector, vector);
	}
}}
