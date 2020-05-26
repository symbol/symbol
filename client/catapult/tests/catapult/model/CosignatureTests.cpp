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

#include "catapult/model/Cosignature.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS CosignatureTests

	TEST(TEST_CLASS, CosignatureHasExpectedSize) {
		EXPECT_EQ(104u, sizeof(Cosignature));
	}

	TEST(TEST_CLASS, CanCreateEmptyCosignature) {
		// Act:
		auto cosignature = Cosignature();

		// Assert:
		EXPECT_EQ(0u, cosignature.Version);
		EXPECT_EQ(Key(), cosignature.SignerPublicKey);
		EXPECT_EQ(Signature(), cosignature.Signature);
	}

	TEST(TEST_CLASS, CanCreateCosignatureFromParameters) {
		// Arrange:
		auto signerPublicKey = test::GenerateRandomByteArray<Key>();
		auto signature = test::GenerateRandomByteArray<Signature>();

		// Act:
		auto cosignature = Cosignature(signerPublicKey, signature);

		// Assert:
		EXPECT_EQ(0u, cosignature.Version);
		EXPECT_EQ(signerPublicKey, cosignature.SignerPublicKey);
		EXPECT_EQ(signature, cosignature.Signature);
	}

	TEST(TEST_CLASS, DetachedCosignatureHasExpectedSize) {
		EXPECT_EQ(sizeof(Cosignature) + 32u, sizeof(DetachedCosignature));
	}

	TEST(TEST_CLASS, CanCreateDetachedCosignatureFromParameters) {
		// Arrange:
		auto signerPublicKey = test::GenerateRandomByteArray<Key>();
		auto signature = test::GenerateRandomByteArray<Signature>();
		auto parentHash = test::GenerateRandomByteArray<Hash256>();

		// Act:
		auto cosignature = DetachedCosignature(signerPublicKey, signature, parentHash);

		// Assert:
		EXPECT_EQ(0u, cosignature.Version);
		EXPECT_EQ(signerPublicKey, cosignature.SignerPublicKey);
		EXPECT_EQ(signature, cosignature.Signature);
		EXPECT_EQ(parentHash, cosignature.ParentHash);
	}

	TEST(TEST_CLASS, CanCreateDetachedCosignatureFromCosignature) {
		// Arrange:
		auto cosignatureSeed = test::CreateRandomDetachedCosignature();
		auto parentHash = test::GenerateRandomByteArray<Hash256>();

		// Act:
		auto cosignature = DetachedCosignature(cosignatureSeed, parentHash);

		// Assert:
		EXPECT_EQ(cosignatureSeed.Version, cosignature.Version);
		EXPECT_EQ(cosignatureSeed.SignerPublicKey, cosignature.SignerPublicKey);
		EXPECT_EQ(cosignatureSeed.Signature, cosignature.Signature);
		EXPECT_EQ(parentHash, cosignature.ParentHash);
	}
}}
