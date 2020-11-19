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

#include "src/cache/HashCache.h"
#include "tests/test/cache/CacheBasicTests.h"
#include "tests/test/cache/CacheMixinsTests.h"
#include "tests/test/cache/DeltaElementsMixinTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS HashCacheTests

	// region mixin traits based tests

	namespace {
		struct HashCacheMixinTraits {
			class CacheType : public HashCache {
			public:
				CacheType() : HashCache(CacheConfiguration(), utils::TimeSpan::FromMinutes(10))
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

		// custom modification policy is needed because elements are immutable
		struct HashCacheDeltaModificationPolicy : public test::DeltaRemoveInsertModificationPolicy {
			static constexpr auto Is_Mutable = false;
			static constexpr auto Is_Strictly_Ordered = false;
		};
	}

	DEFINE_CACHE_CONTAINS_TESTS(HashCacheMixinTraits, ViewAccessor, _View)
	DEFINE_CACHE_CONTAINS_TESTS(HashCacheMixinTraits, DeltaAccessor, _Delta)

	DEFINE_CACHE_ITERATION_TESTS_ORDERING(HashCacheMixinTraits, ViewAccessor, Ordered, _View)

	DEFINE_CACHE_MUTATION_TESTS(HashCacheMixinTraits, DeltaAccessor, _Delta)

	DEFINE_DELTA_ELEMENTS_MIXIN_CUSTOM_TESTS(HashCacheMixinTraits, HashCacheDeltaModificationPolicy, _Delta)

	DEFINE_CACHE_BASIC_TESTS(HashCacheMixinTraits,)

	// endregion

	// *** custom tests ***

	// region ctor

	TEST(TEST_CLASS, CanCreateHashCacheWithCustomRetentionTime) {
		// Act:
		HashCache cache(CacheConfiguration(), utils::TimeSpan::FromHours(123));

		// Assert:
		EXPECT_EQ(utils::TimeSpan::FromHours(123), cache.createView()->retentionTime());
		EXPECT_EQ(utils::TimeSpan::FromHours(123), cache.createDelta()->retentionTime());
		EXPECT_EQ(utils::TimeSpan::FromHours(123), cache.createDetachedDelta().tryLock()->retentionTime());
	}

	// endregion

	// region prune

	TEST(TEST_CLASS, PruningBoundaryIsInitiallyUnset) {
		// Arrange:
		HashCache cache(CacheConfiguration(), utils::TimeSpan::FromHours(123));
		auto delta = cache.createDelta();

		// Assert:
		EXPECT_FALSE(delta->pruningBoundary().isSet());
	}

	TEST(TEST_CLASS, PruneUpdatesPruningBoundary) {
		// Arrange:
		HashCache cache(CacheConfiguration(), utils::TimeSpan::FromHours(32));
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
