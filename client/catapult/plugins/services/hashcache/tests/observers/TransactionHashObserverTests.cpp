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

#include "src/observers/Observers.h"
#include "src/cache/HashCache.h"
#include "catapult/model/EntityHasher.h"
#include "tests/test/HashCacheTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

#define TEST_CLASS TransactionHashObserverTests

	using ObserverTestContext = test::ObserverTestContextT<test::HashCacheFactory>;

	DEFINE_COMMON_OBSERVER_TESTS(TransactionHash,)

	namespace {
		constexpr size_t Num_Hashes = 10;

		void SeedCache(cache::HashCacheDelta& cache) {
			for (auto i = 0u; i < Num_Hashes; ++i)
				cache.insert(state::TimestampedHash(Timestamp(i), test::GenerateRandomByteArray<Hash256>()));
		}

		state::TimestampedHash CreateTimestampedHash(Timestamp deadline, const Hash256& hash) {
			return state::TimestampedHash(deadline, hash);
		}

		model::TransactionNotification MakeNotification(Timestamp deadline, const Hash256& hash) {
			return model::TransactionNotification(Address(), hash, model::EntityType(), deadline);
		}
	}

	TEST(TEST_CLASS, ObserverInsertsHashIntoCacheInModeCommit) {
		// Arrange:
		ObserverTestContext context(NotifyMode::Commit);
		auto pObserver = CreateTransactionHashObserver();

		auto deadline = test::GenerateRandomValue<Timestamp>();
		auto hash = test::GenerateRandomByteArray<Hash256>();
		auto timestampedHash = CreateTimestampedHash(deadline, hash);

		auto& cache = context.observerContext().Cache.sub<cache::HashCache>();
		SeedCache(cache);

		// Sanity:
		EXPECT_EQ(Num_Hashes, cache.size());
		EXPECT_FALSE(cache.contains(timestampedHash));

		// Act:
		pObserver->notify(MakeNotification(deadline, hash), context.observerContext());

		// Assert:
		EXPECT_EQ(Num_Hashes + 1, cache.size());
		EXPECT_TRUE(cache.contains(timestampedHash));
	}

	TEST(TEST_CLASS, ObserverRemovesHashFromCacheInModeRollback) {
		// Arrange:
		ObserverTestContext context(NotifyMode::Rollback);
		auto pObserver = CreateTransactionHashObserver();

		auto deadline = test::GenerateRandomValue<Timestamp>();
		auto hash = test::GenerateRandomByteArray<Hash256>();
		auto timestampedHash = CreateTimestampedHash(deadline, hash);

		auto& cache = context.observerContext().Cache.sub<cache::HashCache>();
		SeedCache(cache);
		cache.insert(timestampedHash);

		// Sanity:
		EXPECT_EQ(Num_Hashes + 1, cache.size());
		EXPECT_TRUE(cache.contains(timestampedHash));

		// Act:
		pObserver->notify(MakeNotification(deadline, hash), context.observerContext());

		// Assert:
		EXPECT_EQ(Num_Hashes, cache.size());
		EXPECT_FALSE(cache.contains(timestampedHash));
	}
}}
