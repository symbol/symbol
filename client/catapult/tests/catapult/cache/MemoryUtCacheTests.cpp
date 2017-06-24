#include "catapult/cache/MemoryUtCache.h"
#include "catapult/utils/ShortHash.h"
#include "tests/test/cache/UnconfirmedTransactionsTestUtils.h"
#include "tests/test/core/EntityTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/nodeps/LockTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

	namespace {
		constexpr auto Default_Options = MemoryUtCacheOptions(1'000'000, 1'000);
		using UnknownTransactions = std::vector<std::shared_ptr<const model::Transaction>>;

		void AssertDeadlines(
				const UnknownTransactions& transactions,
				const std::vector<Timestamp::ValueType>& expectedDeadlines) {
			// Arrange:
			std::vector<Timestamp::ValueType> rawDeadlines;
			for (const auto& pTransaction : transactions)
				rawDeadlines.push_back(pTransaction->Deadline.unwrap());

			// Assert:
			EXPECT_EQ(expectedDeadlines, rawDeadlines);
		}

		void AssertDeadlines(
				const std::vector<model::TransactionInfo>& transactionInfos,
				const std::vector<Timestamp::ValueType>& expectedDeadlines) {
			// Arrange:
			std::vector<Timestamp::ValueType> rawDeadlines;
			for (const auto& info : transactionInfos)
				rawDeadlines.push_back(info.pEntity->Deadline.unwrap());

			// Assert:
			EXPECT_EQ(expectedDeadlines, rawDeadlines);
		}

		std::vector<Hash256> ExtractEverySecondHash(const MemoryUtCache& cache) {
			size_t i = 0;
			std::vector<Hash256> hashes;
			auto view = cache.view();
			view.forEach([&i, &hashes](const auto& info) {
				if (0 == i % 2)
					hashes.push_back(info.EntityHash);

				i++;
				return true;
			});
			return hashes;
		}

		auto PrepareCache(size_t count, const MemoryUtCacheOptions& options = Default_Options) {
			auto pCache = std::make_unique<MemoryUtCache>(options);
			auto infos = test::CreateTransactionInfos(count);
			test::AddAll(*pCache, std::move(infos));
			return pCache;
		}
	}

	// region constructor / add

	TEST(MemoryUtCacheTests, InitiallyCacheIsEmpty) {
		// Act:
		MemoryUtCache cache(Default_Options);

		// Assert:
		EXPECT_EQ(0u, cache.view().size());
	}

	TEST(MemoryUtCacheTests, CanAddSingleTransactionInfo) {
		// Arrange:
		MemoryUtCache cache(Default_Options);
		auto pTransaction = std::shared_ptr<model::Transaction>(test::GenerateRandomTransaction());
		auto entityHash = test::GenerateRandomData<Hash256_Size>();
		auto merkleComponentHash = test::GenerateRandomData<Hash256_Size>();
		auto info = model::TransactionInfo(pTransaction, entityHash, merkleComponentHash);

		// Act:
		EXPECT_TRUE(cache.modifier().add(std::move(info)));

		// Assert:
		auto view = cache.view();
		auto infos = test::ExtractTransactionInfos(view, 1);
		EXPECT_EQ(1u, view.size());
		EXPECT_EQ(1u, infos.size());
		EXPECT_EQ(*pTransaction, *infos[0]->pEntity);
		EXPECT_EQ(entityHash, infos[0]->EntityHash);
		EXPECT_EQ(merkleComponentHash, infos[0]->MerkleComponentHash);
	}

	TEST(MemoryUtCacheTests, CanAddMultipleTransactionInfo) {
		// Arrange:
		MemoryUtCache cache(Default_Options);
		auto originalInfos = test::CreateTransactionInfos(5);

		// Act:
		for (const auto& info : originalInfos) {
			auto copy = info.copy();
			EXPECT_TRUE(cache.modifier().add(std::move(copy)));
		}

		// Assert:
		auto view = cache.view();
		auto infos = test::ExtractTransactionInfos(view, 5);
		EXPECT_EQ(5u, view.size());
		EXPECT_EQ(5u, infos.size());
		auto i = 0u;
		for (const auto& info : originalInfos)
			test::AssertEqual(info, *infos[i++]);
	}

	TEST(MemoryUtCacheTests, AddingSameTransactionInfosTwiceHasNoEffect) {
		// Arrange:
		MemoryUtCache cache(Default_Options);
		auto originalInfos = test::CreateTransactionInfos(1);
		auto pTransaction = test::GenerateRandomTransaction();
		auto copy = originalInfos[0].copy();
		EXPECT_TRUE(cache.modifier().add(std::move(copy)));

		// Sanity:
		EXPECT_EQ(1u, cache.view().size());

		// Act:
		auto copy2 = originalInfos[0].copy();
		EXPECT_FALSE(cache.modifier().add(std::move(copy2)));

		// Assert:
		auto view = cache.view();
		auto infos = test::ExtractTransactionInfos(view, 1);
		EXPECT_EQ(1u, view.size());
		EXPECT_EQ(1u, infos.size());
		test::AssertEqual(originalInfos[0], *infos[0]);
	}

	TEST(MemoryUtCacheTests, AddedTransactionInfosHaveIncreasingIdsStartingWithOne) {
		// Arrange:
		MemoryUtCache cache(Default_Options);
		auto infos = test::CreateTransactionInfos(5);

		// Act:
		test::AddAll(cache, std::move(infos));

		// Assert:
		EXPECT_EQ(5u, cache.view().size());
		test::AssertDeadlines(cache, { 1, 2, 3, 4, 5 });
	}

	// endregion

	// region remove

	TEST(MemoryUtCacheTests, CanRemoveTransactionInfosByHash) {
		// Arrange:
		auto pCache = PrepareCache(10);
		auto hashes = ExtractEverySecondHash(*pCache);

		// Act:
		for (const auto& hash : hashes)
			pCache->modifier().remove(hash);

		// Assert:
		EXPECT_EQ(5u, pCache->view().size());
		test::AssertDeadlines(*pCache, { 2, 4, 6, 8, 10 });
	}

	TEST(MemoryUtCacheTests, RemovingNonExistingTransactionInfosByHashHasNoEffect) {
		// Arrange:
		auto pCache = PrepareCache(5);

		// Sanity:
		EXPECT_EQ(5u, pCache->view().size());
		test::AssertDeadlines(*pCache, { 1, 2, 3, 4, 5 });

		// Act:
		for (auto i = 0u; i < 10; ++i)
			pCache->modifier().remove(test::GenerateRandomData<Hash256_Size>());

		// Assert:
		EXPECT_EQ(5u, pCache->view().size());
		test::AssertDeadlines(*pCache, { 1, 2, 3, 4, 5 });
	}

	TEST(MemoryUtCacheTests, AddingNewTransactionInfosAfterRemovingAddsAtTheEnd) {
		// Arrange:
		auto pCache = PrepareCache(10);
		auto hashes = ExtractEverySecondHash(*pCache);
		test::RemoveAll(*pCache, hashes);

		// Sanity:
		EXPECT_EQ(5u, pCache->view().size());
		test::AssertDeadlines(*pCache, { 2, 4, 6, 8, 10 });

		// Act:
		auto newInfos = test::CreateTransactionInfos(5);
		test::AddAll(*pCache, std::move(newInfos));

		// Assert:
		EXPECT_EQ(10u, pCache->view().size());
		test::AssertDeadlines(*pCache, { 2, 4, 6, 8, 10, 1, 2, 3, 4, 5 });
	}

	// endregion

	// region removeAll

	TEST(MemoryUtCacheTests, CanRemoveAllTransactionsFromCache) {
		// Arrange:
		auto pCache = std::make_unique<MemoryUtCache>(Default_Options);
		auto infos = test::CreateTransactionInfos(5);
		auto copyInfos = test::CopyTransactionInfos(infos);
		test::AddAll(*pCache, std::move(infos));

		// Sanity:
		EXPECT_EQ(5u, pCache->view().size());
		test::AssertDeadlines(*pCache, { 1, 2, 3, 4, 5 });

		// Act:
		auto removedTransactionInfos = pCache->modifier().removeAll();

		// Assert: cache was cleared and all removed transactions were returned
		EXPECT_EQ(0u, pCache->view().size());
		test::AssertContainsNone(*pCache, removedTransactionInfos);

		AssertDeadlines(removedTransactionInfos, { 1, 2, 3, 4, 5 });
		auto i = 0u;
		for (const auto& info : copyInfos)
			test::AssertEqual(info, removedTransactionInfos[i++]);
	}

	// endregion

	// region contains

	TEST(MemoryUtCacheTests, ContainsReturnsTrueIfTransactionInfoIsContainedInCache) {
		// Arrange:
		MemoryUtCache cache(Default_Options);
		auto infos = test::CreateTransactionInfos(10);

		// Sanity:
		test::AssertContainsNone(cache, infos);

		test::AddAll(cache, std::move(infos));
		auto hashes = ExtractEverySecondHash(cache);

		// Assert:
		test::AssertContainsAll(cache, hashes);
	}

	TEST(MemoryUtCacheTests, ContainsReturnsFalseIfTransactionInfoIsNotContainedInCache) {
		// Arrange:
		auto pCache = PrepareCache(10);
		auto hashes = ExtractEverySecondHash(*pCache);

		// Sanity:
		test::AssertContainsAll(*pCache, hashes);

		// Act: remove by hash
		test::RemoveAll(*pCache, hashes);

		// Assert:
		test::AssertContainsNone(*pCache, hashes);
	}

	// endregion

	// region forEach

	namespace {
		void AssertForEachBehavior(size_t count, size_t numRequested, size_t expectedCount) {
			// Arrange:
			auto pCache = PrepareCache(count);

			// Act:
			std::vector<std::shared_ptr<const model::Transaction>> transactions;
			pCache->view().forEach([numRequested, &transactions](const auto& info) {
				transactions.push_back(info.pEntity);
				return numRequested != transactions.size();
			});

			// Assert:
			auto expectedTransactions = test::ExtractTransactions(*pCache, expectedCount);
			ASSERT_EQ(expectedCount, transactions.size());
			for (auto i = 0u; i < expectedCount; ++i)
				EXPECT_EQ(*expectedTransactions[i], *transactions[i]) << "transaction at " << i;
		}
	}

	TEST(MemoryUtCacheTests, ForEachForwardsNoTransactionInfosIfCacheIsEmpty) {
		// Assert:
		AssertForEachBehavior(0, 3, 0);
	}

	TEST(MemoryUtCacheTests, ForEachForwardsAllTransactionsIfNotShortCircuited) {
		// Assert:
		AssertForEachBehavior(10, 100, 10);
	}

	TEST(MemoryUtCacheTests, ForEachForwardsSubsetOfTransactionsIfShortCircuited) {
		// Assert:
		AssertForEachBehavior(10, 5, 5);
	}

	// endregion

	// region prune

	namespace {
		void AssertPruning(size_t pruneCount) {
			// Arrange: transactions have deadlines with timestamp 1, 2, ..., 10
			auto pCache = PrepareCache(10);

			// Act:
			for (auto i = 0u; i < pruneCount; ++i)
				pCache->modifier().prune(Timestamp(5));

			// Assert:
			EXPECT_EQ(6u, pCache->view().size());
			test::AssertDeadlines(*pCache, { 5, 6, 7, 8, 9, 10 });
		}
	}

	TEST(MemoryUtCacheTests, PruneRemovesSequentialExpiredTransactions) {
		// Assert:
		AssertPruning(1);
	}

	TEST(MemoryUtCacheTests, PruneRemovesNonSequentialExpiredTransactions) {
		// Arrange:
		MemoryUtCache cache(Default_Options);
		auto infos = test::CreateTransactionInfos(10, [](auto i) {
			return Timestamp(0 == i % 2 ? i + 1 : 100 - i);
		});

		test::AddAll(cache, std::move(infos));

		// Sanity:
		EXPECT_EQ(10u, cache.view().size());
		test::AssertDeadlines(cache, { 1, 99, 3, 97, 5, 95, 7, 93, 9, 91 });

		// Act:
		cache.modifier().prune(Timestamp(50));

		// Assert: only the transaction whose deadlines were modified did not expire
		EXPECT_EQ(5u, cache.view().size());
		test::AssertDeadlines(cache, { 99, 97, 95, 93, 91 });
	}

	TEST(MemoryUtCacheTests, PruneIsIdempotent) {
		// Assert:
		AssertPruning(10);
	}

	// endregion

	// region shortHashes

	TEST(MemoryUtCacheTests, ShortHashesReturnsAllShortHashes) {
		// Arrange:
		auto pCache = PrepareCache(10);
		auto expectedShortHashes = test::ExtractShortHashes(*pCache);

		// Act:
		auto shortHashes = pCache->view().shortHashes();

		// Assert:
		EXPECT_EQ(expectedShortHashes.size(), shortHashes.size());

		auto i = 0u;
		auto iter = shortHashes.cbegin();
		for (const auto& shortHash : expectedShortHashes) {
			EXPECT_EQ(shortHash, *iter) << "at index " << i;
			++iter;
			++i;
		}
	}

	// endregion

	// region unknownTransactions

	TEST(MemoryUtCacheTests, UnknownTransactionsReturnsEmptyVectorIfCacheIsEmpty) {
		// Arrange:
		auto pCache = PrepareCache(0);

		// Act:
		auto transactions = pCache->view().unknownTransactions(utils::ShortHashesSet());

		// Assert:
		EXPECT_EQ(0u, transactions.size());
	}

	TEST(MemoryUtCacheTests, UnknownTransactionsReturnsAllTransactionsIfShortHashSetIsEmpty) {
		// Arrange:
		auto pCache = PrepareCache(5);

		// Act:
		auto transactions = pCache->view().unknownTransactions(utils::ShortHashesSet());

		// Assert:
		EXPECT_EQ(5u, transactions.size());
		AssertDeadlines(transactions, { 1, 2, 3, 4, 5 });
	}

	TEST(MemoryUtCacheTests, UnknownTransactionsReturnsAllTransactionsNotInShortHashSet) {
		// Arrange:
		auto pCache = PrepareCache(10);
		auto view = pCache->view();
		auto i = 0u;
		utils::ShortHashesSet shortHashes;
		auto infos = test::ExtractTransactionInfos(view, view.size());
		for (const auto* pInfo : infos) {
			if (0 == i % 2) {
				auto shortHash = *reinterpret_cast<const utils::ShortHash*>(pInfo->EntityHash.data());
				shortHashes.insert(shortHash);
			}

			++i;
		}

		// Act:
		auto transactions = view.unknownTransactions(shortHashes);

		// Assert:
		EXPECT_EQ(5u, transactions.size());
		AssertDeadlines(transactions, { 2, 4, 6, 8, 10 });
	}

	TEST(MemoryUtCacheTests, UnknownTransactionsReturnsEmptyVectorIfAllTransactionsAreKnown) {
		// Arrange:
		auto pCache = PrepareCache(10);
		auto view = pCache->view();
		utils::ShortHashesSet shortHashes;
		auto infos = test::ExtractTransactionInfos(view, view.size());
		for (const auto* pInfo : infos) {
			auto shortHash = *reinterpret_cast<const utils::ShortHash*>(pInfo->EntityHash.data());
			shortHashes.insert(shortHash);
		}

		// Act:
		auto transactions = view.unknownTransactions(shortHashes);

		// Assert:
		EXPECT_EQ(0u, transactions.size());
	}

	namespace {
		void AssertMaxResponseSizeIsRespected(uint32_t maxResponseSize, uint32_t numExpectedTransactions) {
			// Arrange:
			auto pCache = PrepareCache(5, MemoryUtCacheOptions(maxResponseSize, 1000));

			// Act:
			auto transactions = pCache->view().unknownTransactions(utils::ShortHashesSet());

			// Assert:
			EXPECT_EQ(numExpectedTransactions, transactions.size());
			EXPECT_GE(maxResponseSize, test::TotalSize(transactions));

			std::vector<Timestamp::ValueType> expectedRawDealines;
			for (auto i = 0u; i < numExpectedTransactions; ++i)
				expectedRawDealines.push_back(i + 1);

			AssertDeadlines(transactions, expectedRawDealines);
		}
	}

	TEST(MemoryUtCacheTests, UnknownTransactionsReturnsTransactionsWithTotalSizeOfAtMostMaxResponseSize) {
		// Arrange: determine transaction size from a generated transaction
		auto transactionSize = test::CreateTransactionInfos(1)[0].pEntity->Size;

		// Assert:
		AssertMaxResponseSizeIsRespected(3 * transactionSize - 1, 2);
		AssertMaxResponseSizeIsRespected(3 * transactionSize, 3);
		AssertMaxResponseSizeIsRespected(3 * transactionSize + 1, 3);
		AssertMaxResponseSizeIsRespected(3 * transactionSize + 10, 3);
		AssertMaxResponseSizeIsRespected(4 * transactionSize - 1, 3);
		AssertMaxResponseSizeIsRespected(4 * transactionSize, 4);
	}

	// endregion

	// region max size

	namespace {
		auto CreateTransactionInfoWithDeadline(Timestamp deadline) {
			auto pTransaction = test::GenerateRandomTransaction();
			pTransaction->Deadline = deadline;
			return model::TransactionInfo(
					std::move(pTransaction),
					test::GenerateRandomData<Hash256_Size>(),
					test::GenerateRandomData<Hash256_Size>());
		}
	}

	TEST(MemoryUtCacheTests, CacheCanContainMaxTransactions) {
		// Arrange: fill the cache with one less than max transactions
		MemoryUtCache cache(MemoryUtCacheOptions(1024, 5));
		test::AddAll(cache, test::CreateTransactionInfos(4));
		auto info = CreateTransactionInfoWithDeadline(Timestamp(1234));

		// Act: add another info
		auto isAdded = cache.modifier().add(std::move(info));

		// Assert: the new info was added
		EXPECT_TRUE(isAdded);
		EXPECT_EQ(5u, cache.view().size());
		test::AssertDeadlines(cache, { 1, 2, 3, 4, 1234 });
	}

	TEST(MemoryUtCacheTests, CacheCannotContainMoreThanMaxTransactions) {
		// Arrange: fill the cache with max transactions
		MemoryUtCache cache(MemoryUtCacheOptions(1024, 5));
		test::AddAll(cache, test::CreateTransactionInfos(5));
		auto info = CreateTransactionInfoWithDeadline(Timestamp(1234));

		// Act: add another info
		auto isAdded = cache.modifier().add(std::move(info));

		// Assert: the new info was not added
		EXPECT_FALSE(isAdded);
		EXPECT_EQ(5u, cache.view().size());
		test::AssertDeadlines(cache, { 1, 2, 3, 4, 5 });
	}

	TEST(MemoryUtCacheTests, CacheCanAcceptNewTransactionsAfterMaxTransactionsAreReduced) {
		// Arrange:
		MemoryUtCache cache(MemoryUtCacheOptions(1024, 5));
		auto info = CreateTransactionInfoWithDeadline(Timestamp(1234));

		// - fill the cache with max transactions
		auto seedInfos = test::CreateTransactionInfos(5);
		auto seedHash = seedInfos[2].EntityHash;
		test::AddAll(cache, std::move(seedInfos));

		// Act: remove a transaction from the cache and add a new transaction
		auto isAdded = false;
		{
			auto modifier = cache.modifier();
			modifier.remove(seedHash);
			isAdded = modifier.add(std::move(info));
		}

		// Assert: the new info was added
		EXPECT_TRUE(isAdded);
		EXPECT_EQ(5u, cache.view().size());
		test::AssertDeadlines(cache, { 1, 2, 4, 5, 1234 });
	}

	// endregion

	// region synchronization

	namespace {
		auto CreateLockProvider() {
			return std::make_unique<MemoryUtCache>(Default_Options);
		}
	}

	DEFINE_LOCK_PROVIDER_TESTS(MemoryUtCacheTests)

	// endregion
}}
