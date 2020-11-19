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

#include "catapult/crypto_voting/BmKeyIdentifier.h"
#include "tests/test/nodeps/Alignment.h"
#include "tests/test/nodeps/Comparison.h"
#include "tests/TestHarness.h"

namespace catapult { namespace crypto {

#define TEST_CLASS BmKeyIdentifierTests

	// region size + alignment

#define KEY_IDENTIFIER_FIELDS FIELD(KeyId)

	TEST(TEST_CLASS, KeyIdentifierHasExpectedSize) {
		// Arrange:
		auto expectedSize = 0u;

#define FIELD(X) expectedSize += SizeOf32<decltype(BmKeyIdentifier::X)>();
		KEY_IDENTIFIER_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(BmKeyIdentifier));
		EXPECT_EQ(8u, sizeof(BmKeyIdentifier));
	}

	TEST(TEST_CLASS, KeyIdentifierHasProperAlignment) {
#define FIELD(X) EXPECT_ALIGNED(BmKeyIdentifier, X);
		KEY_IDENTIFIER_FIELDS
#undef FIELD

		EXPECT_EQ(0u, sizeof(BmKeyIdentifier) % 8);
	}

#undef KEY_IDENTIFIER_FIELDS

	// endregion

	// region operators

	namespace {
		std::vector<BmKeyIdentifier> GenerateIncreasingKeyIdentifierValues() {
			return {
					{ BmKeyIdentifier::Invalid_Id },
					{ 0 },
					{ 1 },
					{ 5 },
					{ 10 }
			};
		}
	}

	DEFINE_EQUALITY_AND_COMPARISON_TESTS(TEST_CLASS, GenerateIncreasingKeyIdentifierValues())

	TEST(TEST_CLASS, KeyIdentifier_CanOutput) {
		// Arrange:
		BmKeyIdentifier keyIdentifier{ 11 };

		// Act:
		auto str = test::ToString(keyIdentifier);

		// Assert:
		EXPECT_EQ("(11)", str);
	}

	// endregion
}}
