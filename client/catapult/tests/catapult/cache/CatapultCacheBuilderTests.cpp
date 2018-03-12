#include "catapult/cache/CatapultCacheBuilder.h"
#include "tests/test/cache/SimpleCache.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS CatapultCacheBuilderTests

	namespace {
		template<size_t CacheId>
		void AddSubCacheWithId(CatapultCacheBuilder& builder) {
			builder.add<test::SimpleCacheStorageTraits>(std::make_unique<test::SimpleCacheT<CacheId>>());
		}

		size_t GetNumSubCaches(const CatapultCache& cache) {
			return cache.storages().size();
		}
	}

	TEST(TEST_CLASS, CanCreateEmptyCatapultCache) {
		// Arrange:
		CatapultCacheBuilder builder;

		// Act:
		auto cache = builder.build();

		// Assert:
		EXPECT_EQ(0u, GetNumSubCaches(cache));
	}

	TEST(TEST_CLASS, CanCreateCatapultCacheWithSingleSubCache) {
		// Arrange:
		CatapultCacheBuilder builder;
		AddSubCacheWithId<2>(builder);

		// Act:
		auto cache = builder.build();

		// Assert:
		EXPECT_EQ(1u, GetNumSubCaches(cache));
	}

	TEST(TEST_CLASS, CanCreateCatapultCacheWithMultipleSubCaches) {
		// Arrange:
		CatapultCacheBuilder builder;
		AddSubCacheWithId<2>(builder);
		AddSubCacheWithId<6>(builder);
		AddSubCacheWithId<4>(builder);

		// Act:
		auto cache = builder.build();

		// Assert:
		EXPECT_EQ(3u, GetNumSubCaches(cache));
	}

	TEST(TEST_CLASS, CannotAddMultipleSubCachesWithSameId) {
		// Arrange:
		CatapultCacheBuilder builder;
		AddSubCacheWithId<2>(builder);
		AddSubCacheWithId<6>(builder);
		AddSubCacheWithId<4>(builder);

		// Act + Assert:
		EXPECT_THROW(AddSubCacheWithId<6>(builder), catapult_invalid_argument);
	}
}}
