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

#include "src/builders/SecretProofBuilder.h"
#include "tests/builders/test/BuilderTestUtils.h"

namespace catapult { namespace builders {

#define TEST_CLASS SecretProofBuilderTests

	namespace {
		using RegularTraits = test::RegularTransactionTraits<model::SecretProofTransaction>;
		using EmbeddedTraits = test::EmbeddedTransactionTraits<model::EmbeddedSecretProofTransaction>;

		struct TransactionProperties {
		public:
			TransactionProperties()
					: HashAlgorithm(model::LockHashAlgorithm::Op_Sha3_256)
					, Secret()
					, Recipient()
			{}

		public:
			model::LockHashAlgorithm HashAlgorithm;
			Hash256 Secret;
			UnresolvedAddress Recipient;
			RawBuffer Proof;
		};

		template<typename TTransaction>
		void AssertTransactionProperties(const TransactionProperties& expectedProperties, const TTransaction& transaction) {
			EXPECT_EQ(expectedProperties.HashAlgorithm, transaction.HashAlgorithm);
			EXPECT_EQ(expectedProperties.Secret, transaction.Secret);
			EXPECT_EQ(expectedProperties.Recipient, transaction.RecipientAddress);
			EXPECT_EQ_MEMORY(expectedProperties.Proof.pData, transaction.ProofPtr(), expectedProperties.Proof.Size);
		}

		template<typename TTraits>
		void AssertCanBuildTransaction(
				size_t additionalSize,
				const TransactionProperties& expectedProperties,
				const consumer<SecretProofBuilder&>& buildTransaction) {
			// Arrange:
			auto networkIdentifier = static_cast<model::NetworkIdentifier>(0x62);
			auto signer = test::GenerateRandomByteArray<Key>();

			// Act:
			SecretProofBuilder builder(networkIdentifier, signer);
			buildTransaction(builder);
			auto pTransaction = TTraits::InvokeBuilder(builder);

			// Assert:
			TTraits::CheckBuilderSize(additionalSize, builder);
			TTraits::CheckFields(additionalSize, *pTransaction);
			EXPECT_EQ(signer, pTransaction->SignerPublicKey);
			EXPECT_EQ(1u, pTransaction->Version);
			EXPECT_EQ(static_cast<model::NetworkIdentifier>(0x62), pTransaction->Network);
			EXPECT_EQ(model::Entity_Type_Secret_Proof, pTransaction->Type);

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
		AssertCanBuildTransaction<TTraits>(0, TransactionProperties(), [](const auto&) {});
	}

	// endregion

	// region additional transaction fields

	TRAITS_BASED_TEST(CanSetHashAlgorithm) {
		// Arrange:
		auto expectedProperties = TransactionProperties();
		expectedProperties.HashAlgorithm = model::LockHashAlgorithm::Op_Hash_160;

		// Assert:
		AssertCanBuildTransaction<TTraits>(0, expectedProperties, [](auto& builder) {
			builder.setHashAlgorithm(model::LockHashAlgorithm::Op_Hash_160);
		});
	}

	TRAITS_BASED_TEST(CanSetSecret) {
		// Arrange:
		auto expectedProperties = TransactionProperties();
		expectedProperties.Secret = test::GenerateRandomByteArray<Hash256>();

		// Assert:
		AssertCanBuildTransaction<TTraits>(0, expectedProperties, [&secret = expectedProperties.Secret](auto& builder) {
			builder.setSecret(secret);
		});
	}

	TRAITS_BASED_TEST(CanSetRecipient) {
		// Arrange:
		auto expectedProperties = TransactionProperties();
		test::FillWithRandomData(expectedProperties.Recipient);

		// Assert:
		AssertCanBuildTransaction<TTraits>(0, expectedProperties, [&recipient = expectedProperties.Recipient](auto& builder) {
			builder.setRecipientAddress(recipient);
		});
	}

	TRAITS_BASED_TEST(CanSetProof) {
		// Arrange:
		auto proof = test::GenerateRandomArray<20>();

		auto expectedProperties = TransactionProperties();
		expectedProperties.Proof = proof;

		// Assert:
		AssertCanBuildTransaction<TTraits>(20, expectedProperties, [&proof](auto& builder) {
			builder.setProof(proof);
		});
	}

	// endregion
}}
