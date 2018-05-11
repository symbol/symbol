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

#include "harvesting/src/TransactionsInfo.h"
#include "catapult/cache/MemoryUtCache.h"
#include "tests/test/cache/UtTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace harvesting {

#define TEST_CLASS TransactionsInfoTests

	namespace {
		auto PrepareCache(size_t count) {
			auto pCache = std::make_unique<cache::MemoryUtCache>(cache::MemoryCacheOptions(1000, 1000));
			auto transactionInfos = test::CreateTransactionInfos(count);
			test::AddAll(*pCache, transactionInfos);
			return pCache;
		}

		void AssertSupplierBehavior(uint32_t count, uint32_t numRequested, uint32_t expectedCount) {
			// Arrange:
			auto pCache = PrepareCache(count);

			// Act:
			auto info = CreateTransactionsInfoSupplier(*pCache)(numRequested);

			// Assert:
			// - check transactions
			auto expectedTransactions = test::ExtractTransactions(*pCache, expectedCount);
			ASSERT_EQ(expectedCount, info.Transactions.size());
			for (auto i = 0u; i < expectedCount; ++i)
				EXPECT_EQ(*expectedTransactions[i], *info.Transactions[i]) << "transaction at " << i;

			// - check hash
			Hash256 expectedHash;
			CalculateBlockTransactionsHash(test::ExtractTransactionInfos(pCache->view(), expectedCount), expectedHash);
			EXPECT_EQ(expectedHash, info.TransactionsHash);
		}
	}

	TEST(TEST_CLASS, SupplierReturnsNoTransactionInfosIfCacheIsEmpty) {
		// Assert:
		AssertSupplierBehavior(0, 3, 0);
	}

	TEST(TEST_CLASS, SupplierReturnsNoTransactionInfosIfZeroInfosAreRequested) {
		// Assert:
		AssertSupplierBehavior(10, 0, 0);
	}

	TEST(TEST_CLASS, SupplierReturnsFirstCountTransactionInfosIfCacheHasEnoughTransactions) {
		// Assert:
		AssertSupplierBehavior(10, 3, 3);
	}

	TEST(TEST_CLASS, SupplierReturnsAllTransactionInfosIfCacheHasExactlyCountTransactions) {
		// Assert:
		AssertSupplierBehavior(10, 10, 10);
	}

	TEST(TEST_CLASS, SupplierReturnsAllTransactionInfosIfCacheHasLessThanCountTransactions) {
		// Assert:
		AssertSupplierBehavior(10, 15, 10);
	}
}}
