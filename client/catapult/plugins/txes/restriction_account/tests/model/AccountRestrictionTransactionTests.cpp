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
#include "tests/test/nodeps/NumericTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS AccountRestrictionTransactionTests

	// region traits

	namespace {
		struct AddressTraits {
			using TransactionType = AccountAddressRestrictionTransaction;
			using ModificationType = AccountRestrictionModification<Address>;
		};

		struct MosaicTraits {
			using TransactionType = AccountMosaicRestrictionTransaction;
			using ModificationType = AccountRestrictionModification<UnresolvedMosaicId>;
		};

		struct OperationTraits {
			using TransactionType = AccountOperationRestrictionTransaction;
			using ModificationType = AccountRestrictionModification<EntityType>;
		};
	}

#define RESTRICTION_TYPE_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(AccountAddressRestrictionTransactionTests, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AddressTraits>(); } \
	TEST(AccountMosaicRestrictionTransactionTests, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MosaicTraits>(); } \
	TEST(AccountOperationRestrictionTransactionTests, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<OperationTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region size + properties

	namespace {
		template<typename T>
		void AssertEntityHasExpectedSize(size_t baseSize, EntityType) {
			// Arrange:
			auto expectedSize =
					baseSize // base
					+ sizeof(uint8_t) // account restriction type
					+ sizeof(uint8_t); // modifications count

			// Assert:
			EXPECT_EQ(expectedSize, sizeof(T));
			EXPECT_EQ(baseSize + 2u, sizeof(T));
		}

		template<typename T>
		void AssertTransactionHasExpectedProperties(EntityType expectedEntityType) {
			// Assert:
			EXPECT_EQ(expectedEntityType, T::Entity_Type);
			EXPECT_EQ(1u, T::Current_Version);
		}
	}

	ADD_BASIC_TRANSACTION_SIZE_PROPERTY_TESTS_WITH_ARGS(AccountAddressRestriction, Entity_Type_Account_Address_Restriction)
	ADD_BASIC_TRANSACTION_SIZE_PROPERTY_TESTS_WITH_ARGS(AccountMosaicRestriction, Entity_Type_Account_Mosaic_Restriction)
	ADD_BASIC_TRANSACTION_SIZE_PROPERTY_TESTS_WITH_ARGS(AccountOperationRestriction, Entity_Type_Account_Operation_Restriction)

	// endregion

	// region data pointers

	namespace {
		template<typename TTraits>
		struct AccountRestrictionTransactionTraits {
			static auto GenerateEntityWithAttachments(uint8_t count) {
				uint32_t entitySize = sizeof(typename TTraits::TransactionType);
				entitySize += 10 * count * sizeof(typename TTraits::ModificationType);
				auto pTransaction = utils::MakeUniqueWithSize<typename TTraits::TransactionType>(entitySize);
				pTransaction->Size = entitySize;
				pTransaction->ModificationsCount = 10 * count;
				return pTransaction;
			}

			template<typename TEntity>
			static auto GetAttachmentPointer(TEntity& entity) {
				return entity.ModificationsPtr();
			}
		};
	}

	DEFINE_ATTACHMENT_POINTER_TESTS(
			AccountAddressRestrictionTransactionTests,
			AccountRestrictionTransactionTraits<AddressTraits>) // ModificationsPtr
	DEFINE_ATTACHMENT_POINTER_TESTS(
			AccountMosaicRestrictionTransactionTests,
			AccountRestrictionTransactionTraits<MosaicTraits>) // ModificationsPtr
	DEFINE_ATTACHMENT_POINTER_TESTS(
			AccountOperationRestrictionTransactionTests,
			AccountRestrictionTransactionTraits<OperationTraits>) // ModificationsPtr

	// endregion

	// region CalculateRealSize

	RESTRICTION_TYPE_BASED_TEST(CanCalculateRealSizeWithReasonableValues) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		transaction.Size = 0;
		transaction.ModificationsCount = 123;

		// Act:
		auto realSize = TTraits::TransactionType::CalculateRealSize(transaction);

		// Assert:
		EXPECT_EQ(sizeof(typename TTraits::TransactionType) + 123u * sizeof(typename TTraits::ModificationType), realSize);
	}

	RESTRICTION_TYPE_BASED_TEST(CalculateRealSizeDoesNotOverflowWithMaxValues) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		test::SetMaxValue(transaction.Size);
		test::SetMaxValue(transaction.ModificationsCount);

		// Act:
		auto realSize = TTraits::TransactionType::CalculateRealSize(transaction);

		// Assert:
		ASSERT_EQ(0xFFFFFFFF, transaction.Size);
		EXPECT_EQ(sizeof(typename TTraits::TransactionType) + 0xFF * sizeof(typename TTraits::ModificationType), realSize);
		EXPECT_GT(0xFFFFFFFF, realSize);
	}

	// endregion
}}
