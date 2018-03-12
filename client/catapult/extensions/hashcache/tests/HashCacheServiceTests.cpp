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
		// Assert:
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
			auto delta = cache.createDelta();
			auto& hashCacheDelta = delta.template sub<cache::HashCache>();

			for (auto i = 0u; i < 5u; ++i)
				hashCacheDelta.insert(state::TimestampedHash(Timestamp(i), test::GenerateRandomData<Hash256_Size>()));

			auto hash = test::GenerateRandomData<Hash256_Size>();
			hashCacheDelta.insert(state::TimestampedHash(Timestamp(5), hash));

			for (auto i = 6u; i < 10u; ++i)
				hashCacheDelta.insert(state::TimestampedHash(Timestamp(i), test::GenerateRandomData<Hash256_Size>()));

			cache.commit(Height());

			// Act + Assert:
			action(context.testState().state().hooks(), hash);
		}
	}

	TEST(TEST_CLASS, KnownHashPredicate_ReturnsTrueIfEntityIsContainedInHashCache) {
		// Act:
		RunHashCacheTest([](const auto& hooks, const auto& hash) {
			// Assert:
			cache::MemoryUtCache utCache({});
			EXPECT_TRUE(hooks.knownHashPredicate(utCache)(Timestamp(5), hash));
		});
	}

	TEST(TEST_CLASS, KnownHashPredicate_ReturnsFalseIfEntityIsNotContainedInHashCache) {
		// Act:
		RunHashCacheTest([](const auto& hooks, const auto& hash) {
			// Assert:
			cache::MemoryUtCache utCache({});
			EXPECT_FALSE(hooks.knownHashPredicate(utCache)(Timestamp(6), hash));
			EXPECT_FALSE(hooks.knownHashPredicate(utCache)(Timestamp(5), test::GenerateRandomData<Hash256_Size>()));
		});
	}
}}
