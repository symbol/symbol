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
		using Modifications = std::vector<model::CosignatoryModification>;

		struct TransactionProperties {
		public:
			TransactionProperties()
					: MinRemovalDelta(0)
					, MinApprovalDelta(0)
			{}

		public:
			int8_t MinRemovalDelta;
			int8_t MinApprovalDelta;
			std::vector<model::CosignatoryModification> Modifications;
		};

		template<typename TTransaction>
		void AssertTransactionProperties(const TransactionProperties& expectedProperties, const TTransaction& transaction) {
			EXPECT_EQ(expectedProperties.MinRemovalDelta, transaction.MinRemovalDelta);
			EXPECT_EQ(expectedProperties.MinApprovalDelta, transaction.MinApprovalDelta);

			auto i = 0u;
			const auto* pModification = transaction.ModificationsPtr();
			for (const auto& modification : expectedProperties.Modifications) {
				auto rawAction = static_cast<uint16_t>(modification.ModificationAction);
				EXPECT_EQ(modification.ModificationAction, pModification->ModificationAction)
						<< "action " << rawAction << " at index " << i;
				EXPECT_EQ(modification.CosignatoryPublicKey, pModification->CosignatoryPublicKey) << "at index " << i;
				++pModification;
			}
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
			EXPECT_EQ(0x6201, pTransaction->Version);
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

	namespace {
		auto CreateModifications(uint8_t count) {
			Modifications modifications;
			for (auto i = 0u; i < count; ++i) {
				auto action = 0 == i % 2 ? model::CosignatoryModificationAction::Add : model::CosignatoryModificationAction::Del;
				modifications.push_back(model::CosignatoryModification{ action, test::GenerateRandomByteArray<Key>() });
			}

			return modifications;
		}

		void AddAll(MultisigAccountModificationBuilder& builder, const Modifications& modifications) {
			for (const auto& modification : modifications)
				builder.addModification(modification);
		}
	}

	TRAITS_BASED_TEST(CanAddSingleModification) {
		// Arrange:
		auto expectedProperties = TransactionProperties();
		expectedProperties.Modifications = CreateModifications(1);
		const auto& modifications = expectedProperties.Modifications;

		// Assert:
		auto totalCosignatorySize = sizeof(model::CosignatoryModification);
		AssertCanBuildTransaction<TTraits>(totalCosignatorySize, expectedProperties, [&modifications](auto& builder) {
			AddAll(builder, modifications);
		});
	}

	TRAITS_BASED_TEST(CanAddMultipleModifications) {
		// Arrange:
		auto expectedProperties = TransactionProperties();
		expectedProperties.Modifications = CreateModifications(5);
		const auto& modifications = expectedProperties.Modifications;

		// Assert:
		auto totalCosignatorySize = 5 * sizeof(model::CosignatoryModification);
		AssertCanBuildTransaction<TTraits>(totalCosignatorySize, expectedProperties, [&modifications](auto& builder) {
			AddAll(builder, modifications);
		});
	}

	TRAITS_BASED_TEST(CanSetDeltasAndAddModifications) {
		// Arrange:
		auto expectedProperties = TransactionProperties();
		expectedProperties.MinRemovalDelta = -3;
		expectedProperties.MinApprovalDelta = 3;
		expectedProperties.Modifications = CreateModifications(4);
		const auto& modifications = expectedProperties.Modifications;

		// Assert:
		auto totalCosignatorySize = 4 * sizeof(model::CosignatoryModification);
		AssertCanBuildTransaction<TTraits>(totalCosignatorySize, expectedProperties, [&modifications](auto& builder) {
			builder.setMinRemovalDelta(-3);
			builder.setMinApprovalDelta(3);
			AddAll(builder, modifications);
		});
	}

	// endregion
}}
