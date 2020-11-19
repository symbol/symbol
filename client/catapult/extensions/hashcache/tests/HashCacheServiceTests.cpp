/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "hashcache/src/HashCacheService.h"
#include "plugins/services/hashcache/src/cache/HashCacheStorage.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/local/ServiceLocatorTestContext.h"
#include "tests/test/local/ServiceTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace hashcache {

#define TEST_CLASS HashCacheServiceTests

	namespace {
		struct HashCacheServiceTraits {
			static constexpr auto CreateRegistrar = CreateHashCacheServiceRegistrar;
		};

		using TestContext = test::ServiceLocatorTestContext<HashCacheServiceTraits>;
	}

	ADD_SERVICE_REGISTRAR_INFO_TEST(HashCache, Initial)

	TEST(TEST_CLASS, NoServicesOrCountersAreRegistered) {
		test::AssertNoServicesOrCountersAreRegistered<TestContext>();
	}

	namespace {
		cache::CatapultCache CreateCache() {
			auto cacheId = cache::HashCache::Id;
			std::vector<std::unique_ptr<cache::SubCachePlugin>> subCaches(cacheId + 1);
			auto retentionTime = CalculateTransactionCacheDuration(model::BlockChainConfiguration::Uninitialized());
			subCaches[cacheId] = test::MakeSubCachePlugin<cache::HashCache, cache::HashCacheStorage>(retentionTime);
			return cache::CatapultCache(std::move(subCaches));
		}

		template<typename TAction>
		void RunHashCacheTest(TAction action) {
			// Arrange:
			TestContext context(CreateCache());
			context.boot();

			auto& cache = context.testState().state().cache();

			// - populate the hash cache
			auto hash = test::GenerateRandomByteArray<Hash256>();
			{
				auto delta = cache.createDelta();
				auto& hashCacheDelta = delta.template sub<cache::HashCache>();

				for (auto i = 0u; i < 5; ++i)
					hashCacheDelta.insert(state::TimestampedHash(Timestamp(i), test::GenerateRandomByteArray<Hash256>()));

				hashCacheDelta.insert(state::TimestampedHash(Timestamp(5), hash));

				for (auto i = 6u; i < 10; ++i)
					hashCacheDelta.insert(state::TimestampedHash(Timestamp(i), test::GenerateRandomByteArray<Hash256>()));

				cache.commit(Height());
			}

			// Act + Assert:
			action(context.testState().state().hooks(), hash);
		}
	}

	TEST(TEST_CLASS, KnownHashPredicate_ReturnsTrueWhenEntityIsContainedInHashCache) {
		// Act:
		RunHashCacheTest([](const auto& hooks, const auto& hash) {
			// Assert:
			cache::MemoryUtCache utCache({});
			EXPECT_TRUE(hooks.knownHashPredicate(utCache)(Timestamp(5), hash));
		});
	}

	TEST(TEST_CLASS, KnownHashPredicate_ReturnsFalseWhenEntityIsNotContainedInHashCache) {
		// Act:
		RunHashCacheTest([](const auto& hooks, const auto& hash) {
			// Assert:
			cache::MemoryUtCache utCache({});
			EXPECT_FALSE(hooks.knownHashPredicate(utCache)(Timestamp(6), hash));
			EXPECT_FALSE(hooks.knownHashPredicate(utCache)(Timestamp(5), test::GenerateRandomByteArray<Hash256>()));
		});
	}
}}
