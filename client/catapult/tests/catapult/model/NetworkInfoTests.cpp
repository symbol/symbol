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

#include "catapult/model/NetworkInfo.h"
#include "catapult/utils/Casting.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS NetworkInfoTests

	// region construction

	TEST(TEST_CLASS, CanCreateDefaultNetwork) {
		// Act:
		NetworkInfo info;

		// Assert:
		EXPECT_EQ(0x00u, utils::to_underlying_type(info.Identifier));
		EXPECT_EQ(Key(), info.PublicKey);
		EXPECT_EQ(GenerationHash(), info.GenerationHash);
	}

	TEST(TEST_CLASS, CanCreateCustomNetwork) {
		// Arrange:
		auto publicKey = test::GenerateRandomByteArray<Key>();
		auto generationHash = test::GenerateRandomByteArray<GenerationHash>();

		// Act:
		NetworkInfo info(static_cast<NetworkIdentifier>(0xB9), publicKey, generationHash);

		// Assert:
		EXPECT_EQ(0xB9u, utils::to_underlying_type(info.Identifier));
		EXPECT_EQ(publicKey, info.PublicKey);
		EXPECT_EQ(generationHash, info.GenerationHash);
	}

	// endregion

	// region parsing

	TEST(TEST_CLASS, CanParseValidNetworkValue) {
		// Arrange:
		auto assertSuccessfulParse = [](const auto& input, const auto& expectedParsedValue) {
			test::AssertParse(input, expectedParsedValue, [](const auto& str, auto& parsedValue) {
				return TryParseValue(str, parsedValue);
			});
		};

		// Assert:
		assertSuccessfulParse("mijin", NetworkIdentifier::Mijin);
		assertSuccessfulParse("mijin-test", NetworkIdentifier::Mijin_Test);
		assertSuccessfulParse("public", NetworkIdentifier::Public);
		assertSuccessfulParse("public-test", NetworkIdentifier::Public_Test);

		assertSuccessfulParse("0", static_cast<NetworkIdentifier>(0));
		assertSuccessfulParse("17", static_cast<NetworkIdentifier>(17));
		assertSuccessfulParse("255", static_cast<NetworkIdentifier>(255));
	}

	TEST(TEST_CLASS, CannotParseInvalidNetworkValue) {
		test::AssertEnumParseFailure("mijin", NetworkIdentifier::Public, [](const auto& str, auto& parsedValue) {
			return TryParseValue(str, parsedValue);
		});
		test::AssertFailedParse("256", NetworkIdentifier::Public, [](const auto& str, auto& parsedValue) {
			return TryParseValue(str, parsedValue);
		});
	}

	// endregion
}}
