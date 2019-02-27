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
#include "MongoCacheStorageTestUtils.h"

namespace catapult { namespace test {

	/// Mongo historical cache storage test suite.
	template<typename TTraits>
	class MongoHistoricalCacheStorageTests : private MongoCacheStorageTestUtils<TTraits> {
	private:
		using CacheType = typename TTraits::CacheType;
		using ElementType = typename TTraits::ModelType;

		using BaseType = MongoCacheStorageTestUtils<TTraits>;
		using BaseType::GetCollectionSize;
		using BaseType::GetDelta;
		using BaseType::AssertDbContents;
		using CacheStorageWrapper = typename BaseType::CacheStorageWrapper;

	public:
		static void AssertSaveHasNoEffectWhenThereAreNoPendingChanges() {
			// Arrange:
			CacheStorageWrapper storage;
			auto cache = TTraits::CreateCache();
			auto delta = cache.createDelta();
			cache.commit(Height());

			// Act:
			storage.get().saveDelta(delta);

			// Assert:
			EXPECT_EQ(0u, GetCollectionSize());
		}

		static void AssertAddedElementIsSavedToStorage() {
			// Arrange:
			CacheStorageWrapper storage;
			auto cache = TTraits::CreateCache();
			auto delta = cache.createDelta();

			// - prepare the cache with a single element
			auto originalElement = TTraits::GenerateRandomElement(11, 0, true);
			TTraits::Add(delta, originalElement);
			storage.get().saveDelta(delta);
			cache.commit(Height());

			// Sanity:
			EXPECT_EQ(1u, GetCollectionSize());
			AssertDbContents({ originalElement });

			// Act:
			auto newElement = TTraits::GenerateRandomElement(54321, 0, true);
			TTraits::Add(delta, newElement);
			storage.get().saveDelta(delta);

			// Sanity:
			EXPECT_EQ(1u, GetDelta(delta).addedElements().size());

			// Assert:
			EXPECT_EQ(2u, GetCollectionSize());
			AssertDbContents({ originalElement, newElement });
		}

		static void AssertModifiedElementIsSavedToStorage() {
			// Arrange:
			CacheStorageWrapper storage;
			auto cache = TTraits::CreateCache();
			auto delta = cache.createDelta();

			// - prepare the cache with a single element
			auto element = TTraits::GenerateRandomElement(11, 0, true);
			TTraits::Add(delta, element);
			storage.get().saveDelta(delta);
			cache.commit(Height());

			// Sanity:
			EXPECT_EQ(1u, GetCollectionSize());
			AssertDbContents({ element });

			// Act:
			auto newElement = TTraits::Mutate(delta, element);
			storage.get().saveDelta(delta);

			// Sanity:
			EXPECT_EQ(1u, GetDelta(delta).modifiedElements().size());

			// Assert: both historical elements are stored
			EXPECT_EQ(2u, GetCollectionSize());
			AssertDbContents({ element, newElement });
		}

		static void AssertDeletedElementIsPartiallyRemovedFromStorage() {
			// Arrange:
			CacheStorageWrapper storage;
			auto cache = TTraits::CreateCache();
			auto delta = cache.createDelta();

			// - prepare the cache with two elements
			auto element1 = TTraits::GenerateRandomElement(11, 0, false);
			auto element2 = TTraits::GenerateRandomElement(11, 1, true);
			TTraits::Add(delta, element1);
			TTraits::Add(delta, element2);
			storage.get().saveDelta(delta);
			cache.commit(Height());

			// Sanity:
			EXPECT_EQ(2u, GetCollectionSize());
			AssertDbContents({ element1, element2 });

			// Act:
			TTraits::Remove(delta, element2);
			storage.get().saveDelta(delta);

			// Sanity:
			EXPECT_EQ(1u, GetDelta(delta).modifiedElements().size());

			// Assert:
			EXPECT_EQ(1u, GetCollectionSize());
			AssertDbContents({ element1 });
		}

		static void AssertDeletedElementIsCompletelyRemovedFromStorage() {
			// Arrange:
			CacheStorageWrapper storage;
			auto cache = TTraits::CreateCache();
			auto delta = cache.createDelta();

			// - prepare the cache with two elements
			auto element1 = TTraits::GenerateRandomElement(11, 0, true);
			auto element2 = TTraits::GenerateRandomElement(12, 0, true);
			TTraits::Add(delta, element1);
			TTraits::Add(delta, element2);
			storage.get().saveDelta(delta);
			cache.commit(Height());

			// Sanity:
			EXPECT_EQ(2u, GetCollectionSize());
			AssertDbContents({ element1, element2 });

			// Act:
			TTraits::Remove(delta, element2);
			storage.get().saveDelta(delta);

			// Sanity:
			EXPECT_EQ(1u, GetDelta(delta).removedElements().size());

			// Assert:
			EXPECT_EQ(1u, GetCollectionSize());
			AssertDbContents({ element1 });
		}

