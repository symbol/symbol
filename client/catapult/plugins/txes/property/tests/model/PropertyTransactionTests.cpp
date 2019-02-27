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

#include "src/model/AddressPropertyTransaction.h"
#include "src/model/MosaicPropertyTransaction.h"
#include "src/model/TransactionTypePropertyTransaction.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/core/VariableSizedEntityTestUtils.h"
#include "tests/test/nodeps/NumericTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

	using TransactionType = AddressPropertyTransaction;

#define TEST_CLASS PropertyTransactionTests

	// region size + properties

	namespace {
		template<typename T>
		void AssertEntityHasExpectedSize(size_t baseSize) {
			// Arrange:
			auto expectedSize =
					baseSize // base
					+ sizeof(uint8_t) // property type
					+ sizeof(uint8_t); // modifications count

			// Assert:
			EXPECT_EQ(expectedSize, sizeof(T));
			EXPECT_EQ(baseSize + 2u, sizeof(T));
		}

		template<typename T>
		void AssertTransactionHasExpectedProperties() {
			// Assert:
			EXPECT_EQ(Entity_Type_Address_Property, T::Entity_Type);
			EXPECT_EQ(1u, T::Current_Version);
		}
	}

	ADD_BASIC_TRANSACTION_SIZE_PROPERTY_TESTS(AddressProperty)

	// endregion

	// region data pointers

	namespace {
		std::unique_ptr<AddressPropertyTransaction> GenerateAddressPropertyTransaction(uint8_t modificationsCount) {
			uint32_t entitySize = sizeof(AddressPropertyTransaction) + modificationsCount * sizeof(PropertyModification<Address>);
			auto pTransaction = utils::MakeUniqueWithSize<AddressPropertyTransaction>(entitySize);
			pTransaction->Size = entitySize;
			pTransaction->ModificationsCount = modificationsCount;
			return pTransaction;
		}

		struct AddressPropertyTransactionTraits {
			static auto GenerateEntityWithAttachments(uint8_t count) {
				return GenerateAddressPropertyTransaction(count * 10);
			}

			template<typename TEntity>
			static auto GetAttachmentPointer(TEntity& entity) {
				return entity.ModificationsPtr();
			}
		};
	}

	DEFINE_ATTACHMENT_POINTER_TESTS(TEST_CLASS, AddressPropertyTransactionTraits) // ModificationsPtr

	// endregion

	// region CalculateRealSize

	TEST(TEST_CLASS, CanCalculateRealSizeWithReasonableValues) {
		// Arrange:
		AddressPropertyTransaction transaction;
		transaction.Size = 0;
		transaction.ModificationsCount = 123;

		// Act:
		auto realSize = AddressPropertyTransaction::CalculateRealSize(transaction);

		// Assert:
		EXPECT_EQ(sizeof(AddressPropertyTransaction) + 123u * sizeof(PropertyModification<Address>), realSize);
	}

	TEST(TEST_CLASS, CalculateRealSizeDoesNotOverflowWithMaxValues) {
		// Arrange:
		AddressPropertyTransaction transaction;
		test::SetMaxValue(transaction.Size);
		test::SetMaxValue(transaction.ModificationsCount);

		// Act:
		auto realSize = AddressPropertyTransaction::CalculateRealSize(transaction);

		// Assert:
		ASSERT_EQ(0xFFFFFFFF, transaction.Size);
		EXPECT_EQ(sizeof(AddressPropertyTransaction) + 0xFF * sizeof(PropertyModification<Address>), realSize);
		EXPECT_GT(0xFFFFFFFF, realSize);
	}

	// endregion
}}
