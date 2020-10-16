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

#include "catapult/ionet/NodeVersion.h"
#include "catapult/version/version.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace ionet {

#define TEST_CLASS NodeVersionTests

	TEST(TEST_CLASS, GetCurrentServerVersionReturnsCorrectVersion) {
		// Act:
		auto version = GetCurrentServerVersion();

		// Assert:
		EXPECT_EQ(static_cast<uint32_t>(CATAPULT_VERSION_MAJOR), (version.unwrap() >> 24) & 0xFF);
		EXPECT_EQ(static_cast<uint32_t>(CATAPULT_VERSION_MINOR), (version.unwrap() >> 16) & 0xFF);
		EXPECT_EQ(static_cast<uint32_t>(CATAPULT_VERSION_REVISION), (version.unwrap() >> 8) & 0xFF);
		EXPECT_EQ(static_cast<uint32_t>(CATAPULT_VERSION_BUILD), (version.unwrap() & 0xFF));
	}

	TEST(TEST_CLASS, CanParseEmptyNodeVersion) {
		test::AssertParse("", GetCurrentServerVersion(), TryParseValue);
	}

	TEST(TEST_CLASS, CanParseValidNodeVersion) {
		test::AssertParse("0.0.0.0", NodeVersion(), TryParseValue); // min
		test::AssertParse("1.1.1.1", NodeVersion(0x01010101), TryParseValue); // one
		test::AssertParse("255.255.255.255", NodeVersion(0xFFFFFFFF), TryParseValue); // max

		test::AssertParse("9.34.12.222", NodeVersion(0x09220CDE), TryParseValue);
	}

	TEST(TEST_CLASS, CannotParseInvalidNodeVersion) {
		// Arrange:
		std::vector<std::string> invalidStrings{
			// wrong number of components
			".",
			"1.1.1",
			"1.1.1.1.1",

			// invalid parts
			"AZ.34.12.222", // 0 => not a number
			"9.BC.12.222", //  1 => hex
			"9.34.256.222", // 2 => too big
			"9.34.12.-222", // 3 => negative

			// empty parts
			"...",
			"9.34..12.222",
			"9.34..222",

			// external whitespace
			"  9.34.12.222",
			"9.34.12.222  ",
			"  9.34.12.222  ",

			// internal whitespace (using \x20 to get around lint)
			"9.34. 12.222",
			"9.34.12\x20.222",
			"9.34. 12\x20.222"
		};

		// Act + Assert:
		for (const auto& str : invalidStrings)
			test::AssertFailedParse(str, NodeVersion(123), TryParseValue);
	}
}}
