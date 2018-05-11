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

#include "src/builders/ModifyMultisigAccountBuilder.h"
#include "sdk/tests/builders/test/BuilderTestUtils.h"

namespace catapult { namespace builders {

#define TEST_CLASS ModifyMultisigAccountBuilderTests

	namespace {
		using RegularTraits = test::RegularTransactionTraits<model::ModifyMultisigAccountTransaction>;
		using EmbeddedTraits = test::EmbeddedTransactionTraits<model::EmbeddedModifyMultisigAccountTransaction>;
		using Modifications = std::vector<model::CosignatoryModification>;

		template<typename TTraits, typename TValidationFunction>
		void AssertCanBuildTransaction(
				size_t additionalSize,
				const consumer<ModifyMultisigAccountBuilder&>& buildTransaction,
				const TValidationFunction& validateTransaction) {
			// Arrange:
			auto networkId = static_cast<model::NetworkIdentifier>(0x62);
			auto signer = test::GenerateRandomData<Key_Size>();

			// Act:
			ModifyMultisigAccountBuilder builder(networkId, signer);
			buildTransaction(builder);
			auto pTransaction = TTraits::InvokeBuilder(builder);

			// Assert:
			TTraits::CheckFields(additionalSize, *pTransaction);
			EXPECT_EQ(signer, pTransaction->Signer);
			EXPECT_EQ(0x6203, pTransaction->Version);
			EXPECT_EQ(model::Entity_Type_Modify_Multisig_Account, pTransaction->Type);

			validateTransaction(*pTransaction);
		}

		auto CreatePropertyChecker(int8_t minRemovalDelta, int8_t minApprovalDelta, const Modifications& modifications) {
			return [minRemovalDelta, minApprovalDelta, &modifications](const auto& transaction) {
				EXPECT_EQ(minRemovalDelta, transaction.MinRemovalDelta);
				EXPECT_EQ(minApprovalDelta, transaction.MinApprovalDelta);

				auto i = 0u;
				const auto* pModification = transaction.ModificationsPtr();
				for (const auto& modification : modifications) {
					EXPECT_EQ(modification.ModificationType, pModification->ModificationType)
							<< "type " << static_cast<uint16_t>(modification.ModificationType) << "at index " << i;
					EXPECT_EQ(modification.CosignatoryPublicKey, pModification->CosignatoryPublicKey) << "at index " << i;
					++pModification;
				}
			};
		}
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Regular) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<RegularTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Embedded) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<EmbeddedTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region constructor

	TRAITS_BASED_TEST(CanCreateTransactionWithDefaultValues) {
		// Assert:
		AssertCanBuildTransaction<TTraits>(0, [](const auto&) {}, CreatePropertyChecker(0, 0, {}));
	}

	// endregion

	// region min cosignatory settings

	TRAITS_BASED_TEST(CanSetMinRemovalDelta) {
		// Assert:
		AssertCanBuildTransaction<TTraits>(
				0,
				[](auto& builder) {
					builder.setMinRemovalDelta(3);
				},
				CreatePropertyChecker(3, 0, {}));
	}

	TRAITS_BASED_TEST(CanSetMinApprovalDelta) {
		// Assert:
		AssertCanBuildTransaction<TTraits>(
				0,
				[](auto& builder) {
					builder.setMinApprovalDelta(3);
				},
				CreatePropertyChecker(0, 3, {}));
	}

	// endregion

	// region modifications

	namespace {
		auto CreateModifications(uint8_t count) {
			Modifications modifications;
			for (auto i = 0u; i < count; ++i) {
				auto type = 0 == i % 2 ? model::CosignatoryModificationType::Add : model::CosignatoryModificationType::Del;
				modifications.push_back(model::CosignatoryModification{ type, test::GenerateRandomData<Key_Size>() });
			}

			return modifications;
		}

		void AddAll(ModifyMultisigAccountBuilder& builder, const Modifications& modifications) {
			for (const auto& modification : modifications)
				builder.addCosignatoryModification(modification.ModificationType, modification.CosignatoryPublicKey);
		}
	}

	TRAITS_BASED_TEST(CanAddSingleModification) {
		// Arrange:
		auto modifications = CreateModifications(1);

		// Assert:
		AssertCanBuildTransaction<TTraits>(
				sizeof(model::CosignatoryModification),
				[&modifications](auto& builder) {
					AddAll(builder, modifications);
				},
				CreatePropertyChecker(0, 0, modifications));
	}

	TRAITS_BASED_TEST(CanAddMultipleModifications) {
		// Arrange:
		auto modifications = CreateModifications(5);

		// Assert:
		AssertCanBuildTransaction<TTraits>(
				5 * sizeof(model::CosignatoryModification),
				[&modifications](auto& builder) {
					AddAll(builder, modifications);
				},
				CreatePropertyChecker(0, 0, modifications));
	}

	TRAITS_BASED_TEST(CanSetDeltasAndAddModifications) {
		// Arrange:
		auto modifications = CreateModifications(4);

		// Assert:
		AssertCanBuildTransaction<TTraits>(
				4 * sizeof(model::CosignatoryModification),
				[&modifications](auto& builder) {
					builder.setMinRemovalDelta(-3);
					builder.setMinApprovalDelta(3);
					AddAll(builder, modifications);
				},
				CreatePropertyChecker(-3, 3, modifications));
	}

	// endregion
}}
