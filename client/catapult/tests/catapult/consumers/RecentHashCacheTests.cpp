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

#include "catapult/consumers/RecentHashCache.h"
#include "tests/test/nodeps/TimeSupplier.h"
#include "tests/TestHarness.h"

namespace catapult { namespace consumers {

#define TEST_CLASS RecentHashCacheTests

	namespace {
		constexpr auto Default_Options = HashCheckOptions(600'000, 60'000, 1'000);
		constexpr auto CreateTimeSupplier = test::CreateTimeSupplierFromSeconds;

		chain::TimeSupplier DefaultTimeSupplier() {
			return []() { return Timestamp(1); };
		}

		RecentHashCache CreateDefaultCache() {
			return RecentHashCache(DefaultTimeSupplier(), Default_Options);
		}
	}

	// region ctor

	TEST(TEST_CLASS, CacheIsInitiallyEmpty) {
		// Act:
		auto cache = CreateDefaultCache();

		// Assert:
		EXPECT_EQ(0u, cache.size());
	}

	// endregion

	// region add

	TEST(TEST_CLASS, CanAddSingleUnknownHash) {
		// Arrange:
		auto cache = CreateDefaultCache();
		auto hash = test::GenerateRandomByteArray<Hash256>();

		// Act:
		auto result = cache.add(hash);

		// Assert:
		EXPECT_EQ(1u, cache.size());
		EXPECT_TRUE(cache.contains(hash));
		EXPECT_TRUE(result);
	}

	TEST(TEST_CLASS, CanAddMultipleUnknownHashes) {
		// Arrange:
		constexpr auto Num_Hashes = 5u;
		auto cache = CreateDefaultCache();
		auto hashes = test::GenerateRandomDataVector<Hash256>(Num_Hashes);
		std::vector<bool> results;

		// Act:
		for (const auto& hash : hashes)
			results.push_back(cache.add(hash));

		// Assert:
		EXPECT_EQ(Num_Hashes, results.size());
		EXPECT_EQ(Num_Hashes, cache.size());

		for (auto i = 0u; i < Num_Hashes; ++i) {
			auto message = "hash at index " + std::to_string(i);
			EXPECT_TRUE(results[i]) << message;
			EXPECT_TRUE(cache.contains(hashes[i])) << message;
		}
	}

	TEST(TEST_CLASS, HashIsNotAddedWhenItIsAlreadyKnown) {
		// Arrange:
		auto cache = CreateDefaultCache();
		auto hash = test::GenerateRandomByteArray<Hash256>();
		cache.add(hash);

		// Act:
		auto result = cache.add(hash);

		// Assert:
		EXPECT_EQ(1u, cache.size());
		EXPECT_FALSE(result);
	}

	TEST(TEST_CLASS, HashIsNotEvictedFromCacheBeforeCacheDuration) {
		// Arrange:
		RecentHashCache cache(CreateTimeSupplier({ 10, 11, 405, 612 }), Default_Options);
		auto hash1 = test::GenerateRandomByteArray<Hash256>();
		auto hash2 = test::GenerateRandomByteArray<Hash256>();

		// Act:
		auto result1 = cache.add(hash1); // t11
		auto result2 = cache.add(hash2); // t405 - triggers prune, but hash1 is not evicted because (405 - 11) == 394

		// Assert:
		EXPECT_EQ(2u, cache.size());
		EXPECT_TRUE(result1);
		EXPECT_TRUE(result2);
		EXPECT_TRUE(cache.contains(hash1));
		EXPECT_TRUE(cache.contains(hash2));
	}

	TEST(TEST_CLASS, HashIsNotEvictedFromCacheAtCacheDuration) {
		// Arrange:
		RecentHashCache cache(CreateTimeSupplier({ 10, 11, 611, 612 }), Default_Options);
		auto hash1 = test::GenerateRandomByteArray<Hash256>();
		auto hash2 = test::GenerateRandomByteArray<Hash256>();

		// Act:
		auto result1 = cache.add(hash1); // t11
		auto result2 = cache.add(hash2); // t611 - triggers prune, but hash1 is not evicted because (611 - 11) == 600

		// Assert:
		EXPECT_EQ(2u, cache.size());
		EXPECT_TRUE(result1);
		EXPECT_TRUE(result2);
		EXPECT_TRUE(cache.contains(hash1));
		EXPECT_TRUE(cache.contains(hash2));
	}

