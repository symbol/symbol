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

#include "src/builders/SecretLockBuilder.h"
#include "tests/builders/test/BuilderTestUtils.h"

namespace catapult { namespace builders {

#define TEST_CLASS SecretLockBuilderTests

	namespace {
		using RegularTraits = test::RegularTransactionTraits<model::SecretLockTransaction>;
		using EmbeddedTraits = test::EmbeddedTransactionTraits<model::EmbeddedSecretLockTransaction>;

		struct TransactionProperties {
		public:
			TransactionProperties()
					: HashAlgorithm(model::LockHashAlgorithm::Op_Sha3_256)
					, Secret()
					, Recipient()
			{}

		public:
			model::UnresolvedMosaic Mosaic;
			BlockDuration Duration;
			model::LockHashAlgorithm HashAlgorithm;
			Hash256 Secret;
			UnresolvedAddress Recipient;
		};

		template<typename TTransaction>
		void AssertTransactionProperties(const TransactionProperties& expectedProperties, const TTransaction& transaction) {
			EXPECT_EQ(expectedProperties.Mosaic.MosaicId, transaction.Mosaic.MosaicId);
			EXPECT_EQ(expectedProperties.Mosaic.Amount, transaction.Mosaic.Amount);
			EXPECT_EQ(expectedProperties.Duration, transaction.Duration);
			EXPECT_EQ(expectedProperties.HashAlgorithm, transaction.HashAlgorithm);
			EXPECT_EQ(expectedProperties.Secret, transaction.Secret);
			EXPECT_EQ(expectedProperties.Recipient, transaction.RecipientAddress);
		}

		template<typename TTraits>
		void AssertCanBuildTransaction(
				const TransactionProperties& expectedProperties,
				const consumer<SecretLockBuilder&>& buildTransaction) {
			// Arrange:
			auto networkIdentifier = static_cast<model::NetworkIdentifier>(0x62);
			auto signer = test::GenerateRandomByteArray<Key>();

			// Act:
			SecretLockBuilder builder(networkIdentifier, signer);
			buildTransaction(builder);
			auto pTransaction = TTraits::InvokeBuilder(builder);

			// Assert:
			TTraits::CheckBuilderSize(0, builder);
			TTraits::CheckFields(0, *pTransaction);
			EXPECT_EQ(signer, pTransaction->SignerPublicKey);
			EXPECT_EQ(1u, pTransaction->Version);
			EXPECT_EQ(static_cast<model::NetworkIdentifier>(0x62), pTransaction->Network);
			EXPECT_EQ(model::Entity_Type_Secret_Lock, pTransaction->Type);

			AssertTransactionProperties(expectedProperties, *pTransaction);
		}
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Regular) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<RegularTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Embedded) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<EmbeddedTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region constructor

	TRAITS_BASED_TEST(CanCreateTransactionWithDefaultValues) {
		AssertCanBuildTransaction<TTraits>(TransactionProperties(), [](const auto&) {});
	}

	// endregion

	// region additional transaction fields

	TRAITS_BASED_TEST(CanSetMosaic) {
		// Arrange:
		auto expectedProperties = TransactionProperties();
		expectedProperties.Mosaic = { UnresolvedMosaicId(123), Amount(234) };

		// Assert:
		AssertCanBuildTransaction<TTraits>(expectedProperties, [](auto& builder) {
			builder.setMosaic({ UnresolvedMosaicId(123), Amount(234) });
		});
	}

	TRAITS_BASED_TEST(CanSetBlockDuration) {
		// Arrange:
		auto expectedProperties = TransactionProperties();
		expectedProperties.Duration = BlockDuration(123);

		// Assert:
		AssertCanBuildTransaction<TTraits>(expectedProperties, [](auto& builder) {
			builder.setDuration(BlockDuration(123));
		});
	}

	TRAITS_BASED_TEST(CanSetHashAlgorithm) {
		// Arrange:
		auto expectedProperties = TransactionProperties();
		expectedProperties.HashAlgorithm = model::LockHashAlgorithm::Op_Hash_160;

		// Assert:
		AssertCanBuildTransaction<TTraits>(expectedProperties, [](auto& builder) {
			builder.setHashAlgorithm(model::LockHashAlgorithm::Op_Hash_160);
		});
	}

	TRAITS_BASED_TEST(CanSetSecret) {
		// Arrange:
		auto expectedProperties = TransactionProperties();
		test::FillWithRandomData(expectedProperties.Secret);

		// Assert:
		AssertCanBuildTransaction<TTraits>(expectedProperties, [&secret = expectedProperties.Secret](auto& builder) {
			builder.setSecret(secret);
		});
	}

	TRAITS_BASED_TEST(CanSetRecipient) {
		// Arrange:
		auto expectedProperties = TransactionProperties();
		test::FillWithRandomData(expectedProperties.Recipient);

		// Assert:
		AssertCanBuildTransaction<TTraits>(expectedProperties, [&recipient = expectedProperties.Recipient](auto& builder) {
			builder.setRecipientAddress(recipient);
		});
	}

	// endregion
}}
