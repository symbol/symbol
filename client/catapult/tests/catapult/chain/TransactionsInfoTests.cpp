#include "catapult/chain/TransactionsInfo.h"
#include "catapult/cache/MemoryUtCache.h"
#include "tests/test/cache/UnconfirmedTransactionsTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace chain {

	namespace {
		auto PrepareCache(size_t count) {
			auto pCache = std::make_unique<cache::MemoryUtCache>(cache::MemoryUtCacheOptions(1000, 1000));
			auto infos = test::CreateTransactionInfos(count);
			test::AddAll(*pCache, std::move(infos));
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

	TEST(TransactionInfosTests, SupplierReturnsNoTransactionInfosIfCacheIsEmpty) {
		// Assert:
		AssertSupplierBehavior(0, 3, 0);
	}

	TEST(TransactionInfosTests, SupplierReturnsNoTransactionInfosIfZeroInfosAreRequested) {
		// Assert:
		AssertSupplierBehavior(10, 0, 0);
	}

	TEST(TransactionInfosTests, SupplierReturnsFirstCountTransactionInfosIfCacheHasEnoughTransactions) {
		// Assert:
		AssertSupplierBehavior(10, 3, 3);
	}

	TEST(TransactionInfosTests, SupplierReturnsAllTransactionInfosIfCacheHasExactlyCountTransactions) {
		// Assert:
		AssertSupplierBehavior(10, 10, 10);
	}

	TEST(TransactionInfosTests, SupplierReturnsAllTransactionInfosIfCacheHasLessThanCountTransactions) {
		// Assert:
		AssertSupplierBehavior(10, 15, 10);
	}
}}