	TEST(TEST_CLASS, HashIsEvictedFromCacheAfterCacheDuration) {
		// Arrange:
		RecentHashCache cache(CreateTimeSupplier({ 10, 11, 612, 613 }), Default_Options);
		auto hash1 = test::GenerateRandomByteArray<Hash256>();
		auto hash2 = test::GenerateRandomByteArray<Hash256>();

		// Act:
		auto result1 = cache.add(hash1); // t11
		auto result2 = cache.add(hash2); // t612 - triggers prune, but hash1 is not evicted because (612 - 11) == 601

		// Assert: size is 1 because hash1 was removed
		EXPECT_EQ(1u, cache.size());
		EXPECT_TRUE(result1);
		EXPECT_TRUE(result2);
		EXPECT_FALSE(cache.contains(hash1));
		EXPECT_TRUE(cache.contains(hash2));
	}

	TEST(TEST_CLASS, SingleHashCannotSelfEvict) {
		// Arrange:
		RecentHashCache cache(CreateTimeSupplier({ 10, 11, 612, 613 }), Default_Options);
		auto hash = test::GenerateRandomByteArray<Hash256>();

		// Act:
		auto result1 = cache.add(hash); // t11
		auto result2 = cache.add(hash); // t612 - triggers prune but does not evict hash because hash time is updated to t612

		// Assert:
		EXPECT_EQ(1u, cache.size());
		EXPECT_TRUE(result1);
		EXPECT_FALSE(result2);
		EXPECT_TRUE(cache.contains(hash));
	}

	TEST(TEST_CLASS, MultiplePruningsCannotOccurWithinPruneInterval) {
		// Arrange:
		RecentHashCache cache(CreateTimeSupplier({ 10, 11, 553, 612, 613 }), Default_Options);
		auto hash1 = test::GenerateRandomByteArray<Hash256>();
		auto hash2 = test::GenerateRandomByteArray<Hash256>();

		// Act:
		auto result1 = cache.add(hash1); // t11
		auto result2 = cache.add(hash2); // t553 - triggers a prune but does not evict hash1
		auto result3 = cache.add(hash2); // t612 - does not trigger a prune because (612 - 553) < 60

		// Assert:
		EXPECT_EQ(2u, cache.size());
		EXPECT_TRUE(result1);
		EXPECT_TRUE(result2);
		EXPECT_FALSE(result3);
		EXPECT_TRUE(cache.contains(hash1));
		EXPECT_TRUE(cache.contains(hash2));
	}

	TEST(TEST_CLASS, MultiplePruningsCanOccurAtPruneInterval) {
		// Arrange:
		RecentHashCache cache(CreateTimeSupplier({ 10, 11, 552, 612, 613 }), Default_Options);
		auto hash1 = test::GenerateRandomByteArray<Hash256>();
		auto hash2 = test::GenerateRandomByteArray<Hash256>();

		// Act:
		auto result1 = cache.add(hash1); // t11
		auto result2 = cache.add(hash2); // t552 - triggers a prune but does not evict hash1
		auto result3 = cache.add(hash2); // t612 - triggers a prune and should evict hash1

		// Assert:
		EXPECT_EQ(1u, cache.size());
		EXPECT_TRUE(result1);
		EXPECT_TRUE(result2);
		EXPECT_FALSE(result3);
		EXPECT_FALSE(cache.contains(hash1));
		EXPECT_TRUE(cache.contains(hash2));
	}

	TEST(TEST_CLASS, SinglePruneCanEvictMultipleEntities) {
		// Arrange: create five hashes
		constexpr auto Num_Hashes = 5u;
		RecentHashCache cache(CreateTimeSupplier({ 10, 11, 12, 12, 14, 14, 613 }), Default_Options);
		auto hashes = test::GenerateRandomDataVector<Hash256>(Num_Hashes);
		std::vector<bool> results;

		// - cache the hashes at t11, t12, t12, t14, t14
		for (const auto& hash : hashes)
			results.push_back(cache.add(hash));

		// Act:
		auto result1 = cache.add(hashes[1]); // t613 - triggers a prune and should evict hashes[0] and hashes[2] (hashes[1] extends itself)

		// Assert: 2 hashes were pruned
		EXPECT_EQ(3u, cache.size());
		EXPECT_FALSE(result1);
		for (auto result : results)
			EXPECT_TRUE(result);

		for (auto i : { 1u, 3u, 4u })
			EXPECT_TRUE(cache.contains(hashes[i])) << "hash at index " << i;

		for (auto i : { 0u, 2u })
			EXPECT_FALSE(cache.contains(hashes[i])) << "hash at index " << i;
	}

	// endregion

	// region contains

