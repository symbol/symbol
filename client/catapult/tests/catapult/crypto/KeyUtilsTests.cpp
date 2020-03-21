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

#include "catapult/crypto/KeyUtils.h"
#include "catapult/crypto/PrivateKey.h"
#include "tests/TestHarness.h"
#include <array>
#include <unordered_map>

namespace catapult { namespace crypto {

#define TEST_CLASS KeyUtilsTests

	namespace {
		template<typename T>
		std::string FormatKeyAsString(const T& key) {
			std::ostringstream out;
			out << FormatKey(key);
			return out.str();
		}
	}

	TEST(TEST_CLASS, CanOutputPrivateKey) {
		// Arrange:
		auto key = PrivateKey::FromString("031729D10DB52ECF0AD3684558DB31895DDFA5CD7F4143AF6E822E114E16E31C");

		// Act:
		std::string actual = FormatKeyAsString(key);

		// Assert:
		EXPECT_EQ("031729D10DB52ECF0AD3684558DB31895DDFA5CD7F4143AF6E822E114E16E31C", actual);
	}

	TEST(TEST_CLASS, IsValidKeyStringReturnsTrueForValidKeyString) {
		// Act:
		auto isValid = IsValidKeyString("031729D10DB52ECF0AD3684558DB31895DDFA5CD7F4143AF6E822E114E16E31C");

		// Assert:
		EXPECT_TRUE(isValid);
	}

	TEST(TEST_CLASS, IsValidKeyStringReturnsFalseForInvalidKeyString) {
		// Arrange:
		std::unordered_map<std::string, std::string> keyStrings{
			{ "T31729D10DB52ECF0AD3684558DB31895DDFA5CD7F4143AF6E822E114E16E31C", "invalid char T at the beginning" },
			{ "031729D10DB52ECF0AD3684558DB31895TDFA5CD7F4143AF6E822E114E16E31T", "invalid char T in the middle part" },
			{ "031729D10DB52ECF0AD3684558DB31895DDFA5CD7F4143AF6E822E114E16E31T", "invalid char T at the end" },
			{ "U31729D10DB52ECF0XD3684558DB3189YDDFA5CD7F4143AF6Z822E114E16E31T", "multiple invalid chars" },
			{ "031729D10DB52ECF0AD3684558DB31895DDFA5CD7F4143AF6E822E114E16E3", "string too short" },
			{ "031729D10DB52ECF0AD3684558DB31895DDFA5CD7F4143AF6E822E114E16E31CFF", "string too long" }
		};

		for (const auto& pair : keyStrings) {
			// Act:
			auto isValid = IsValidKeyString(pair.first);

			// Assert:
			EXPECT_FALSE(isValid) << pair.second;
		}
	}
}}
