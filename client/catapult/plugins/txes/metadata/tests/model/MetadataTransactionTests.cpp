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
#include "src/model/MetadataTypes.h"
#include "src/model/MosaicMetadataTransaction.h"
#include "src/model/NamespaceMetadataTransaction.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/core/VariableSizedEntityTestUtils.h"
#include "tests/test/nodeps/Alignment.h"
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

	// region size + alignment + properties

#define TRANSACTION_FIELDS FIELD(TargetAddress) FIELD(ScopedMetadataKey) FIELD(ValueSizeDelta) FIELD(ValueSize)

	namespace {
		template<typename T>
		void AssertTransactionHasExpectedSize(size_t baseSize, EntityType, size_t expectedTargetIdSize) {
			// Arrange:
			auto expectedSize = baseSize + expectedTargetIdSize;

#define FIELD(X) expectedSize += SizeOf32<decltype(T::X)>();
			TRANSACTION_FIELDS
#undef FIELD

			// Assert:
			EXPECT_EQ(expectedSize, sizeof(T));
			EXPECT_EQ(baseSize + expectedTargetIdSize + 36u, sizeof(T));
		}

		using AccountMetadataFlag = std::integral_constant<MetadataType, MetadataType::Account>;
		using MosaicMetadataFlag = std::integral_constant<MetadataType, MetadataType::Mosaic>;
		using NamespaceMetadataFlag = std::integral_constant<MetadataType, MetadataType::Namespace>;

		template<typename T, typename = void>
		struct MetadataTypeAccessor : public AccountMetadataFlag {};

		template<typename T>
		struct MetadataTypeAccessor<
				T,
				utils::traits::is_type_expression_t<decltype(reinterpret_cast<const T*>(0)->TargetMosaicId)>>
				: public MosaicMetadataFlag
		{};

		template<typename T>
		struct MetadataTypeAccessor<
				T,
				utils::traits::is_type_expression_t<decltype(reinterpret_cast<const T*>(0)->TargetNamespaceId)>>
				: public NamespaceMetadataFlag
		{};

		template<typename T>
		void AssertTransactionHasProperAlignment() {
#define FIELD(X) EXPECT_ALIGNED(T, X);
			TRANSACTION_FIELDS
#undef FIELD

			if constexpr (std::is_base_of_v<MosaicMetadataFlag, MetadataTypeAccessor<T>>)
				EXPECT_ALIGNED(T, TargetMosaicId);

			if constexpr (std::is_base_of_v<NamespaceMetadataFlag, MetadataTypeAccessor<T>>)
				EXPECT_ALIGNED(T, TargetNamespaceId);
		}

		template<typename T>
		void AssertTransactionHasExpectedProperties(EntityType expectedEntityType, size_t) {
			// Assert:
			EXPECT_EQ(expectedEntityType, T::Entity_Type);
			EXPECT_EQ(1u, T::Current_Version);
		}
	}

#undef TRANSACTION_FIELDS

	ADD_BASIC_TRANSACTION_SIZE_PROPERTY_TESTS_WITH_ARGS(AccountMetadata, Entity_Type_Account_Metadata, 0)
	ADD_BASIC_TRANSACTION_SIZE_PROPERTY_TESTS_WITH_ARGS(MosaicMetadata, Entity_Type_Mosaic_Metadata, sizeof(uint64_t))
	ADD_BASIC_TRANSACTION_SIZE_PROPERTY_TESTS_WITH_ARGS(NamespaceMetadata, Entity_Type_Namespace_Metadata, sizeof(uint64_t))

	// endregion

	// region data pointers

	namespace {
		template<typename TTraits>
		struct MetadataTransactionTraits {
			static auto GenerateEntityWithAttachments(uint8_t count) {
				uint32_t entitySize = SizeOf32<typename TTraits::TransactionType>() + count;
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

	// region ExtractAdditionalRequiredCosignatories

	METADATA_TYPE_BASED_TEST(ExtractAdditionalRequiredCosignatories_ExtractsTargetAddressWhenEqualToSigner) {
		// Arrange:
		typename TTraits::EmbeddedTransactionType transaction;
		test::FillWithRandomData(transaction);
		transaction.TargetAddress = GetSignerAddress(transaction).template copyTo<UnresolvedAddress>();

		// Act:
		auto additionalCosignatories = ExtractAdditionalRequiredCosignatories(transaction);

		// Assert:
		EXPECT_EQ(UnresolvedAddressSet{ transaction.TargetAddress }, additionalCosignatories);
	}

	METADATA_TYPE_BASED_TEST(ExtractAdditionalRequiredCosignatories_ExtractsTargetAddressWhenNotEqualToSigner) {
		// Arrange:
		typename TTraits::EmbeddedTransactionType transaction;
		test::FillWithRandomData(transaction);

		// Act:
		auto additionalCosignatories = ExtractAdditionalRequiredCosignatories(transaction);

		// Assert:
		EXPECT_EQ(UnresolvedAddressSet{ transaction.TargetAddress }, additionalCosignatories);
	}

	// endregion
}}