	TEST(TEST_CLASS, ContainsReturnsTrueWhenHashIsKnown) {
		// Arrange:
		auto cache = CreateDefaultCache();
		auto hash1 = test::GenerateRandomByteArray<Hash256>();
		auto hash2 = test::GenerateRandomByteArray<Hash256>();
		cache.add(hash1);
		cache.add(hash2);

		// Assert:
		EXPECT_EQ(2u, cache.size());
		EXPECT_TRUE(cache.contains(hash1));
		EXPECT_TRUE(cache.contains(hash2));
	}

	TEST(TEST_CLASS, ContainsReturnsFalseWhenHashIsUnknown) {
		// Arrange:
		auto cache = CreateDefaultCache();
		for (auto i = 0u; i < 10; ++i)
			cache.add(test::GenerateRandomByteArray<Hash256>());

		// Assert:
		for (auto i = 0u; i < 5; ++i)
			EXPECT_FALSE(cache.contains(test::GenerateRandomByteArray<Hash256>()));
	}

	// endregion

	// region full cache

	namespace {
		constexpr auto Max_Cache_Size = 5u;
		constexpr auto Max_Cache_Size_Options = HashCheckOptions(600'000, 60'000, Max_Cache_Size);

		void FillCache(RecentHashCache& cache, const std::vector<Hash256>& hashes) {
			for (const auto& hash : hashes)
				cache.add(hash);
		}
	}

	TEST(TEST_CLASS, CanFillCache) {
		// Arrange:
		RecentHashCache cache(CreateTimeSupplier({ 10, 11, 12, 13, 14, 15 }), Max_Cache_Size_Options);
		auto hashes = test::GenerateRandomDataVector<Hash256>(Max_Cache_Size - 1);
		FillCache(cache, hashes); // t11..t14

		// Act: add another hash
		auto hash = test::GenerateRandomByteArray<Hash256>();
		auto result = cache.add(hash);

		// Assert: hash is unknown and was added
		EXPECT_TRUE(result);
		EXPECT_EQ(Max_Cache_Size, cache.size());
		EXPECT_TRUE(cache.contains(hash));
	}

	TEST(TEST_CLASS, CannotAddUnknownHashWhenCacheIsFull) {
		// Arrange:
		RecentHashCache cache(CreateTimeSupplier({ 10, 11, 12, 13, 14, 15, 15 }), Max_Cache_Size_Options);
		auto hashes = test::GenerateRandomDataVector<Hash256>(Max_Cache_Size);
		FillCache(cache, hashes); // t11..t15

		// Sanity:
		EXPECT_EQ(Max_Cache_Size, cache.size());

		// Act: try to add another hash
		auto hash = test::GenerateRandomByteArray<Hash256>();
		auto result = cache.add(hash);

		// Assert: hash is unknown but hash was not added
		EXPECT_EQ(Max_Cache_Size, cache.size());
		EXPECT_TRUE(result);
		EXPECT_FALSE(cache.contains(hash));
	}

	TEST(TEST_CLASS, CanAddUnknownHashWhenCacheIsFullButAtLeastOneHashIsEvicted) {
		// Arrange:
		RecentHashCache cache(CreateTimeSupplier({ 10, 11, 12, 13, 14, 15, 612 }), Max_Cache_Size_Options);
		auto hashes = test::GenerateRandomDataVector<Hash256>(Max_Cache_Size);
		FillCache(cache, hashes); // t11..t15

		// Sanity:
		EXPECT_EQ(Max_Cache_Size, cache.size());

		// Act: try to add another hash
		auto hash = test::GenerateRandomByteArray<Hash256>();
		auto result = cache.add(hash); // t612 - triggers a prune and should evict hashes[0]

		// Assert: hash is unknown and was added
		EXPECT_EQ(Max_Cache_Size, cache.size());
		EXPECT_TRUE(result);
		EXPECT_TRUE(cache.contains(hash));
		EXPECT_FALSE(cache.contains(hashes[0]));
	}

	// endregion

	// region SynchronizedRecentHashCache - add

	TEST(TEST_CLASS, SynchronizedRecentHashCache_AddBehaviorIsConsistentWithNonSynchronizedCache) {
		// Arrange:
		auto cache = SynchronizedRecentHashCache(DefaultTimeSupplier(), Default_Options);
		auto hashes = test::GenerateRandomDataVector<Hash256>(3);

		// Act:
		auto result1 = cache.add(hashes[0]);
		auto result2 = cache.add(hashes[1]);
		auto result3 = cache.add(hashes[0]); // duplicate
		auto result4 = cache.add(hashes[2]);

		// Assert:
		EXPECT_TRUE(result1);
		EXPECT_TRUE(result2);
		EXPECT_FALSE(result3);
		EXPECT_TRUE(result4);
	}

	// endregion
}}
