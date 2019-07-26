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

#include "src/builders/AccountAddressRestrictionBuilder.h"
#include "src/builders/AccountMosaicRestrictionBuilder.h"
#include "src/builders/AccountOperationRestrictionBuilder.h"
#include "sdk/tests/builders/test/BuilderTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"

namespace catapult { namespace builders {

#define TEST_CLASS AccountRestrictionBuilderTests

	namespace {
		template<typename TModification>
		struct TransactionPropertiesT {
		public:
			TransactionPropertiesT() : RestrictionType(model::AccountRestrictionType(0))
			{}

		public:
			model::AccountRestrictionType RestrictionType;
			std::vector<TModification> Modifications;
		};

		// region traits

		template<typename TRestrictionValue>
		struct ModificationTraitsMixin {
			using Modification = model::AccountRestrictionModification<TRestrictionValue>;
			using Modifications = std::vector<Modification>;
			using TransactionProperties = TransactionPropertiesT<Modification>;

			static constexpr size_t Restriction_Modification_Size = 1 + sizeof(TRestrictionValue);
		};

		struct AddressTraits : public ModificationTraitsMixin<UnresolvedAddress> {
			using BuilderType = AccountAddressRestrictionBuilder;
			using RegularTraits = test::RegularTransactionTraits<model::AccountAddressRestrictionTransaction>;
			using EmbeddedTraits = test::EmbeddedTransactionTraits<model::EmbeddedAccountAddressRestrictionTransaction>;

			static auto TransactionType() {
				return model::Entity_Type_Account_Address_Restriction;
			}

			static auto RandomUnresolvedValue() {
				return test::GenerateRandomUnresolvedAddress();
			}
		};

		struct MosaicTraits : public ModificationTraitsMixin<UnresolvedMosaicId> {
			using BuilderType = AccountMosaicRestrictionBuilder;
			using RegularTraits = test::RegularTransactionTraits<model::AccountMosaicRestrictionTransaction>;
			using EmbeddedTraits = test::EmbeddedTransactionTraits<model::EmbeddedAccountMosaicRestrictionTransaction>;

			static auto TransactionType() {
				return model::Entity_Type_Account_Mosaic_Restriction;
			}

			static auto RandomUnresolvedValue() {
				return test::GenerateRandomValue<UnresolvedMosaicId>();
			}
		};

		struct OperationTraits : public ModificationTraitsMixin<model::EntityType> {
			using BuilderType = AccountOperationRestrictionBuilder;
			using RegularTraits = test::RegularTransactionTraits<model::AccountOperationRestrictionTransaction>;
			using EmbeddedTraits = test::EmbeddedTransactionTraits<model::EmbeddedAccountOperationRestrictionTransaction>;

			static auto TransactionType() {
				return model::Entity_Type_Account_Operation_Restriction;
			}

			static auto RandomUnresolvedValue() {
				return static_cast<model::EntityType>(test::RandomByte());
			}
		};

		// endregion

		template<typename TTransactionProperties, typename TTransaction>
		void AssertTransactionProperties(const TTransactionProperties& expectedProperties, const TTransaction& transaction) {
			EXPECT_EQ(expectedProperties.RestrictionType, transaction.RestrictionType);

			auto i = 0u;
			const auto* pModification = transaction.ModificationsPtr();
			for (const auto& modification : expectedProperties.Modifications) {
				auto rawType = static_cast<uint16_t>(modification.ModificationType);
				EXPECT_EQ(modification.ModificationType, pModification->ModificationType) << "type " << rawType << " at index " << i;
				EXPECT_EQ(modification.Value, pModification->Value) << "at index " << i;
				++pModification;
			}
		}

		template<typename TTraits, typename TRestrictionTraits>
		void AssertCanBuildTransaction(
				size_t additionalSize,
				const typename TRestrictionTraits::TransactionProperties& expectedProperties,
				const consumer<typename TRestrictionTraits::BuilderType&>& buildTransaction) {
			// Arrange:
			auto networkId = static_cast<model::NetworkIdentifier>(0x62);
			auto signer = test::GenerateRandomByteArray<Key>();

			// Act:
			typename TRestrictionTraits::BuilderType builder(networkId, signer);
			buildTransaction(builder);
			auto pTransaction = TTraits::InvokeBuilder(builder);

			// Assert:
			TTraits::CheckBuilderSize(additionalSize, builder);
			TTraits::CheckFields(additionalSize, *pTransaction);
			EXPECT_EQ(signer, pTransaction->Signer);
			EXPECT_EQ(0x6201, pTransaction->Version);
			EXPECT_EQ(TRestrictionTraits::TransactionType(), pTransaction->Type);

			AssertTransactionProperties(expectedProperties, *pTransaction);
		}
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits, typename TRestrictionTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Regular_Address) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<typename AddressTraits::RegularTraits, AddressTraits>(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_Embedded_Address) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<typename AddressTraits::EmbeddedTraits, AddressTraits>(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_Regular_Mosaic) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<typename MosaicTraits::RegularTraits, MosaicTraits>(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_Embedded_Mosaic) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<typename MosaicTraits::EmbeddedTraits, MosaicTraits>(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_Regular_Operation) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<typename OperationTraits::RegularTraits, OperationTraits>(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_Embedded_Operation) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<typename OperationTraits::EmbeddedTraits, OperationTraits>(); \
	} \
	template<typename TTraits, typename TRestrictionTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region constructor

