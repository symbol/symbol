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

#include "src/builders/MultisigAccountModificationBuilder.h"
#include "sdk/tests/builders/test/BuilderTestUtils.h"

namespace catapult { namespace builders {

#define TEST_CLASS MultisigAccountModificationBuilderTests

	namespace {
		using RegularTraits = test::RegularTransactionTraits<model::MultisigAccountModificationTransaction>;
		using EmbeddedTraits = test::EmbeddedTransactionTraits<model::EmbeddedMultisigAccountModificationTransaction>;

		struct TransactionProperties {
		public:
			TransactionProperties()
					: MinRemovalDelta(0)
					, MinApprovalDelta(0)
			{}

		public:
			int8_t MinRemovalDelta;
			int8_t MinApprovalDelta;
			std::vector<Key> PublicKeyAdditions;
			std::vector<Key> PublicKeyDeletions;
		};

		void AssertKeys(const std::vector<Key>& expectedKeys, const Key* pKeys, uint16_t numKeys) {
			ASSERT_EQ(expectedKeys.size(), numKeys);

			auto i = 0u;
			for (const auto& expectedKey : expectedKeys) {
				EXPECT_EQ(expectedKey, pKeys[i]) << "key " << expectedKey << " at index " << i;
				++i;
			}
		}

		template<typename TTransaction>
		void AssertTransactionProperties(const TransactionProperties& expectedProperties, const TTransaction& transaction) {
			EXPECT_EQ(expectedProperties.MinRemovalDelta, transaction.MinRemovalDelta);
			EXPECT_EQ(expectedProperties.MinApprovalDelta, transaction.MinApprovalDelta);

			AssertKeys(expectedProperties.PublicKeyAdditions, transaction.PublicKeyAdditionsPtr(), transaction.PublicKeyAdditionsCount);
			AssertKeys(expectedProperties.PublicKeyDeletions, transaction.PublicKeyDeletionsPtr(), transaction.PublicKeyDeletionsCount);
		}

		template<typename TTraits>
		void AssertCanBuildTransaction(
				size_t additionalSize,
				const TransactionProperties& expectedProperties,
				const consumer<MultisigAccountModificationBuilder&>& buildTransaction) {
			// Arrange:
			auto networkId = static_cast<model::NetworkIdentifier>(0x62);
			auto signer = test::GenerateRandomByteArray<Key>();

			// Act:
			MultisigAccountModificationBuilder builder(networkId, signer);
			buildTransaction(builder);
			auto pTransaction = TTraits::InvokeBuilder(builder);

			// Assert:
			TTraits::CheckBuilderSize(additionalSize, builder);
			TTraits::CheckFields(additionalSize, *pTransaction);
			EXPECT_EQ(signer, pTransaction->SignerPublicKey);
			EXPECT_EQ(1u, pTransaction->Version);
			EXPECT_EQ(static_cast<model::NetworkIdentifier>(0x62), pTransaction->Network);
			EXPECT_EQ(model::Entity_Type_Multisig_Account_Modification, pTransaction->Type);

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

	// region min cosignatory settings

	TRAITS_BASED_TEST(CanSetMinRemovalDelta) {
		// Arrange:
		auto expectedProperties = TransactionProperties();
		expectedProperties.MinRemovalDelta = 3;

		// Assert:
		AssertCanBuildTransaction<TTraits>(0, expectedProperties, [](auto& builder) {
			builder.setMinRemovalDelta(3);
		});
	}

	TRAITS_BASED_TEST(CanSetMinApprovalDelta) {
		// Arrange:
		auto expectedProperties = TransactionProperties();
		expectedProperties.MinApprovalDelta = 3;

		// Assert:
		AssertCanBuildTransaction<TTraits>(0, expectedProperties, [](auto& builder) {
			builder.setMinApprovalDelta(3);
		});
	}

	// endregion

	// region modifications

	TRAITS_BASED_TEST(CanAddSingleKeyAddition) {
		// Arrange:
		auto expectedProperties = TransactionProperties();
		expectedProperties.PublicKeyAdditions = test::GenerateRandomDataVector<Key>(1);
		const auto& publicKeyAdditions = expectedProperties.PublicKeyAdditions;

		// Assert:
		AssertCanBuildTransaction<TTraits>(Key::Size, expectedProperties, [&publicKeyAdditions](auto& builder) {
			for (const auto& key : publicKeyAdditions)
				builder.addPublicKeyAddition(key);
		});
	}

	TRAITS_BASED_TEST(CanAddSingleKeyDeletion) {
		// Arrange:
		auto expectedProperties = TransactionProperties();
		expectedProperties.PublicKeyDeletions = test::GenerateRandomDataVector<Key>(1);
		const auto& publicKeyDeletions = expectedProperties.PublicKeyDeletions;

		// Assert:
		AssertCanBuildTransaction<TTraits>(Key::Size, expectedProperties, [&publicKeyDeletions](auto& builder) {
			for (const auto& key : publicKeyDeletions)
				builder.addPublicKeyDeletion(key);
		});
	}

	TRAITS_BASED_TEST(CanSetDeltasAndAdditionsAndDeletions) {
		// Arrange:
		auto expectedProperties = TransactionProperties();
		expectedProperties.MinRemovalDelta = -3;
		expectedProperties.MinApprovalDelta = 3;
		expectedProperties.PublicKeyAdditions = test::GenerateRandomDataVector<Key>(4);
		expectedProperties.PublicKeyDeletions = test::GenerateRandomDataVector<Key>(2);
		const auto& publicKeyAdditions = expectedProperties.PublicKeyAdditions;
		const auto& publicKeyDeletions = expectedProperties.PublicKeyDeletions;

		// Assert:
		AssertCanBuildTransaction<TTraits>(6 * Key::Size, expectedProperties, [&publicKeyAdditions, publicKeyDeletions](auto& builder) {
			builder.setMinRemovalDelta(-3);
			builder.setMinApprovalDelta(3);

			for (const auto& key : publicKeyAdditions)
				builder.addPublicKeyAddition(key);

			for (const auto& key : publicKeyDeletions)
				builder.addPublicKeyDeletion(key);
		});
	}

	// endregion
}}
