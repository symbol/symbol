#include "src/cache/HashCache.h"
#include "tests/test/cache/CacheBasicTests.h"
#include "tests/test/cache/CacheMixinsTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS HashCacheTests

	// region mixin traits based tests

	namespace {
		struct HashCacheMixinTraits {
			class CacheType : public HashCache {
			public:
				CacheType() : HashCache(utils::TimeSpan::FromMinutes(10))
				{}
			};

			using IdType = state::TimestampedHash;
			using ValueType = state::TimestampedHash;

			static uint8_t GetRawId(const IdType& id) {
				return static_cast<uint8_t>(id.Time.unwrap());
			}

			static const IdType& GetId(const ValueType& value) {
				return value;
			}

			static IdType MakeId(uint8_t id) {
				return state::TimestampedHash(Timestamp(id), { { static_cast<uint8_t>(id * id) } });
			}

			static ValueType CreateWithId(uint8_t id) {
				return MakeId(id);
			}
		};
	}

	DEFINE_CACHE_CONTAINS_TESTS(HashCacheMixinTraits, ViewAccessor, _View);
	DEFINE_CACHE_CONTAINS_TESTS(HashCacheMixinTraits, DeltaAccessor, _Delta);

	DEFINE_CACHE_ITERATION_TESTS_ORDERING(HashCacheMixinTraits, ViewAccessor, Ordered, _View);

	DEFINE_CACHE_MUTATION_TESTS(HashCacheMixinTraits, DeltaAccessor, _Delta);

	DEFINE_CACHE_BASIC_TESTS(HashCacheMixinTraits,);

	// endregion

	// *** custom tests ***

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
}}