	TRAITS_BASED_TEST(CanCreateTransactionWithDefaultValues) {
		auto expectedProperties = typename TRestrictionTraits::TransactionProperties();
		AssertCanBuildTransaction<TTraits, TRestrictionTraits>(0, expectedProperties, [](const auto&) {});
	}

	// endregion

	// region Restriction type

	TRAITS_BASED_TEST(CanSetRestrictionType) {
		// Arrange:
		auto expectedProperties = typename TRestrictionTraits::TransactionProperties();
		expectedProperties.RestrictionType = model::AccountRestrictionType(12);

		// Assert:
		AssertCanBuildTransaction<TTraits, TRestrictionTraits>(0, expectedProperties, [](auto& builder) {
			builder.setRestrictionType(model::AccountRestrictionType(12));
		});
	}

	// endregion

	// region modifications

	namespace {
		template<typename TRestrictionTraits>
		auto CreateModifications(uint8_t count) {
			typename TRestrictionTraits::Modifications modifications;
			for (auto i = 0u; i < count; ++i) {
				auto type = 0 == i % 2 ? model::AccountRestrictionModificationType::Add : model::AccountRestrictionModificationType::Del;
				modifications.push_back({ type, TRestrictionTraits::RandomUnresolvedValue() });
			}

			return modifications;
		}

		template<typename TRestrictionTraits>
		void AddAll(typename TRestrictionTraits::BuilderType& builder, const typename TRestrictionTraits::Modifications& modifications) {
			for (const auto& modification : modifications)
				builder.addModification(modification);
		}
	}

	TRAITS_BASED_TEST(CanAddSingleModification) {
		// Arrange:
		auto expectedProperties = typename TRestrictionTraits::TransactionProperties();
		expectedProperties.Modifications = CreateModifications<TRestrictionTraits>(1);
		const auto& modifications = expectedProperties.Modifications;

		// Assert:
		auto totalCosignatorySize = TRestrictionTraits::Restriction_Modification_Size;
		AssertCanBuildTransaction<TTraits, TRestrictionTraits>(totalCosignatorySize, expectedProperties, [&modifications](auto& builder) {
			AddAll<TRestrictionTraits>(builder, modifications);
		});
	}

	TRAITS_BASED_TEST(CanAddMultipleModifications) {
		// Arrange:
		auto expectedProperties = typename TRestrictionTraits::TransactionProperties();
		expectedProperties.Modifications = CreateModifications<TRestrictionTraits>(5);
		const auto& modifications = expectedProperties.Modifications;

		// Assert:
		auto totalCosignatorySize = 5 * TRestrictionTraits::Restriction_Modification_Size;
		AssertCanBuildTransaction<TTraits, TRestrictionTraits>(totalCosignatorySize, expectedProperties, [&modifications](auto& builder) {
			AddAll<TRestrictionTraits>(builder, modifications);
		});
	}

	TRAITS_BASED_TEST(CanSetRestrictionTypeAndAddModifications) {
		// Arrange:
		auto expectedProperties = typename TRestrictionTraits::TransactionProperties();
		expectedProperties.RestrictionType = model::AccountRestrictionType(12);
		expectedProperties.Modifications = CreateModifications<TRestrictionTraits>(4);
		const auto& modifications = expectedProperties.Modifications;

		// Assert:
		auto totalCosignatorySize = 4 * TRestrictionTraits::Restriction_Modification_Size;
		AssertCanBuildTransaction<TTraits, TRestrictionTraits>(totalCosignatorySize, expectedProperties, [&modifications](auto& builder) {
			builder.setRestrictionType(model::AccountRestrictionType(12));
			AddAll<TRestrictionTraits>(builder, modifications);
		});
	}

	// endregion
}}