		static void AssertCanSaveMultipleElementsWithDistinctHistory() {
			// Arrange:
			CacheStorageWrapper storage;
			auto cache = TTraits::CreateCache();
			auto delta = cache.createDelta();

			// Act:
			std::vector<ElementType> elements;
			for (auto i = 0u; i < 100; ++i) {
				elements.push_back(TTraits::GenerateRandomElement(i, 0, true));
				TTraits::Add(delta, elements.back());
			}

			storage.get().saveDelta(delta);
			cache.commit(Height());

			// Assert:
			EXPECT_EQ(100u, GetCollectionSize());
			AssertDbContents(elements);
		}

		static void AssertCanSaveMultipleElementsWithSharedHistory() {
			// Arrange:
			CacheStorageWrapper storage;
			auto cache = TTraits::CreateCache();
			auto delta = cache.createDelta();

			// Act: notice that indexes are 0-based
			std::vector<ElementType> elements;
			for (auto i = 0u; i < 100; ++i) {
				elements.push_back(TTraits::GenerateRandomElement(123, i, false));
				TTraits::Add(delta, elements.back());
			}

			// - add active element
			elements.push_back(TTraits::GenerateRandomElement(123, 100, true));
			TTraits::Add(delta, elements.back());

			storage.get().saveDelta(delta);
			cache.commit(Height());

			// Assert:
			EXPECT_EQ(101u, GetCollectionSize());
			AssertDbContents(elements);
		}

		static void AssertCanAddAndModifyAndDeleteMultipleElements() {
			// Arrange:
			CacheStorageWrapper storage;
			auto cache = TTraits::CreateCache();
			auto delta = cache.createDelta();

			// - seed 100 elements
			std::vector<ElementType> elements;
			for (auto i = 0u; i < 100; ++i) {
				elements.push_back(TTraits::GenerateRandomElement(i, 0, true));
				TTraits::Add(delta, elements.back());
			}

			storage.get().saveDelta(delta);
			cache.commit(Height());

			// Act: drop some and modify some
			std::vector<ElementType> expected;
			size_t numModified = 0;
			size_t numRemoved = 0;
			enum class Action : uint8_t { Remove, Modify, Add };
			for (auto& element : elements) {
				switch (static_cast<Action>(RandomByte() % 3)) {
				case Action::Remove:
					TTraits::Remove(delta, element);
					++numRemoved;
					break;

				case Action::Modify:
					expected.push_back(TTraits::Mutate(delta, element)); // both versions should be present
					++numModified;
					[[fallthrough]];

				case Action::Add:
					expected.push_back(element);
					break;
				}
			}

			storage.get().saveDelta(delta);

			// Sanity:
			EXPECT_NE(0u, numRemoved);
			EXPECT_NE(0u, numModified);
			EXPECT_LT(numModified, expected.size());

			// Assert:
			EXPECT_EQ(expected.size(), GetCollectionSize());
			AssertDbContents(expected);
		}

		static void AssertCanLoadFromEmptyDatabase() {
			// Arrange:
			CacheStorageWrapper storage;
			auto cache = TTraits::CreateCache();

			// Act:
			storage.get().loadAll(cache, Height(1));
			auto contents = GetCacheContents(cache);

			// Assert:
			EXPECT_EQ(0u, GetCollectionSize());
		}

		static void AssertCanLoadFromNonEmptyDatabaseWithDistinctHistory() {
			// Act:
			RunCustomLoadTest([](auto& delta) {
				// Arrange: seed the database with 100 elements
				for (auto i = 0u; i < 100; ++i)
					TTraits::Add(delta, TTraits::GenerateRandomElement(i, 0, true));

				return std::make_pair(100u, 100);
			});
		}

		static void AssertCanLoadFromNonEmptyDatabaseWithSharedHistory() {
			// Act:
			RunCustomLoadTest([](auto& delta) {
				// Arrange: seed the database with 100 elements (notice that indexes are 0-based)
				auto key = test::GenerateRandomData<Key_Size>();
				for (auto i = 0u; i < 100; ++i)
					TTraits::Add(delta, TTraits::CreateElement(key, 123, i, false));

				// - add active element
				TTraits::Add(delta, TTraits::CreateElement(key, 123, 100, true));
				return std::make_pair(101u, 101);
			});
		}

