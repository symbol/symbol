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
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS TransactionTests

	// region structure + size

#ifndef _MSC_VER // disable for both gcc and clang
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof" // allow offsetof on non-standard layout Transaction type
#endif

	TEST(TEST_CLASS, StructureInheritancePreservesLayout) {
		// Assert: the derived fields should start where the base fields end
		ASSERT_EQ(sizeof(VerifiableEntity), offsetof(Transaction, MaxFee));
	}

#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif

	TEST(TEST_CLASS, EntityHasExpectedSize) {
		// Arrange:
		auto expectedSize =
				sizeof(VerifiableEntity) // base
				+ sizeof(uint64_t) // max fee
				+ sizeof(uint64_t); // deadline

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(Transaction));
		EXPECT_EQ(104u + 16u, sizeof(Transaction));
	}

	// endregion

	// region IsSizeValid

	namespace {
		bool IsSizeValid(const Transaction& transaction) {
			auto registry = mocks::CreateDefaultTransactionRegistry();
			return IsSizeValid(transaction, registry);
		}
	}

	TEST(TEST_CLASS, SizeIsInvalidForTransactionWithUnknownType) {
		// Arrange:
		Transaction transaction;
		transaction.Type = static_cast<EntityType>(-1);
		transaction.Size = sizeof(Transaction);

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(transaction));
	}

	namespace {
		std::unique_ptr<Transaction> CreateMockTransaction(uint32_t delta) {
			auto pTransaction = mocks::CreateMockTransaction(7);
			pTransaction->Size += delta;
			return std::move(pTransaction);
		}
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

	// endregion
}}
