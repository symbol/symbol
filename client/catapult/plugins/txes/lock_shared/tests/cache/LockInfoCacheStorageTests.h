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

#pragma once
#include "plugins/txes/lock_shared/tests/test/LockInfoCacheTestUtils.h"
#include "tests/test/cache/CacheStorageTestUtils.h"

namespace catapult { namespace cache {

	/// Lock info cache storage test suite.
	template<typename TTraits>
	class LockInfoCacheStorageTests {
	public:
		// region LoadInto

		static void AssertCanLoadValueWithHistoryDepthEqualToOneIntoCache() {
			// Arrange:
			auto lockIdentifier = CreateId(148);
			auto originalHistory = typename TTraits::HistoryType(lockIdentifier);
			originalHistory.push_back(CreateValue(lockIdentifier));

			// Act:
			CacheType cache;
			{
				auto delta = cache.createDelta();
				TTraits::StorageType::LoadInto(originalHistory, *delta);
				cache.commit();
			}

			// Assert: the cache contains the value
			auto view = cache.createView();
			EXPECT_EQ(1u, view->size());
			ASSERT_TRUE(view->contains(lockIdentifier));

			// - the loaded cache value is correct
			auto cacheIter = view->find(lockIdentifier);
			EXPECT_EQ(lockIdentifier, cacheIter.get().id());
			ASSERT_EQ(1u, cacheIter.get().historyDepth());

			TTraits::AssertEqual(originalHistory.back(), cacheIter.get().back());
		}

		static void AssertCanLoadValueWithHistoryDepthGreaterThanOneIntoCache() {
			// Arrange:
			auto lockIdentifier = CreateId(148);
			auto originalHistory = typename TTraits::HistoryType(lockIdentifier);
			originalHistory.push_back(CreateValue(lockIdentifier));
			originalHistory.push_back(CreateValue(lockIdentifier));
			originalHistory.push_back(CreateValue(lockIdentifier));

			// Act:
			CacheType cache;
			{
				auto delta = cache.createDelta();
				TTraits::StorageType::LoadInto(originalHistory, *delta);
				cache.commit();
			}

			// Assert: the cache contains the value
			auto view = cache.createView();
			EXPECT_EQ(1u, view->size());
			ASSERT_TRUE(view->contains(lockIdentifier));

			// - the loaded cache value is correct
			auto cacheIter = view->find(lockIdentifier);
			EXPECT_EQ(lockIdentifier, cacheIter.get().id());
			ASSERT_EQ(3u, cacheIter.get().historyDepth());

			auto originalHistoryIter = originalHistory.begin();
			auto historyIter = cacheIter.get().begin();
			for (auto i = 0u; i < originalHistory.historyDepth(); ++i)
				TTraits::AssertEqual(*originalHistoryIter++, *historyIter++);
		}

		// endregion

		// region Purge

		static void AssertCanPurgeExistingValueWithHistoryDepthEqualToOneFromCache() {
			AssertCanPurgeExistingValue(1);
		}

		static void AssertCanPurgeExistingValueWithHistoryDepthGreaterThanOneFromCache() {
			AssertCanPurgeExistingValue(3);
		}

	private:
		static void AssertCanPurgeExistingValue(size_t historyDepth) {
			// Arrange:
			auto lockIdentifier1 = CreateId(148);
			auto originalHistory = typename TTraits::HistoryType(lockIdentifier1);
			for (auto i = 0u; i < historyDepth; ++i)
				originalHistory.push_back(CreateValue(lockIdentifier1));

			auto lockIdentifier2 = CreateId(200);
			auto otherHistory = typename TTraits::HistoryType(lockIdentifier2);
			otherHistory.push_back(CreateValue(lockIdentifier2));

			// - add one value that will not be purged
			CacheType cache;
			{
				auto delta = cache.createDelta();
				TTraits::StorageType::LoadInto(originalHistory, *delta);
				TTraits::StorageType::LoadInto(otherHistory, *delta);
				cache.commit();
			}

			// Sanity:
			EXPECT_TRUE(cache.createView()->contains(lockIdentifier1));
			EXPECT_TRUE(cache.createView()->contains(lockIdentifier2));

			// Act:
			{
				auto delta = cache.createDelta();
				TTraits::StorageType::Purge(originalHistory, *delta);
				cache.commit();
			}

			// Assert:
			EXPECT_FALSE(cache.createView()->contains(lockIdentifier1));
			EXPECT_TRUE(cache.createView()->contains(lockIdentifier2));
		}

	public:
		static void AssertCanPurgeNonexistentValueFromCache() {
			// Arrange:
			auto lockIdentifier1 = CreateId(148);
			auto originalHistory = typename TTraits::HistoryType(lockIdentifier1);
			originalHistory.push_back(CreateValue(lockIdentifier1));

			auto lockIdentifier2 = CreateId(200);
			auto otherHistory = typename TTraits::HistoryType(lockIdentifier2);
			otherHistory.push_back(CreateValue(lockIdentifier2));

			// - add one value that will not be purged
			CacheType cache;
			{
				auto delta = cache.createDelta();
				TTraits::StorageType::LoadInto(otherHistory, *delta);
				cache.commit();
			}

			// Sanity:
			EXPECT_FALSE(cache.createView()->contains(lockIdentifier1));
			EXPECT_TRUE(cache.createView()->contains(lockIdentifier2));

			// Act:
			{
				auto delta = cache.createDelta();
				TTraits::StorageType::Purge(originalHistory, *delta);
				cache.commit();
			}

			// Assert:
			EXPECT_FALSE(cache.createView()->contains(lockIdentifier1));
			EXPECT_TRUE(cache.createView()->contains(lockIdentifier2));
		}

		// endregion

	private:
		class CacheType : public TTraits::CacheType {
		public:
			CacheType() : TTraits::CacheType(CacheConfiguration())
			{}
		};

		static auto CreateId(uint8_t id) {
			return typename TTraits::KeyType{ { id } };
		}

		static auto CreateValue(const typename TTraits::KeyType& key) {
			auto lockInfo = TTraits::CreateLockInfo(Height(1));
			TTraits::SetKey(lockInfo, key);
			return lockInfo;
		}
	};
}}

#define MAKE_LOCK_INFO_CACHE_STORAGE_TEST(TRAITS_NAME, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { LockInfoCacheStorageTests<TRAITS_NAME>::Assert##TEST_NAME(); }

#define DEFINE_LOCK_INFO_CACHE_STORAGE_TESTS(TRAITS_NAME) \
	MAKE_LOCK_INFO_CACHE_STORAGE_TEST(TRAITS_NAME, CanLoadValueWithHistoryDepthEqualToOneIntoCache) \
	MAKE_LOCK_INFO_CACHE_STORAGE_TEST(TRAITS_NAME, CanLoadValueWithHistoryDepthGreaterThanOneIntoCache) \
	MAKE_LOCK_INFO_CACHE_STORAGE_TEST(TRAITS_NAME, CanPurgeExistingValueWithHistoryDepthEqualToOneFromCache) \
	MAKE_LOCK_INFO_CACHE_STORAGE_TEST(TRAITS_NAME, CanPurgeExistingValueWithHistoryDepthGreaterThanOneFromCache) \
	MAKE_LOCK_INFO_CACHE_STORAGE_TEST(TRAITS_NAME, CanPurgeNonexistentValueFromCache)