	public:
		/// Executes a custom loading test after initializing the database with \a seedCache.
		/// \note This is required for fully testing the branches in the namespace cache loading.
		template<typename TSeedCache>
		static void RunCustomLoadTest(TSeedCache seedCache) {
			// Arrange:
			CacheStorageWrapper storage;

			// - seed the database
			std::pair<size_t, size_t> expectedCollectionSizes; // (cache-size, deep-size)
			{
				auto cache1 = TTraits::CreateCache();
				auto delta1 = cache1.createDelta();
				expectedCollectionSizes = seedCache(delta1);
				storage.get().saveDelta(delta1);
			}

			// Sanity:
			EXPECT_EQ(expectedCollectionSizes.second, GetCollectionSize());

			// Act: load into a second cache
			auto cache2 = TTraits::CreateCache();
			storage.get().loadAll(cache2, Height(1));
			auto elements = GetCacheContents(cache2);

			// Assert:
			EXPECT_EQ(expectedCollectionSizes.first, elements.size());
			AssertDbContents(elements, expectedCollectionSizes.second - expectedCollectionSizes.first);
		}

		/// Executes a custom loading test after initializing the database with \a seedCache by passing the seeded cache to \a assertCache.
		/// \note This is required for deeply interrogating the cache when differences are not detected by element equals operator.
		template<typename TSeedCache, typename TAssertCache>
		static void RunCustomLoadTest(TSeedCache seedCache, TAssertCache assertCache) {
			// Arrange:
			CacheStorageWrapper storage;

			// - seed the database
			{
				auto cache1 = TTraits::CreateCache();
				auto delta1 = cache1.createDelta();
				seedCache(delta1);
				storage.get().saveDelta(delta1);
			}

			// Act: load into a second cache
			auto cache2 = TTraits::CreateCache();
			storage.get().loadAll(cache2, Height(1));

			// Assert:
			assertCache(cache2);
		}

	private:
		static auto GetCacheContents(const cache::CatapultCache& cache) {
			std::vector<ElementType> contents;
			const auto& subCache = cache.sub<CacheType>();
			auto view = subCache.createView();
			auto pIterableView = view->tryMakeIterableView();
			for (const auto& pair : *pIterableView) {
				auto historyIndex = 0u;
				const auto& history = pair.second;
				for (const auto& historicalRecord : history) {
					contents.push_back(TTraits::CreateElement(historicalRecord, historyIndex, historyIndex == history.historyDepth() - 1));
					++historyIndex;
				}
			}

			return contents;
		}
	};

#define MAKE_HISTORICAL_CACHE_STORAGE_TEST(TRAITS_NAME, POSTFIX, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME##POSTFIX) { test::MongoHistoricalCacheStorageTests<TRAITS_NAME>::Assert##TEST_NAME(); }

#define DEFINE_HISTORICAL_CACHE_STORAGE_TESTS(TRAITS_NAME, POSTFIX) \
	MAKE_HISTORICAL_CACHE_STORAGE_TEST(TRAITS_NAME, POSTFIX, SaveHasNoEffectWhenThereAreNoPendingChanges) \
	MAKE_HISTORICAL_CACHE_STORAGE_TEST(TRAITS_NAME, POSTFIX, AddedElementIsSavedToStorage) \
	MAKE_HISTORICAL_CACHE_STORAGE_TEST(TRAITS_NAME, POSTFIX, ModifiedElementIsSavedToStorage) \
	MAKE_HISTORICAL_CACHE_STORAGE_TEST(TRAITS_NAME, POSTFIX, DeletedElementIsPartiallyRemovedFromStorage) \
	MAKE_HISTORICAL_CACHE_STORAGE_TEST(TRAITS_NAME, POSTFIX, DeletedElementIsCompletelyRemovedFromStorage) \
	\
	MAKE_HISTORICAL_CACHE_STORAGE_TEST(TRAITS_NAME, POSTFIX, CanSaveMultipleElementsWithDistinctHistory) \
	MAKE_HISTORICAL_CACHE_STORAGE_TEST(TRAITS_NAME, POSTFIX, CanSaveMultipleElementsWithSharedHistory) \
	MAKE_HISTORICAL_CACHE_STORAGE_TEST(TRAITS_NAME, POSTFIX, CanAddAndModifyAndDeleteMultipleElements) \
	\
	MAKE_HISTORICAL_CACHE_STORAGE_TEST(TRAITS_NAME, POSTFIX, CanLoadFromEmptyDatabase) \
	MAKE_HISTORICAL_CACHE_STORAGE_TEST(TRAITS_NAME, POSTFIX, CanLoadFromNonEmptyDatabaseWithDistinctHistory) \
	MAKE_HISTORICAL_CACHE_STORAGE_TEST(TRAITS_NAME, POSTFIX, CanLoadFromNonEmptyDatabaseWithSharedHistory)
}}
