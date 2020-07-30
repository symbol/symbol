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

#include "src/model/AccountAddressRestrictionTransaction.h"
#include "src/model/AccountMosaicRestrictionTransaction.h"
#include "src/model/AccountOperationRestrictionTransaction.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/core/VariableSizedEntityTestUtils.h"
#include "tests/test/nodeps/Alignment.h"
#include "tests/test/nodeps/NumericTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS AccountRestrictionTransactionTests

	// region traits

	namespace {
		struct AddressTraits {
			using TransactionType = AccountAddressRestrictionTransaction;
			using RestrictionType = UnresolvedAddress;
		};

		struct MosaicTraits {
			using TransactionType = AccountMosaicRestrictionTransaction;
			using RestrictionType = UnresolvedMosaicId;
		};

		struct OperationTraits {
			using TransactionType = AccountOperationRestrictionTransaction;
			using RestrictionType = EntityType;
		};
	}

#define RESTRICTION_TYPE_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(AccountAddressRestrictionTransactionTests, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AddressTraits>(); } \
	TEST(AccountMosaicRestrictionTransactionTests, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MosaicTraits>(); } \
	TEST(AccountOperationRestrictionTransactionTests, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<OperationTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region size + alignment + properties

#define TRANSACTION_FIELDS FIELD(RestrictionFlags) FIELD(RestrictionAdditionsCount) FIELD(RestrictionDeletionsCount)

	namespace {
		template<typename T>
		void AssertTransactionHasExpectedSize(size_t baseSize, EntityType) {
			// Arrange:
			auto expectedSize = baseSize + sizeof(uint32_t);

#define FIELD(X) expectedSize += SizeOf32<decltype(T::X)>();
			TRANSACTION_FIELDS
#undef FIELD

			// Assert:
			EXPECT_EQ(expectedSize, sizeof(T));
			EXPECT_EQ(baseSize + 4 + 4u, sizeof(T));
		}

		template<typename T>
		void AssertTransactionHasProperAlignment() {
#define FIELD(X) EXPECT_ALIGNED(T, X);
			TRANSACTION_FIELDS
#undef FIELD

			EXPECT_EQ(0u, sizeof(T) % 8);
		}

		template<typename T>
		void AssertTransactionHasExpectedProperties(EntityType expectedEntityType) {
			// Assert:
			EXPECT_EQ(expectedEntityType, T::Entity_Type);
			EXPECT_EQ(1u, T::Current_Version);
		}
	}

#undef TRANSACTION_FIELDS

	ADD_BASIC_TRANSACTION_SIZE_PROPERTY_TESTS_WITH_ARGS(AccountAddressRestriction, Entity_Type_Account_Address_Restriction)
	ADD_BASIC_TRANSACTION_SIZE_PROPERTY_TESTS_WITH_ARGS(AccountMosaicRestriction, Entity_Type_Account_Mosaic_Restriction)
	ADD_BASIC_TRANSACTION_SIZE_PROPERTY_TESTS_WITH_ARGS(AccountOperationRestriction, Entity_Type_Account_Operation_Restriction)

	// endregion

	// region data pointers

	namespace {
		template<typename TTraits>
		struct AccountRestrictionTransactionTraits {
			static constexpr auto Restriction_Size = SizeOf32<typename TTraits::RestrictionType>();

			static auto GenerateEntityWithAttachments(uint8_t numAdditions, uint8_t numDeletions) {
				uint32_t entitySize = SizeOf32<typename TTraits::TransactionType>() + (numAdditions + numDeletions) * Restriction_Size;
				auto pTransaction = utils::MakeUniqueWithSize<typename TTraits::TransactionType>(entitySize);
				pTransaction->Size = entitySize;
				pTransaction->RestrictionAdditionsCount = numAdditions;
				pTransaction->RestrictionDeletionsCount = numDeletions;
				return pTransaction;
			}

			static constexpr size_t GetAttachment1Size(uint8_t numAdditions) {
				return numAdditions * Restriction_Size;
			}

			template<typename TEntity>
			static auto GetAttachmentPointer1(TEntity& entity) {
				return entity.RestrictionAdditionsPtr();
			}

			template<typename TEntity>
			static auto GetAttachmentPointer2(TEntity& entity) {
				return entity.RestrictionDeletionsPtr();
			}
		};
	}

	DEFINE_DUAL_ATTACHMENT_POINTER_TESTS(AccountAddressRestrictionTransactionTests, AccountRestrictionTransactionTraits<AddressTraits>)
	DEFINE_DUAL_ATTACHMENT_POINTER_TESTS(AccountMosaicRestrictionTransactionTests, AccountRestrictionTransactionTraits<MosaicTraits>)
	DEFINE_DUAL_ATTACHMENT_POINTER_TESTS(AccountOperationRestrictionTransactionTests, AccountRestrictionTransactionTraits<OperationTraits>)

	// endregion

	// region CalculateRealSize

	RESTRICTION_TYPE_BASED_TEST(CanCalculateRealSizeWithReasonableValues) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		transaction.Size = 0;
		transaction.RestrictionAdditionsCount = 7;
		transaction.RestrictionDeletionsCount = 4;

		// Act:
		auto realSize = TTraits::TransactionType::CalculateRealSize(transaction);

		// Assert:
		EXPECT_EQ(sizeof(typename TTraits::TransactionType) + 11u * sizeof(typename TTraits::RestrictionType), realSize);
	}

	RESTRICTION_TYPE_BASED_TEST(CalculateRealSizeDoesNotOverflowWithMaxValues) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		test::SetMaxValue(transaction.Size);
		test::SetMaxValue(transaction.RestrictionAdditionsCount);
		test::SetMaxValue(transaction.RestrictionDeletionsCount);

		// Act:
		auto realSize = TTraits::TransactionType::CalculateRealSize(transaction);

		// Assert:
		ASSERT_EQ(0xFFFFFFFF, transaction.Size);
		EXPECT_EQ(sizeof(typename TTraits::TransactionType) + (0xFF + 0xFF) * sizeof(typename TTraits::RestrictionType), realSize);
		EXPECT_GT(0xFFFFFFFF, realSize);
	}

	// endregion
}}
