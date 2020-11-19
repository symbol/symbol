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

#include "catapult/cache/ReadOnlySimpleCache.h"
#include "tests/test/cache/SimpleCache.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS ReadOnlySimpleCacheTests

	TEST(TEST_CLASS, ReadOnlyViewOnlyContainsCommittedElements) {
		// Arrange:
		test::SimpleCache cache;
		{
			auto cacheDelta = cache.createDelta();
			cacheDelta->increment(); // committed
			cache.commit();
			cacheDelta->increment(); // uncommitted
		}

		// Act:
		auto cacheView = cache.createView();
		test::BasicSimpleCache::CacheReadOnlyType readOnlyCache(*cacheView);

		// Assert:
		EXPECT_EQ(1u, readOnlyCache.size());
		EXPECT_TRUE(readOnlyCache.contains(1));
		EXPECT_FALSE(readOnlyCache.contains(2));
		EXPECT_FALSE(readOnlyCache.contains(3));
	}

	TEST(TEST_CLASS, ReadOnlyDeltaContainsBothCommittedAndUncommittedElements) {
		// Arrange:
		test::SimpleCache cache;
		auto cacheDelta = cache.createDelta();
		cacheDelta->increment(); // committed
		cache.commit();
		cacheDelta->increment(); // uncommitted

		// Act:
		test::BasicSimpleCache::CacheReadOnlyType readOnlyCache(*cacheDelta);

		// Assert:
		EXPECT_EQ(2u, readOnlyCache.size());
		EXPECT_TRUE(readOnlyCache.contains(1));
		EXPECT_TRUE(readOnlyCache.contains(2));
		EXPECT_FALSE(readOnlyCache.contains(3));
	}
}}
