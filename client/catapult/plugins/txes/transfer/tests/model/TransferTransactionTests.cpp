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

#include "src/model/TransferTransaction.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/core/VariableSizedEntityTestUtils.h"
#include "tests/test/nodeps/NumericTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS TransferTransactionTests

	// region size + properties

	namespace {
		template<typename T>
		void AssertEntityHasExpectedSize(size_t baseSize) {
			// Arrange:
			auto expectedSize =
					baseSize // base
					+ sizeof(Address) // recipient
					+ sizeof(uint16_t) // message size
					+ sizeof(uint8_t); // mosaics count

			// Assert:
			EXPECT_EQ(expectedSize, sizeof(T));
			EXPECT_EQ(baseSize + 28u, sizeof(T));
		}

		template<typename T>
		void AssertTransactionHasExpectedProperties() {
			// Assert:
			EXPECT_EQ(Entity_Type_Transfer, T::Entity_Type);
			EXPECT_EQ(3u, T::Current_Version);
		}
	}

	ADD_BASIC_TRANSACTION_SIZE_PROPERTY_TESTS(Transfer)

	// endregion

	// region data pointers

	namespace {
		struct TransferTransactionTraits {
			static auto GenerateEntityWithAttachments(uint16_t messageSize, uint8_t numMosaics) {
				uint32_t entitySize = sizeof(TransferTransaction) + messageSize + numMosaics * sizeof(Mosaic);
				auto pTransaction = utils::MakeUniqueWithSize<TransferTransaction>(entitySize);
				pTransaction->Size = entitySize;
				pTransaction->MessageSize = messageSize;
				pTransaction->MosaicsCount = numMosaics;
				return pTransaction;
			}

			static constexpr size_t GetAttachment1Size(uint16_t messageSize) {
				return messageSize;
			}

			template<typename TEntity>
			static auto GetAttachmentPointer1(TEntity& entity) {
				return entity.MessagePtr();
			}

			template<typename TEntity>
			static auto GetAttachmentPointer2(TEntity& entity) {
				return entity.MosaicsPtr();
			}
		};
	}

	DEFINE_DUAL_ATTACHMENT_POINTER_TESTS(TEST_CLASS, TransferTransactionTraits)

	// endregion

	// region CalculateRealSize

	TEST(TEST_CLASS, CanCalculateRealSizeWithReasonableValues) {
		// Arrange:
		TransferTransaction transaction;
		transaction.Size = 0;
		transaction.MessageSize = 100;
		transaction.MosaicsCount = 7;

		// Act:
		auto realSize = TransferTransaction::CalculateRealSize(transaction);

		// Assert:
		EXPECT_EQ(16u, sizeof(Mosaic));
		EXPECT_EQ(sizeof(TransferTransaction) + 100 + 7 * sizeof(Mosaic), realSize);
	}

	TEST(TEST_CLASS, CalculateRealSizeDoesNotOverflowWithMaxValues) {
		// Arrange:
		TransferTransaction transaction;
		test::SetMaxValue(transaction.Size);
		test::SetMaxValue(transaction.MessageSize);
		test::SetMaxValue(transaction.MosaicsCount);

		// Act:
		auto realSize = TransferTransaction::CalculateRealSize(transaction);

		// Assert:
		ASSERT_EQ(0xFFFFFFFF, transaction.Size);
		EXPECT_EQ(sizeof(TransferTransaction) + 0xFFFF + 0xFF * sizeof(Mosaic), realSize);
		EXPECT_GT(0xFFFFFFFF, realSize);
	}

	// endregion
}}
