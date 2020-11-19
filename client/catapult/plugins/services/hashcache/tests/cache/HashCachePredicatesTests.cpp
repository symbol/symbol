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

#include "src/cache/HashCachePredicates.h"
#include "src/cache/HashCache.h"
#include "tests/test/HashCacheTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS HashCachePredicatesTests

	namespace {
		Hash256 PopulateHashCache(CatapultCache& cache) {
			auto delta = cache.createDelta();
			auto& hashCacheDelta = delta.sub<HashCache>();

			for (auto i = 0u; i < 5; ++i)
				hashCacheDelta.insert(state::TimestampedHash(Timestamp(i), test::GenerateRandomByteArray<Hash256>()));

			auto hash = test::GenerateRandomByteArray<Hash256>();
			hashCacheDelta.insert(state::TimestampedHash(Timestamp(5), hash));

			for (auto i = 6u; i < 10; ++i)
				hashCacheDelta.insert(state::TimestampedHash(Timestamp(i), test::GenerateRandomByteArray<Hash256>()));

			cache.commit(Height());
			return hash;
		}
	}

	// region HashCacheContains

	TEST(TEST_CLASS, HashCacheContains_ReturnsTrueWhenElementIsContainedInHashCache) {
		// Arrange:
		auto cache = test::HashCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		auto hash = PopulateHashCache(cache);

		// Act + Assert:
		EXPECT_TRUE(HashCacheContains(cache, Timestamp(5), hash));
	}

	TEST(TEST_CLASS, HashCacheContains_ReturnsFalseWhenElementIsNotContainedInHashCache) {
		// Arrange:
		auto cache = test::HashCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		auto hash = PopulateHashCache(cache);

		// Act + Assert:
		EXPECT_FALSE(HashCacheContains(cache, Timestamp(6), hash));
		EXPECT_FALSE(HashCacheContains(cache, Timestamp(5), test::GenerateRandomByteArray<Hash256>()));
	}

	// endregion
}}
