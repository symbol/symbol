#include "src/cache/HashCache.h"
#include "tests/test/cache/CacheContentsTests.h"
#include "tests/test/cache/CacheIterationTests.h"
#include "tests/test/cache/CacheSynchronizationTests.h"
#include "tests/TestHarness.h"

#define TEST_CLASS HashCacheTests

namespace catapult { namespace cache {
	// region ctor

	TEST(TEST_CLASS, CanCreateHashCacheWithCustomRetentionTime) {
		// Act:
		HashCache cache(utils::TimeSpan::FromHours(123));

		// Assert:
		EXPECT_EQ(utils::TimeSpan::FromHours(123), cache.createView()->retentionTime());
		EXPECT_EQ(utils::TimeSpan::FromHours(123), cache.createDelta()->retentionTime());
		EXPECT_EQ(utils::TimeSpan::FromHours(123), cache.createDetachedDelta().lock()->retentionTime());
	}

	// endregion

	// region prune

	TEST(TEST_CLASS, PruningBoundaryIsInitiallyUnset) {
		// Arrange:
		HashCache cache(utils::TimeSpan::FromHours(123));
		auto delta = cache.createDelta();

		// Assert:
		EXPECT_FALSE(delta->pruningBoundary().isSet());
	}

	TEST(TEST_CLASS, PruneUpdatesPruningBoundary) {
		// Arrange:
		HashCache cache(utils::TimeSpan::FromHours(32));
		auto delta = cache.createDelta();

		// Act (40 hours):
		delta->prune(Timestamp(40 * 60 * 60 * 1000));
		auto pruningBoundary = delta->pruningBoundary();

		// Assert (40 - 32 hours):
		EXPECT_EQ(Timestamp(8 * 60 * 60 * 1000), pruningBoundary.value().Time);
		EXPECT_EQ(state::TimestampedHash::HashType(), pruningBoundary.value().Hash);
	}

	// endregion

	// region general cache tests

	namespace {
		struct HashCacheEntityTraits {
		public:
			static auto CreateEntity(size_t id) {
				return state::TimestampedHash(Timestamp(id), { { static_cast<uint8_t>(id * id) } });
			}

			static const auto& ToKey(const state::TimestampedHash& value) {
				return value;
			}
		};

		struct HashCacheTraits : test::SetCacheTraits<HashCache, state::TimestampedHash, HashCacheEntityTraits> {
		public:
			template<typename TAction>
			static void RunEmptyCacheTest(TAction action) {
				// Arrange:
				HashCache cache(utils::TimeSpan::FromHours(123));

				// Act:
				action(cache);
			}

			template<typename TAction>
			static void RunCacheTest(TAction action) {
				// Arrange:
				HashCache cache(utils::TimeSpan::FromHours(123));
				auto entities = InsertMultiple(cache, { 1, 4, 9 });

				// Act:
				action(cache, entities);
			}
		};
	}

	DEFINE_CACHE_CONTENTS_TESTS(TEST_CLASS, HashCacheTraits)
	DEFINE_CACHE_ITERATION_TESTS(TEST_CLASS, HashCacheTraits, Ordered)
	DEFINE_CACHE_SYNC_TESTS(TEST_CLASS, HashCacheTraits)

	// endregion
}}
