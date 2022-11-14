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

#include "src/builders/AccountAddressRestrictionBuilder.h"
#include "src/builders/AccountMosaicRestrictionBuilder.h"
#include "src/builders/AccountOperationRestrictionBuilder.h"
#include "sdk/tests/builders/test/BuilderTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"

namespace catapult { namespace builders {

#define TEST_CLASS AccountRestrictionBuilderTests

	namespace {
		template<typename TRestrictionValue>
		struct TransactionPropertiesT {
		public:
			TransactionPropertiesT() : RestrictionFlags(model::AccountRestrictionFlags(0))
			{}

		public:
			model::AccountRestrictionFlags RestrictionFlags;
			std::vector<TRestrictionValue> RestrictionAdditions;
			std::vector<TRestrictionValue> RestrictionDeletions;
		};

		// region traits

		template<typename TRestrictionValue>
		struct ModificationTraitsMixin {
			using TransactionProperties = TransactionPropertiesT<TRestrictionValue>;

			static constexpr size_t Restriction_Size = sizeof(TRestrictionValue);
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

		template<typename TRestrictionValue>
		void AssertValues(const std::vector<TRestrictionValue>& expectedValues, const TRestrictionValue* pValues, uint16_t numValues) {
			ASSERT_EQ(expectedValues.size(), numValues);

			auto i = 0u;
			for (const auto& expectedValue : expectedValues) {
				EXPECT_EQ(expectedValue, pValues[i]) << "value " << expectedValue << " at index " << i;
				++i;
			}
		}

		template<typename TTransactionProperties, typename TTransaction>
		void AssertTransactionProperties(const TTransactionProperties& expectedProperties, const TTransaction& transaction) {
			EXPECT_EQ(expectedProperties.RestrictionFlags, transaction.RestrictionFlags);

			AssertValues(
					expectedProperties.RestrictionAdditions,
					transaction.RestrictionAdditionsPtr(),
					transaction.RestrictionAdditionsCount);
			AssertValues(
					expectedProperties.RestrictionDeletions,
					transaction.RestrictionDeletionsPtr(),
					transaction.RestrictionDeletionsCount);
		}

		template<typename TTraits, typename TRestrictionTraits>
		void AssertCanBuildTransaction(
				size_t additionalSize,
				const typename TRestrictionTraits::TransactionProperties& expectedProperties,
				const consumer<typename TRestrictionTraits::BuilderType&>& buildTransaction) {
			// Arrange:
			auto networkIdentifier = static_cast<model::NetworkIdentifier>(0x62);
			auto signer = test::GenerateRandomByteArray<Key>();

			// Act:
			typename TRestrictionTraits::BuilderType builder(networkIdentifier, signer);
			buildTransaction(builder);
			auto pTransaction = TTraits::InvokeBuilder(builder);

			// Assert:
			TTraits::CheckBuilderSize(additionalSize, builder);
			TTraits::CheckFields(additionalSize, *pTransaction);
			EXPECT_EQ(signer, pTransaction->SignerPublicKey);
			EXPECT_EQ(1u, pTransaction->Version);
			EXPECT_EQ(static_cast<model::NetworkIdentifier>(0x62), pTransaction->Network);
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

	// region restriction flags

	TRAITS_BASED_TEST(CanSetRestrictionFlags) {
		// Arrange:
		auto expectedProperties = typename TRestrictionTraits::TransactionProperties();
		expectedProperties.RestrictionFlags = model::AccountRestrictionFlags(12);

		// Assert:
		AssertCanBuildTransaction<TTraits, TRestrictionTraits>(0, expectedProperties, [](auto& builder) {
			builder.setRestrictionFlags(model::AccountRestrictionFlags(12));
		});
	}

	// endregion

	// region modifications

	namespace {
		template<typename TRestrictionTraits>
		auto CreateRestrictionValues(uint8_t count) {
			std::vector<decltype(TRestrictionTraits::RandomUnresolvedValue())> values;
			for (auto i = 0u; i < count; ++i)
				values.push_back(TRestrictionTraits::RandomUnresolvedValue());

			return values;
		}
	}

	TRAITS_BASED_TEST(CanAddSingleValueAddition) {
		// Arrange:
		auto expectedProperties = typename TRestrictionTraits::TransactionProperties();
		expectedProperties.RestrictionAdditions = CreateRestrictionValues<TRestrictionTraits>(1);
		const auto& restrictionAdditions = expectedProperties.RestrictionAdditions;

		// Assert:
		auto additionalSize = TRestrictionTraits::Restriction_Size;
		AssertCanBuildTransaction<TTraits, TRestrictionTraits>(additionalSize, expectedProperties, [&restrictionAdditions](auto& builder) {
			for (const auto& value : restrictionAdditions)
				builder.addRestrictionAddition(value);
		});
	}

	TRAITS_BASED_TEST(CanAddSingleValueDeletion) {
		// Arrange:
		auto expectedProperties = typename TRestrictionTraits::TransactionProperties();
		expectedProperties.RestrictionDeletions = CreateRestrictionValues<TRestrictionTraits>(1);
		const auto& restrictionDeletions = expectedProperties.RestrictionDeletions;

		// Assert:
		auto additionalSize = TRestrictionTraits::Restriction_Size;
		AssertCanBuildTransaction<TTraits, TRestrictionTraits>(additionalSize, expectedProperties, [&restrictionDeletions](auto& builder) {
			for (const auto& value : restrictionDeletions)
				builder.addRestrictionDeletion(value);
		});
	}

	TRAITS_BASED_TEST(CanSetRestrictionFlagsAndAdditionsAndDeletions) {
		// Arrange:
		auto expectedProperties = typename TRestrictionTraits::TransactionProperties();
		expectedProperties.RestrictionFlags = model::AccountRestrictionFlags(12);
		expectedProperties.RestrictionAdditions = CreateRestrictionValues<TRestrictionTraits>(4);
		expectedProperties.RestrictionDeletions = CreateRestrictionValues<TRestrictionTraits>(2);
		const auto& restrictionAdditions = expectedProperties.RestrictionAdditions;
		const auto& restrictionDeletions = expectedProperties.RestrictionDeletions;

		// Assert:
		auto additionalSize = 6 * TRestrictionTraits::Restriction_Size;
		AssertCanBuildTransaction<TTraits, TRestrictionTraits>(additionalSize, expectedProperties, [&](auto& builder) {
			builder.setRestrictionFlags(model::AccountRestrictionFlags(12));

			for (const auto& value : restrictionAdditions)
				builder.addRestrictionAddition(value);

			for (const auto& value : restrictionDeletions)
				builder.addRestrictionDeletion(value);
		});
	}

	// endregion
}}
