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

#include "catapult/model/Transaction.h"
#include "catapult/preprocessor.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/nodeps/Alignment.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS TransactionTests

	// region size + alignment

#define TRANSACTION_FIELDS FIELD(MaxFee) FIELD(Deadline)

	TEST(TEST_CLASS, TransactionHasExpectedSize) {
		// Arrange:
		auto expectedSize = sizeof(VerifiableEntity);

#define FIELD(X) expectedSize += SizeOf32<decltype(Transaction::X)>();
		TRANSACTION_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(Transaction));
		EXPECT_EQ(112u + 16, sizeof(Transaction));
	}

	TEST(TEST_CLASS, TransactionHasProperAlignment) {
#define FIELD(X) EXPECT_ALIGNED(Transaction, X);
		TRANSACTION_FIELDS
#undef FIELD
	}

#undef TRANSACTION_FIELDS

	// endregion

	// region IsSizeValid

	namespace {
		bool IsSizeValid(const Transaction& transaction, mocks::PluginOptionFlags options = mocks::PluginOptionFlags::Default) {
			auto registry = mocks::CreateDefaultTransactionRegistry(options);
			return IsSizeValid(transaction, registry);
		}

		std::unique_ptr<Transaction> CreateMockTransaction(uint32_t delta) {
			auto pTransaction = mocks::CreateMockTransaction(7);
			pTransaction->Size += delta;
			return PORTABLE_MOVE(pTransaction);
		}
	}

	TEST(TEST_CLASS, SizeIsInvalidForTransactionWithUnknownType) {
		// Arrange:
		Transaction transaction;
		transaction.Type = static_cast<EntityType>(std::numeric_limits<uint16_t>::max());
		transaction.Size = sizeof(Transaction);

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(transaction));
	}

	TEST(TEST_CLASS, SizeIsInvalidForTransactionWithoutTopLevelSupport) {
		// Arrange:
		auto pTransaction = CreateMockTransaction(0);

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pTransaction, mocks::PluginOptionFlags::Not_Top_Level));
	}

	TEST(TEST_CLASS, SizeIsValidForTransactionWithEqualReportedSizeAndActualSize) {
		// Arrange:
		auto pTransaction = CreateMockTransaction(0);

		// Act + Assert:
		EXPECT_TRUE(IsSizeValid(*pTransaction));
	}

	TEST(TEST_CLASS, SizeIsInvalidForTransactionWithReportedSizeLessThanActualSize) {
		// Arrange:
		auto pTransaction = CreateMockTransaction(static_cast<uint32_t>(-1));

		// Act:
		EXPECT_FALSE(IsSizeValid(*pTransaction));
	}

	TEST(TEST_CLASS, SizeIsInvalidForTransactionWithReportedSizeGreaterThanActualSize) {
		// Arrange:
		auto pTransaction = CreateMockTransaction(1);

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pTransaction));
	}

	TEST(TEST_CLASS, SizeIsInvalidForTransactionWithReportedSizeLessThanHeaderSize) {
		// Arrange:
		std::vector<uint8_t> buffer(sizeof(SizePrefixedEntity));
		auto* pTransaction = reinterpret_cast<Transaction*>(&buffer[0]);
		pTransaction->Size = sizeof(SizePrefixedEntity);

		// Act:
		EXPECT_FALSE(IsSizeValid(*pTransaction));
	}

	TEST(TEST_CLASS, SizeIsInvalidForTransactionWithReportedSizeLessThanDerivedHeaderSize) {
		// Arrange:
		auto pTransaction = std::make_unique<Transaction>();
		pTransaction->Type = mocks::MockTransaction::Entity_Type;
		pTransaction->Size = sizeof(Transaction);

		// Act:
		EXPECT_FALSE(IsSizeValid(*pTransaction));
	}

	// endregion
}}
