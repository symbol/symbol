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

#include "src/state/SecretLockInfoHistorySerializer.h"
#include "plugins/txes/lock_shared/tests/state/LockInfoHistorySerializerTests.h"
#include "tests/test/SecretLockInfoCacheTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS SecretLockInfoHistorySerializerTests

	namespace {
		// region PackedSecretLockInfo

#pragma pack(push, 1)

		struct PackedSecretLockInfo : public PackedLockInfo {
		public:
			explicit PackedSecretLockInfo(const SecretLockInfo& secretLockInfo)
					: PackedLockInfo(secretLockInfo)
					, HashAlgorithm(secretLockInfo.HashAlgorithm)
					, Secret(secretLockInfo.Secret)
					, RecipientAddress(secretLockInfo.RecipientAddress)
			{}

		public:
			model::LockHashAlgorithm HashAlgorithm;
			Hash256 Secret;
			Address RecipientAddress;
		};

#pragma pack(pop)

		// endregion

		struct SecretLockInfoTraits : public test::BasicSecretLockInfoTestTraits {
		public:
			using HistoryType = SecretLockInfoHistory;
			using PackedValueType = PackedSecretLockInfo;

			using SerializerType = SecretLockInfoHistorySerializer;
			using NonHistoricalSerializerType = SecretLockInfoHistoryNonHistoricalSerializer;

		public:
			static Hash256 SetEqualIdentifier(std::vector<SecretLockInfo>& lockInfos) {
				auto secret = test::GenerateRandomByteArray<Hash256>();
				auto recipientAddress = test::GenerateRandomByteArray<Address>();
				auto compositeHash = model::CalculateSecretLockInfoHash(secret, recipientAddress);
				for (auto& lockInfo : lockInfos) {
					lockInfo.Secret = secret;
					lockInfo.RecipientAddress = recipientAddress;
					lockInfo.CompositeHash = compositeHash;
				}

				return compositeHash;
			}
		};
	}

	DEFINE_LOCK_INFO_HISTORY_SERIALIZER_TESTS(SecretLockInfoTraits)

	TEST(TEST_CLASS, LoadCalculatesCompositeHash) {
		// Arrange:
		auto originalLockInfo = test::BasicSecretLockInfoTestTraits::CreateLockInfo();
		test::FillWithRandomData(originalLockInfo.CompositeHash);
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream outputStream(buffer);

		// Act:
		SecretLockInfoSerializer::Save(originalLockInfo, outputStream);
		mocks::MockMemoryStream inputStream(buffer);
		auto lockInfo = SecretLockInfoSerializer::Load(inputStream);

		// Assert: the random composite hash was not saved but recalculated during load
		auto expectedCompositeHash = model::CalculateSecretLockInfoHash(originalLockInfo.Secret, originalLockInfo.RecipientAddress);
		EXPECT_EQ(expectedCompositeHash, lockInfo.CompositeHash);
	}
}}
