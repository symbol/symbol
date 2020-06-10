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
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS NetworkInfoTests

	TEST(TEST_CLASS, CanCreateDefaultNetwork) {
		// Act:
		NetworkInfo networkInfo;

		// Assert:
		EXPECT_EQ(NetworkIdentifier::Zero, networkInfo.Identifier);
		EXPECT_EQ(NodeIdentityEqualityStrategy::Key, networkInfo.NodeEqualityStrategy);
		EXPECT_EQ(Key(), networkInfo.NemesisSignerPublicKey);
		EXPECT_EQ(GenerationHashSeed(), networkInfo.GenerationHashSeed);
		EXPECT_EQ(utils::TimeSpan(), networkInfo.EpochAdjustment);
	}

	TEST(TEST_CLASS, CanCreateCustomNetwork) {
		// Arrange:
		auto nemesisSignerPublicKey = test::GenerateRandomByteArray<Key>();
		auto generationHashSeed = test::GenerateRandomByteArray<GenerationHashSeed>();

		// Act:
		NetworkInfo networkInfo(
				static_cast<NetworkIdentifier>(0xB9),
				static_cast<NodeIdentityEqualityStrategy>(0xA7),
				nemesisSignerPublicKey,
				generationHashSeed,
				utils::TimeSpan::FromHours(123));

		// Assert:
		EXPECT_EQ(static_cast<NetworkIdentifier>(0xB9), networkInfo.Identifier);
		EXPECT_EQ(static_cast<NodeIdentityEqualityStrategy>(0xA7), networkInfo.NodeEqualityStrategy);
		EXPECT_EQ(nemesisSignerPublicKey, networkInfo.NemesisSignerPublicKey);
		EXPECT_EQ(generationHashSeed, networkInfo.GenerationHashSeed);
		EXPECT_EQ(utils::TimeSpan::FromHours(123), networkInfo.EpochAdjustment);
	}

	TEST(TEST_CLASS, CanGetUniqueNetworkFingerprintForNetwork) {
		// Arrange:
		NetworkInfo networkInfo(
				static_cast<NetworkIdentifier>(0xB9),
				static_cast<NodeIdentityEqualityStrategy>(0xA7),
				test::GenerateRandomByteArray<Key>(),
				test::GenerateRandomByteArray<GenerationHashSeed>(),
				utils::TimeSpan::FromHours(123));

		// Act:
		auto fingerprint = GetUniqueNetworkFingerprint(networkInfo);

		// Assert:
		EXPECT_EQ(static_cast<NetworkIdentifier>(0xB9), fingerprint.Identifier);
		EXPECT_EQ(networkInfo.GenerationHashSeed, fingerprint.GenerationHashSeed);
	}
}}
