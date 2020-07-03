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

#include "catapult/model/HeightHashPair.h"
#include "catapult/utils/HexParser.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS HeightHashPairTests

	// region equality operators

	namespace {
		std::unordered_set<std::string> GetEqualTags() {
			return { "default", "copy" };
		}

		std::unordered_map<std::string, HeightHashPair> GenerateEqualityInstanceMap() {
			auto hash1 = test::GenerateRandomByteArray<Hash256>();
			auto hash2 = test::GenerateRandomByteArray<Hash256>();

			return {
				{ "default", { Height(123), hash1 } },
				{ "copy", { Height(123), hash1 } },

				{ "diff-height", { Height(345), hash1 } },
				{ "diff-hash", { Height(123), hash2 } },
				{ "diff-height-hash", { Height(345), hash2 } }
			};
		}
	}

	TEST(TEST_CLASS, OperatorEqualReturnsTrueOnlyForEqualValues) {
		test::AssertOperatorEqualReturnsTrueForEqualObjects("default", GenerateEqualityInstanceMap(), GetEqualTags());
	}

	TEST(TEST_CLASS, OperatorNotEqualReturnsTrueOnlyForUnequalValues) {
		test::AssertOperatorNotEqualReturnsTrueForUnequalObjects("default", GenerateEqualityInstanceMap(), GetEqualTags());
	}

	// endregion

	// region to string

	TEST(TEST_CLASS, CanOutputHeightHashPair) {
		// Arrange:
		auto heightHashPair = HeightHashPair{
			Height(123),
			utils::ParseByteArray<Hash256>("7FC169546CB19DD9C71918D66BD4A251CEF0956B1C9CC022E4127400E8FA7AFF")
		};

		// Act:
		auto str = test::ToString(heightHashPair);

		// Assert:
		EXPECT_EQ("7FC169546CB19DD9C71918D66BD4A251CEF0956B1C9CC022E4127400E8FA7AFF @ 123", str);
	}

	// endregion
}}
