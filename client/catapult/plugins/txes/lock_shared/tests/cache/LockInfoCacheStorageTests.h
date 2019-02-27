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

#pragma once
#include "plugins/txes/lock_shared/tests/test/LockInfoCacheTestUtils.h"
#include "tests/test/cache/CacheStorageTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

	/// Lock info cache storage test suite.
	template<typename TLockInfoTraits>
	class LockInfoCacheStorageTests {
	public:
		static void AssertCanLoadValueIntoCache() {
			// Arrange: create a random value to insert
			auto originalLockInfo = test::CreateLockInfos<TLockInfoTraits>(1)[0];

			// Act:
			typename TLockInfoTraits::CacheType cache(CacheConfiguration{});
			{
				auto delta = cache.createDelta();
				TLockInfoTraits::StorageType::LoadInto(originalLockInfo, *delta);
				cache.commit();
			}

			// Assert: the cache contains the value
			auto view = cache.createView();
			EXPECT_EQ(1u, view->size());

			const auto& key = TLockInfoTraits::ToKey(originalLockInfo);
			ASSERT_TRUE(view->contains(key));
			const auto& loadedLockInfo = view->find(key).get();

			// - the loaded cache value is correct
			TLockInfoTraits::AssertEqual(originalLockInfo, loadedLockInfo);
		}
	};
}}

#define MAKE_LOCK_INFO_CACHE_STORAGE_TEST(TRAITS_NAME, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { LockInfoCacheStorageTests<TRAITS_NAME>::Assert##TEST_NAME(); }

#define DEFINE_LOCK_INFO_CACHE_STORAGE_TESTS(TRAITS_NAME) \
	MAKE_LOCK_INFO_CACHE_STORAGE_TEST(TRAITS_NAME, CanLoadValueIntoCache)
