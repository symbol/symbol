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

#include "src/model/AccountMetadataTransaction.h"
#include "src/model/MosaicMetadataTransaction.h"
#include "src/model/NamespaceMetadataTransaction.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/core/VariableSizedEntityTestUtils.h"
#include "tests/test/nodeps/NumericTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS MetadataTransactionTests

	// region traits

	namespace {
		struct AccountTraits {
			using TransactionType = AccountMetadataTransaction;
			using EmbeddedTransactionType = EmbeddedAccountMetadataTransaction;
		};

		struct MosaicTraits {
			using TransactionType = MosaicMetadataTransaction;
			using EmbeddedTransactionType = EmbeddedMosaicMetadataTransaction;
		};

		struct NamespaceTraits {
			using TransactionType = NamespaceMetadataTransaction;
			using EmbeddedTransactionType = EmbeddedNamespaceMetadataTransaction;
		};
	}

#define METADATA_TYPE_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(AccountMetadataTransactionTests, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountTraits>(); } \
	TEST(MosaicMetadataTransactionTests, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MosaicTraits>(); } \
	TEST(NamespaceMetadataTransactionTests, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NamespaceTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region size + properties

	namespace {
		template<typename T>
		void AssertEntityHasExpectedSize(size_t baseSize, EntityType, size_t expectedTargetIdSize) {
			// Arrange:
			auto expectedSize =
					baseSize // base
					+ Key::Size // target public key
					+ sizeof(uint64_t) // scoped metadata key
					+ expectedTargetIdSize // target id
					+ sizeof(uint16_t) // previous value size
					+ sizeof(uint16_t); // value size

			// Assert:
			EXPECT_EQ(expectedSize, sizeof(T));
			EXPECT_EQ(baseSize + expectedTargetIdSize + 44u, sizeof(T));
		}

		template<typename T>
		void AssertTransactionHasExpectedProperties(EntityType expectedEntityType, size_t) {
			// Assert:
			EXPECT_EQ(expectedEntityType, T::Entity_Type);
			EXPECT_EQ(1u, T::Current_Version);
		}
	}

	ADD_BASIC_TRANSACTION_SIZE_PROPERTY_TESTS_WITH_ARGS(AccountMetadata, Entity_Type_Account_Metadata, 0)
	ADD_BASIC_TRANSACTION_SIZE_PROPERTY_TESTS_WITH_ARGS(MosaicMetadata, Entity_Type_Mosaic_Metadata, sizeof(uint64_t))
	ADD_BASIC_TRANSACTION_SIZE_PROPERTY_TESTS_WITH_ARGS(NamespaceMetadata, Entity_Type_Namespace_Metadata, sizeof(uint64_t))

	// endregion

	// region data pointers

	namespace {
		template<typename TTraits>
		struct MetadataTransactionTraits {
			static auto GenerateEntityWithAttachments(uint8_t count) {
				uint32_t entitySize = sizeof(typename TTraits::TransactionType) + count;
				auto pTransaction = utils::MakeUniqueWithSize<typename TTraits::TransactionType>(entitySize);
				pTransaction->Size = entitySize;
				pTransaction->ValueSize = count;
				return pTransaction;
			}

			template<typename TEntity>
			static auto GetAttachmentPointer(TEntity& entity) {
				return entity.ValuePtr();
			}
		};
	}

	DEFINE_ATTACHMENT_POINTER_TESTS(AccountMetadataTransactionTests, MetadataTransactionTraits<AccountTraits>) // ValuePtr
	DEFINE_ATTACHMENT_POINTER_TESTS(MosaicMetadataTransactionTests, MetadataTransactionTraits<MosaicTraits>) // ValuePtr
	DEFINE_ATTACHMENT_POINTER_TESTS(NamespaceMetadataTransactionTests, MetadataTransactionTraits<NamespaceTraits>) // ValuePtr

	// endregion

	// region CalculateRealSize

	METADATA_TYPE_BASED_TEST(CanCalculateRealSizeWithReasonableValues) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		transaction.Size = 0;
		transaction.ValueSize = 123;

		// Act:
		auto realSize = TTraits::TransactionType::CalculateRealSize(transaction);

		// Assert:
		EXPECT_EQ(sizeof(typename TTraits::TransactionType) + 123, realSize);
	}

	METADATA_TYPE_BASED_TEST(CalculateRealSizeDoesNotOverflowWithMaxValues) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		test::SetMaxValue(transaction.Size);
		test::SetMaxValue(transaction.ValueSize);

		// Act:
		auto realSize = TTraits::TransactionType::CalculateRealSize(transaction);

		// Assert:
		ASSERT_EQ(0xFFFFFFFF, transaction.Size);
		EXPECT_EQ(sizeof(typename TTraits::TransactionType) + 0xFFFF, realSize);
		EXPECT_GT(0xFFFFFFFF, realSize);
	}

	// endregion

	// region ExtractAdditionalRequiredCosigners

	METADATA_TYPE_BASED_TEST(ExtractAdditionalRequiredCosigners_ExtractsNothingWhenTargetPublicKeyIsEqualToSigner) {
		// Arrange:
		typename TTraits::EmbeddedTransactionType transaction;
		test::FillWithRandomData(transaction);
		transaction.TargetPublicKey = transaction.Signer;

		// Act:
		auto additionalCosigners = ExtractAdditionalRequiredCosigners(transaction);

		// Assert:
		EXPECT_EQ(utils::KeySet(), additionalCosigners);
	}

	METADATA_TYPE_BASED_TEST(ExtractAdditionalRequiredCosigners_ExtractsTargetPublicKeyWhenNotEqualToSigner) {
		// Arrange:
		typename TTraits::EmbeddedTransactionType transaction;
		test::FillWithRandomData(transaction);

		// Act:
		auto additionalCosigners = ExtractAdditionalRequiredCosigners(transaction);

		// Assert:
		EXPECT_EQ(utils::KeySet{ transaction.TargetPublicKey }, additionalCosigners);
	}

	// endregion
}}
