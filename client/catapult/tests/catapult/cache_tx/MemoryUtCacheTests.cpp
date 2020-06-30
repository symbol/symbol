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

#include "catapult/cache_tx/MemoryUtCache.h"
#include "catapult/utils/ShortHash.h"
#include "tests/catapult/cache_tx/test/TransactionCacheTests.h"
#include "tests/test/cache/UtTestUtils.h"
#include "tests/test/core/EntityTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/nodeps/LockTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS MemoryUtCacheTests

	namespace {
		constexpr auto Default_Options = MemoryCacheOptions(1'000'000, 1'000);
		using UnknownTransactions = std::vector<std::shared_ptr<const model::Transaction>>;

		void AssertDeadlines(const UnknownTransactions& transactions, const std::vector<Timestamp::ValueType>& expectedDeadlines) {
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
			for (const auto& transactionInfo : transactionInfos)
				rawDeadlines.push_back(transactionInfo.pEntity->Deadline.unwrap());

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

		void AssertCacheSize(MemoryUtCache& cache, size_t expectedSize) {
			// Assert: check both view and modifier sizes
			EXPECT_EQ(cache.view().size(), expectedSize);
			EXPECT_EQ(cache.modifier().size(), expectedSize);
		}
	}

	// region constructor

	TEST(TEST_CLASS, InitiallyCacheIsEmpty) {
		// Act:
		MemoryUtCache cache(Default_Options);

		// Assert:
		AssertCacheSize(cache, 0);
	}

	// endregion

	// region size

	TEST(TEST_CLASS, ModifierSizeIsDynamic) {
		// Arrange:
		MemoryUtCache cache(Default_Options);

		// Act: modify cache and check sizes
		{
			auto modifier = cache.modifier();

			// Sanity:
			EXPECT_EQ(0u, modifier.size());

			// Act: add five
			auto originalTransactionInfos = test::CreateTransactionInfos(5);
			for (const auto& transactionInfo : originalTransactionInfos)
				EXPECT_TRUE(modifier.add(transactionInfo));

			// Sanity:
			EXPECT_EQ(5u, modifier.size());

			// Act: remove two
			modifier.remove(originalTransactionInfos[1].EntityHash);
			modifier.remove(originalTransactionInfos[4].EntityHash);

			// Assert:
			EXPECT_EQ(3u, modifier.size());
		}

		// Sanity:
		AssertCacheSize(cache, 3);
	}

	// endregion

	// region add

	TEST(TEST_CLASS, CanAddSingleTransactionInfo) {
		// Arrange:
		MemoryUtCache cache(Default_Options);
		auto originalInfo = test::CreateRandomTransactionInfo();

		// Act:
		EXPECT_TRUE(cache.modifier().add(originalInfo));

		// Assert:
		AssertCacheSize(cache, 1);

		auto view = cache.view();
		auto transactionInfos = test::ExtractTransactionInfos(view, 1);
		ASSERT_EQ(1u, transactionInfos.size());

		test::AssertEqual(originalInfo, *transactionInfos[0]);
	}

	TEST(TEST_CLASS, CanAddMultipleTransactionInfos) {
		// Arrange:
		MemoryUtCache cache(Default_Options);
		auto originalTransactionInfos = test::CreateTransactionInfos(5);

		// Act:
		for (const auto& transactionInfo : originalTransactionInfos)
			EXPECT_TRUE(cache.modifier().add(transactionInfo));

		// Assert:
		AssertCacheSize(cache, 5);

		auto view = cache.view();
		auto transactionInfos = test::ExtractTransactionInfos(view, 5);
		ASSERT_EQ(5u, transactionInfos.size());

		auto i = 0u;
		for (const auto& transactionInfo : originalTransactionInfos)
			test::AssertEqual(transactionInfo, *transactionInfos[i++]);
	}

	TEST(TEST_CLASS, AddingSameTransactionInfosTwiceHasNoEffect) {
		// Arrange:
		MemoryUtCache cache(Default_Options);
		auto originalTransactionInfo = test::CreateRandomTransactionInfo();
		EXPECT_TRUE(cache.modifier().add(originalTransactionInfo));

		// Sanity:
		AssertCacheSize(cache, 1);

		// Act:
		EXPECT_FALSE(cache.modifier().add(originalTransactionInfo));

		// Assert:
		AssertCacheSize(cache, 1);

		auto view = cache.view();
		auto transactionInfos = test::ExtractTransactionInfos(view, 1);
		ASSERT_EQ(1u, transactionInfos.size());

		test::AssertEqual(originalTransactionInfo, *transactionInfos[0]);
	}

	TEST(TEST_CLASS, AddedTransactionInfosHaveIncreasingIdsStartingWithOne) {
		// Arrange:
		MemoryUtCache cache(Default_Options);
		auto transactionInfos = test::CreateTransactionInfos(5);

		// Act:
		test::AddAll(cache, transactionInfos);

		// Assert:
		AssertCacheSize(cache, 5);
		test::AssertDeadlines(cache, { 1, 2, 3, 4, 5 });
	}

	// endregion

	// region remove

	TEST(TEST_CLASS, CanRemoveTransactionInfosByHash) {
		// Arrange:
		auto pCache = test::CreateSeededMemoryUtCache(10);
		auto hashes = ExtractEverySecondHash(*pCache);

		// Act:
		std::vector<model::TransactionInfo> removedTransactionInfos;
		for (const auto& hash : hashes)
			removedTransactionInfos.push_back(pCache->modifier().remove(hash));

		// Assert:
		AssertCacheSize(*pCache, 5);
		test::AssertDeadlines(*pCache, { 2, 4, 6, 8, 10 });

		// - all removed infos should contain original transaction
		EXPECT_EQ(5u, removedTransactionInfos.size());

		auto i = 0u;
		for (const auto& removedInfo : removedTransactionInfos) {
			EXPECT_EQ(Timestamp(2 * i + 1), removedInfo.pEntity->Deadline) << "removed info at " << i;
			++i;
		}
	}

	TEST(TEST_CLASS, RemovingNonexistentTransactionInfosByHashHasNoEffect) {
		// Arrange:
		auto pCache = test::CreateSeededMemoryUtCache(5);

		// Sanity:
		AssertCacheSize(*pCache, 5);
		test::AssertDeadlines(*pCache, { 1, 2, 3, 4, 5 });

		// Act:
		std::vector<model::TransactionInfo> removedTransactionInfos;
		for (auto i = 0u; i < 10; ++i)
			removedTransactionInfos.push_back(pCache->modifier().remove(test::GenerateRandomByteArray<Hash256>()));

		// Assert:
		AssertCacheSize(*pCache, 5);
		test::AssertDeadlines(*pCache, { 1, 2, 3, 4, 5 });

		// - all removed infos should be empty
		EXPECT_EQ(10u, removedTransactionInfos.size());

		auto i = 0u;
		for (const auto& removedInfo : removedTransactionInfos) {
			EXPECT_FALSE(!!removedInfo) << "removed info at " << i;
			++i;
		}
	}

	TEST(TEST_CLASS, CanAddNewTransactionInfoWithSameHashAsRemovedTransactionInfo) {
		// Arrange:
		auto pCache = test::CreateSeededMemoryUtCache(5);
		auto hashes = ExtractEverySecondHash(*pCache);
		pCache->modifier().remove(hashes[1]);

		// Sanity:
		AssertCacheSize(*pCache, 4);
		test::AssertDeadlines(*pCache, { 1, 2, 4, 5 });

		// Act:
		auto newTransactionInfo = test::CreateTransactionInfoWithDeadline(103);
		newTransactionInfo.EntityHash = hashes[1];
		pCache->modifier().add(std::move(newTransactionInfo));

		// Assert:
		AssertCacheSize(*pCache, 5);
		test::AssertDeadlines(*pCache, { 1, 2, 4, 5, 103 });
	}

	TEST(TEST_CLASS, AddingNewTransactionInfosAfterRemovingAddsAtTheEnd) {
		// Arrange:
		auto pCache = test::CreateSeededMemoryUtCache(10);
		auto hashes = ExtractEverySecondHash(*pCache);
		test::RemoveAll(*pCache, hashes);

		// Sanity:
		AssertCacheSize(*pCache, 5);
		test::AssertDeadlines(*pCache, { 2, 4, 6, 8, 10 });

		// Act:
		auto newTransactionInfos = test::CreateTransactionInfos(5);
		test::AddAll(*pCache, newTransactionInfos);

		// Assert:
		AssertCacheSize(*pCache, 10);
		test::AssertDeadlines(*pCache, { 2, 4, 6, 8, 10, 1, 2, 3, 4, 5 });
	}

	// endregion

	// region count

	namespace {
		struct CacheInitializationResult {
			Key SameSigner;
			std::vector<model::TransactionInfo> TransactionInfosWithSameSigner;
			std::vector<model::TransactionInfo> TransactionInfos;
		};

		std::vector<model::TransactionInfo> CreateTransactionInfos(const Key& signer, size_t count) {
			std::vector<model::TransactionInfo> transactionInfos;
			for (auto i = 0u; i < count; ++i) {
				auto transactionInfo = model::TransactionInfo(test::GenerateRandomTransaction(signer));
				test::FillWithRandomData(transactionInfo.EntityHash);
				transactionInfos.push_back(std::move(transactionInfo));
			}

			return transactionInfos;
		}

		CacheInitializationResult DefaultInitializeCache(MemoryUtCache& cache) {
			CacheInitializationResult initializationResult;
			initializationResult.SameSigner = test::GenerateRandomByteArray<Key>();
			initializationResult.TransactionInfosWithSameSigner = CreateTransactionInfos(initializationResult.SameSigner, 5);
			initializationResult.TransactionInfos = test::CreateTransactionInfos(8);
			test::AddAll(cache, initializationResult.TransactionInfosWithSameSigner);
			test::AddAll(cache, initializationResult.TransactionInfos);
			return initializationResult;
		}
	}

	TEST(TEST_CLASS, CountReturnsNumberOfTransactionsInCacheSignedByAccount) {
		// Arrange:
		MemoryUtCache cache(Default_Options);

		// - add some transactions with same signer and some transactions with random signers
		auto initializationResult = DefaultInitializeCache(cache);

		// Act:
		auto count = cache.modifier().count(initializationResult.SameSigner);

		// Assert:
		EXPECT_EQ(5u, count);
	}

	TEST(TEST_CLASS, AddUpdatesCounters) {
		// Arrange:
		MemoryUtCache cache(Default_Options);

		// - add some transactions with same signer and some transactions with random signers
		auto key = test::GenerateRandomByteArray<Key>();
		auto transactionInfosWithSameSigner = CreateTransactionInfos(key, 5);
		auto transactionInfos = test::CreateTransactionInfos(8);

		// Act:
		test::AddAll(cache, transactionInfosWithSameSigner);
		test::AddAll(cache, transactionInfos);

		// Assert:
		EXPECT_EQ(13u, cache.view().size());
		auto modifier = cache.modifier();
		EXPECT_EQ(5u, modifier.count(key));
		for (const auto& transactionInfo : transactionInfos)
			EXPECT_EQ(1u, modifier.count(transactionInfo.pEntity->SignerPublicKey));
	}

	TEST(TEST_CLASS, RemoveUpdatesCounters) {
		// Arrange:
		MemoryUtCache cache(Default_Options);

		// - add some transactions with same signer and some transactions with random signers
		auto initializationResult = DefaultInitializeCache(cache);

		// Sanity:
		EXPECT_EQ(13u, cache.view().size());
		EXPECT_EQ(5u, cache.modifier().count(initializationResult.SameSigner));

		// Act: remove 2 transaction infos
		cache.modifier().remove(initializationResult.TransactionInfosWithSameSigner[1].EntityHash);
		cache.modifier().remove(initializationResult.TransactionInfosWithSameSigner[4].EntityHash);

		// Assert:
		EXPECT_EQ(11u, cache.view().size());
		auto modifier = cache.modifier();
		EXPECT_EQ(3u, modifier.count(initializationResult.SameSigner));
		for (const auto& transactionInfo : initializationResult.TransactionInfos)
			EXPECT_EQ(1u, modifier.count(transactionInfo.pEntity->SignerPublicKey));
	}

	TEST(TEST_CLASS, RemoveAllUpdatesCounters) {
		// Arrange:
		MemoryUtCache cache(Default_Options);
		auto initializationResult = DefaultInitializeCache(cache);
		auto modifier = cache.modifier();

		// Sanity:
		EXPECT_EQ(5u, modifier.count(initializationResult.SameSigner));
		for (const auto& transactionsInfo : initializationResult.TransactionInfos)
			EXPECT_EQ(1u, modifier.count(transactionsInfo.pEntity->SignerPublicKey));

		// Act:
		modifier.removeAll();

		// Assert:
		EXPECT_EQ(0u, modifier.count(initializationResult.SameSigner));
		for (const auto& transactionsInfo : initializationResult.TransactionInfos)
			EXPECT_EQ(0u, modifier.count(transactionsInfo.pEntity->SignerPublicKey));
	}

	// endregion

	// region removeAll

	TEST(TEST_CLASS, CanRemoveAllTransactionsFromCache) {
		// Arrange:
		MemoryUtCache cache(Default_Options);
		auto transactionInfos = test::CreateTransactionInfos(5);
		test::AddAll(cache, transactionInfos);

		// Sanity:
		AssertCacheSize(cache, 5);
		test::AssertDeadlines(cache, { 1, 2, 3, 4, 5 });

		// Act:
		auto removedTransactionInfos = cache.modifier().removeAll();

		// Assert: cache was cleared and all removed transactions were returned
		AssertCacheSize(cache, 0);
		test::AssertContainsNone(cache, removedTransactionInfos);

		AssertDeadlines(removedTransactionInfos, { 1, 2, 3, 4, 5 });
		auto i = 0u;
		for (const auto& transactionInfo : transactionInfos)
			test::AssertEqual(transactionInfo, removedTransactionInfos[i++]);
	}

	// endregion

	// region contains

	TEST(TEST_CLASS, ContainsReturnsTrueWhenTransactionInfoIsContainedInCache) {
		// Arrange:
		MemoryUtCache cache(Default_Options);
		auto transactionInfos = test::CreateTransactionInfos(10);

		// Sanity:
		test::AssertContainsNone(cache, transactionInfos);

		test::AddAll(cache, transactionInfos);
		auto hashes = ExtractEverySecondHash(cache);

		// Assert:
		test::AssertContainsAll(cache, hashes);
	}

	TEST(TEST_CLASS, ContainsReturnsFalseWhenTransactionInfoIsNotContainedInCache) {
		// Arrange:
		auto pCache = test::CreateSeededMemoryUtCache(10);
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
		void AssertForEachBehavior(uint32_t count, size_t numRequested, size_t expectedCount) {
			// Arrange:
			auto pCache = test::CreateSeededMemoryUtCache(count);

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

	TEST(TEST_CLASS, ForEachForwardsNoTransactionInfosWhenCacheIsEmpty) {
		AssertForEachBehavior(0, 3, 0);
	}

	TEST(TEST_CLASS, ForEachForwardsAllTransactionsWhenNotShortCircuited) {
		AssertForEachBehavior(10, 100, 10);
	}

	TEST(TEST_CLASS, ForEachForwardsSubsetOfTransactionsWhenShortCircuited) {
		AssertForEachBehavior(10, 5, 5);
	}

	// endregion

	// region shortHashes

	TEST(TEST_CLASS, ShortHashesReturnsShortHashesForAllTransactions) {
		// Arrange:
		MemoryUtCache cache(Default_Options);
		auto transactionInfos = test::CreateTransactionInfos(10);
		std::vector<utils::ShortHash> expectedShortHashes;
		for (const auto& transactionInfo : transactionInfos)
			expectedShortHashes.push_back(utils::ToShortHash(transactionInfo.EntityHash));

		test::AddAll(cache, transactionInfos);

		// Act:
		auto shortHashes = cache.view().shortHashes();

		// Assert:
		EXPECT_EQ(expectedShortHashes.size(), shortHashes.size());

		auto i = 0u;
		auto iter = shortHashes.cbegin();
		for (const auto& expectedShortHash : expectedShortHashes) {
			EXPECT_EQ(expectedShortHash, *iter) << "at index " << i;
			++iter;
			++i;
		}
	}

	// endregion

	// region unknownTransactions

	namespace {
		struct MemoryUtCacheUnknownTransactionsTraits {
		public:
			using CacheType = MemoryUtCache;

		public:
			static UnknownTransactions GetUnknownTransactions(
					const MemoryUtCacheView& view,
					const utils::ShortHashesSet& knownShortHashes) {
				return view.unknownTransactions(BlockFeeMultiplier(10), knownShortHashes);
			}

			static void AddAllToCache(cache::UtCache& cache, const std::vector<model::TransactionInfo>& transactionInfos) {
				test::AddAll(cache, transactionInfos);
			}

			static void AssertUnknownResult(
					const std::vector<model::TransactionInfo>& expectedTransactionInfos,
					const std::vector<std::shared_ptr<const model::Transaction>>& unknownTransactions) {
				std::vector<Timestamp::ValueType> expectedDeadlines;
				for (const auto& transactionInfo : expectedTransactionInfos)
					expectedDeadlines.push_back(transactionInfo.pEntity->Deadline.unwrap());

				AssertDeadlines(unknownTransactions, expectedDeadlines);
			}

			static auto MapToFilterId(const model::TransactionInfo& transactionInfo) {
				return utils::ToShortHash(transactionInfo.EntityHash);
			}
		};
	}

	DEFINE_BASIC_UNKNOWN_TRANSACTIONS_TESTS(MemoryUtCacheTests, MemoryUtCacheUnknownTransactionsTraits)

	namespace {
		void AssertMaxResponseSizeIsRespected(uint32_t numExpectedTransactions, uint32_t maxResponseSize) {
			// Arrange:
			MemoryUtCache cache(MemoryCacheOptions(maxResponseSize, 1000));
			test::AddAll(cache, test::CreateTransactionInfos(5));

			// Act:
			auto transactions = cache.view().unknownTransactions(BlockFeeMultiplier(10), {});

			// Assert:
			EXPECT_EQ(numExpectedTransactions, transactions.size());
			EXPECT_GE(maxResponseSize, test::TotalSize(transactions));

			// - notice that transactions are ordered by id
			std::vector<Timestamp::ValueType> expectedRawDealines;
			for (auto i = 0u; i < numExpectedTransactions; ++i)
				expectedRawDealines.push_back(i + 1);

			AssertDeadlines(transactions, expectedRawDealines);
		}
	}

	TEST(TEST_CLASS, UnknownTransactionsReturnsTransactionsWithTotalSizeOfAtMostMaxResponseSize) {
		// Arrange: determine transaction size from a generated transaction
		auto transactionSize = test::CreateTransactionInfos(1)[0].pEntity->Size;

		// Assert:
		AssertMaxResponseSizeIsRespected(2, 3 * transactionSize - 1);
		AssertMaxResponseSizeIsRespected(3, 3 * transactionSize);
		AssertMaxResponseSizeIsRespected(3, 3 * transactionSize + 1);

		AssertMaxResponseSizeIsRespected(3, 4 * transactionSize - 1);
		AssertMaxResponseSizeIsRespected(4, 4 * transactionSize);
	}

	namespace {
		void AssertFeeMultiplierIsRespected(BlockFeeMultiplier feeMultiplier, const std::vector<Timestamp::ValueType>& expectedDeadlines) {
			// Arrange: determine transaction size from a generated transaction
			auto transactionSize = test::CreateTransactionInfos(1)[0].pEntity->Size;

			// - generate transactions with (deadline, fee multiples) { (1, 0x), (2, 20x), (3, 0x), (4, 60x) ... (10, 180x) }
			auto i = 0u;
			auto transactionInfos = test::CreateTransactionInfos(10);
			for (auto& transactionInfo : transactionInfos) {
				const_cast<Amount&>(transactionInfo.pEntity->MaxFee) = Amount(transactionSize * (0 == i % 2 ? 0 : i * 20));
				++i;
			}

			MemoryUtCache cache(Default_Options);
			test::AddAll(cache, transactionInfos);

			// Act:
			auto transactions = cache.view().unknownTransactions(feeMultiplier, {});

			// Assert:
			AssertDeadlines(transactions, expectedDeadlines);
		}
	}

	TEST(TEST_CLASS, UnknownTransactionsFiltersTransactionsByFeeMultiplier) {
		AssertFeeMultiplierIsRespected(BlockFeeMultiplier(59), { 4, 6, 8, 10 });
		AssertFeeMultiplierIsRespected(BlockFeeMultiplier(60), { 4, 6, 8, 10 });
		AssertFeeMultiplierIsRespected(BlockFeeMultiplier(61), { 6, 8, 10 });

		AssertFeeMultiplierIsRespected(BlockFeeMultiplier(0), { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 });
		AssertFeeMultiplierIsRespected(BlockFeeMultiplier(1), { 2, 4, 6, 8, 10 });
		AssertFeeMultiplierIsRespected(BlockFeeMultiplier(180), { 10 });
		AssertFeeMultiplierIsRespected(BlockFeeMultiplier(181), {});
	}

	// endregion

	// region max size

	namespace {
		auto CreateTransactionInfoWithDeadline(Timestamp deadline) {
			return test::CreateTransactionInfoWithDeadline(deadline.unwrap());
		}
	}

	TEST(TEST_CLASS, CacheCanContainMaxTransactions) {
		// Arrange: fill the cache with one less than max transactions
		MemoryUtCache cache(MemoryCacheOptions(1024, 5));
		test::AddAll(cache, test::CreateTransactionInfos(4));
		auto transactionInfo = CreateTransactionInfoWithDeadline(Timestamp(1234));

		// Act: add another info
		auto isAdded = cache.modifier().add(transactionInfo);

		// Assert: the new info was added
		EXPECT_TRUE(isAdded);
		AssertCacheSize(cache, 5);
		test::AssertDeadlines(cache, { 1, 2, 3, 4, 1234 });
	}

	TEST(TEST_CLASS, CacheCannotContainMoreThanMaxTransactions) {
		// Arrange: fill the cache with max transactions
		MemoryUtCache cache(MemoryCacheOptions(1024, 5));
		test::AddAll(cache, test::CreateTransactionInfos(5));
		auto transactionInfo = CreateTransactionInfoWithDeadline(Timestamp(1234));

		// Act: add another info
		auto isAdded = cache.modifier().add(transactionInfo);

		// Assert: the new info was not added
		EXPECT_FALSE(isAdded);
		AssertCacheSize(cache, 5);
		test::AssertDeadlines(cache, { 1, 2, 3, 4, 5 });
	}

	TEST(TEST_CLASS, CacheCanAcceptNewTransactionsAfterMaxTransactionsAreReduced) {
		// Arrange:
		MemoryUtCache cache(MemoryCacheOptions(1024, 5));
		auto transactionInfo = CreateTransactionInfoWithDeadline(Timestamp(1234));

		// - fill the cache with max transactions
		auto seedInfos = test::CreateTransactionInfos(5);
		auto seedHash = seedInfos[2].EntityHash;
		test::AddAll(cache, seedInfos);

		// Act: remove a transaction from the cache and add a new transaction
		auto isAdded = false;
		{
			auto modifier = cache.modifier();
			modifier.remove(seedHash);
			isAdded = modifier.add(transactionInfo);
		}

		// Assert: the new info was added
		EXPECT_TRUE(isAdded);
		AssertCacheSize(cache, 5);
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
