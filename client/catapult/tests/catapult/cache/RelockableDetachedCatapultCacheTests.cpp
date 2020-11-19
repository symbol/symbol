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

#include "catapult/cache/RelockableDetachedCatapultCache.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS RelockableDetachedCatapultCacheTests

	namespace {
		void SetHeight(CatapultCache& cache, Height height) {
			auto delta = cache.createDelta();
			cache.commit(height);
		}
	}

	TEST(TEST_CLASS, CanCreateRelockableDetachedCatapultCache) {
		// Arrange:
		auto cache = test::CreateEmptyCatapultCache();
		SetHeight(cache, Height(7));

		// Act:
		RelockableDetachedCatapultCache detachedCatapultCache(cache);

		// Assert: the detached cache should have the correct height and be lockable
		EXPECT_EQ(Height(7), detachedCatapultCache.height());
		EXPECT_TRUE(!!detachedCatapultCache.getAndTryLock());
	}

	TEST(TEST_CLASS, RelockableDetachedCatapultCacheIsInvalidatedWhenUnderlyingCacheChanges) {
		// Arrange:
		auto cache = test::CreateEmptyCatapultCache();
		SetHeight(cache, Height(7));

		// - create the detached cache
		RelockableDetachedCatapultCache detachedCatapultCache(cache);

		// Act: invalidate it
		SetHeight(cache, Height(11));

		// Assert: the detached cache should have the original height and not be lockable
		EXPECT_EQ(Height(7), detachedCatapultCache.height());
		EXPECT_FALSE(!!detachedCatapultCache.getAndTryLock());
	}

	TEST(TEST_CLASS, RelockableDetachedCatapultCacheCanBeRebasedOnTopOfUnderlyingCache) {
		// Arrange:
		auto cache = test::CreateEmptyCatapultCache();
		SetHeight(cache, Height(7));

		// - create the detached cache
		RelockableDetachedCatapultCache detachedCatapultCache(cache);

		// - invalidate it
		SetHeight(cache, Height(11));

		// Act: rebase it
		{
			auto pDelta = detachedCatapultCache.rebaseAndLock();

			// Assert: the returned delta should always be a valid pointer
			EXPECT_TRUE(!!pDelta);
		}

		// Assert: the detached cache should have the new height and be lockable
		EXPECT_EQ(Height(11), detachedCatapultCache.height());
		EXPECT_TRUE(!!detachedCatapultCache.getAndTryLock());
	}
}}
