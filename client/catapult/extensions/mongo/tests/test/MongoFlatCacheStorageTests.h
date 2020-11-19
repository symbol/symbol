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
#include "MongoCacheStorageTestUtils.h"
#include "catapult/cache/CatapultCache.h"

namespace catapult { namespace test {

	/// Mongo flat cache storage test suite.
	template<typename TTraits>
	class MongoFlatCacheStorageTests : private MongoCacheStorageTestUtils<TTraits> {
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
			storage.get().saveDelta(cache::CacheChanges(delta));

			// Assert:
			EXPECT_EQ(0u, GetCollectionSize());
		}

		static void AssertAddedElementIsSavedToStorage() {
			// Arrange:
			CacheStorageWrapper storage;
			auto cache = TTraits::CreateCache();
			auto delta = cache.createDelta();

			// - prepare the cache with a single element
			auto originalElement = TTraits::GenerateRandomElement(11);
			TTraits::Add(delta, originalElement);
			storage.get().saveDelta(cache::CacheChanges(delta));
			cache.commit(Height());

			// Sanity:
			EXPECT_EQ(1u, GetCollectionSize());
			AssertDbContents({ originalElement });

			// Act:
			auto newElement = TTraits::GenerateRandomElement(54321);
			TTraits::Add(delta, newElement);
			storage.get().saveDelta(cache::CacheChanges(delta));

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
			auto element = TTraits::GenerateRandomElement(11);
			TTraits::Add(delta, element);
			storage.get().saveDelta(cache::CacheChanges(delta));
			cache.commit(Height());

			// Sanity:
			EXPECT_EQ(1u, GetCollectionSize());
			AssertDbContents({ element });

			// Act:
			TTraits::Mutate(delta, element);
			storage.get().saveDelta(cache::CacheChanges(delta));

			// Sanity:
			EXPECT_EQ(1u, GetDelta(delta).modifiedElements().size());

			// Assert:
			EXPECT_EQ(1u, GetCollectionSize());
			AssertDbContents({ element });
		}

		static void AssertDeletedElementIsRemovedFromStorage() {
			// Arrange:
			CacheStorageWrapper storage;
			auto cache = TTraits::CreateCache();
			auto delta = cache.createDelta();

			// - prepare the cache with two elements
			auto element1 = TTraits::GenerateRandomElement(11);
			auto element2 = TTraits::GenerateRandomElement(12);
			TTraits::Add(delta, element1);
			TTraits::Add(delta, element2);
			storage.get().saveDelta(cache::CacheChanges(delta));
			cache.commit(Height());

			// Sanity:
			EXPECT_EQ(2u, GetCollectionSize());
			AssertDbContents({ element1, element2 });

			// Act:
			TTraits::Remove(delta, element2);
			storage.get().saveDelta(cache::CacheChanges(delta));

			// Sanity:
			EXPECT_EQ(1u, GetDelta(delta).removedElements().size());

			// Assert:
			EXPECT_EQ(1u, GetCollectionSize());
			AssertDbContents({ element1 });
		}

		static void AssertCanSaveMultipleElements() {
			// Arrange:
			CacheStorageWrapper storage;
			auto cache = TTraits::CreateCache();
			auto delta = cache.createDelta();

			// Act:
			std::vector<ElementType> elements;
			for (auto i = 0u; i < 100; ++i) {
				elements.push_back(TTraits::GenerateRandomElement(i));
				TTraits::Add(delta, elements.back());
			}

			storage.get().saveDelta(cache::CacheChanges(delta));
			cache.commit(Height());

			// Assert:
			EXPECT_EQ(100u, GetCollectionSize());
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
				elements.push_back(TTraits::GenerateRandomElement(i));
				TTraits::Add(delta, elements.back());
			}

			storage.get().saveDelta(cache::CacheChanges(delta));
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
					TTraits::Mutate(delta, element);
					++numModified;
					[[fallthrough]];

				case Action::Add:
					expected.push_back(element);
					break;
				}
			}

			storage.get().saveDelta(cache::CacheChanges(delta));

			// Sanity:
			EXPECT_NE(0u, numRemoved);
			EXPECT_NE(0u, numModified);
			EXPECT_LT(numModified, expected.size());

			// Assert:
			EXPECT_EQ(expected.size(), GetCollectionSize());
			AssertDbContents(expected);
		}

		static void AssertElementsBothAddedAndRemovedAreIgnored() {
			// Arrange:
			CacheStorageWrapper storage;
			auto cache = TTraits::CreateCache();
			std::vector<ElementType> elements;

			{
				auto delta = cache.createDelta();

				// - seed a few elements
				for (auto i = 0u; i < 5; ++i) {
					elements.push_back(TTraits::GenerateRandomElement(i));
					TTraits::Add(delta, elements.back());
				}

				storage.get().saveDelta(cache::CacheChanges(delta));
				cache.commit(Height());

				// Sanity:
				AssertDbContents(elements);
			}

			// Act: add and then remove an element
			auto element = TTraits::GenerateRandomElement(10);
			auto delta = cache.createDelta();
			TTraits::Add(delta, element);
			TTraits::Remove(delta, element);
			storage.get().saveDelta(cache::CacheChanges(delta));

			// Assert: the db collection did not change
			AssertDbContents(elements);
		}

	private:
		static auto GetCacheContents(const cache::CatapultCache& cache) {
			std::vector<ElementType> contents;
			const auto& subCache = cache.sub<CacheType>();
			auto view = subCache.createView();
			auto pIterableView = view->tryMakeIterableView();
			for (const auto& pair : *pIterableView)
				contents.push_back(pair.second);

			return contents;
		}
	};

#define MAKE_FLAT_CACHE_STORAGE_TEST(TRAITS_NAME, POSTFIX, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME##POSTFIX) { test::MongoFlatCacheStorageTests<TRAITS_NAME>::Assert##TEST_NAME(); }

#define DEFINE_FLAT_CACHE_STORAGE_TESTS(TRAITS_NAME, POSTFIX) \
	MAKE_FLAT_CACHE_STORAGE_TEST(TRAITS_NAME, POSTFIX, SaveHasNoEffectWhenThereAreNoPendingChanges) \
	MAKE_FLAT_CACHE_STORAGE_TEST(TRAITS_NAME, POSTFIX, AddedElementIsSavedToStorage) \
	MAKE_FLAT_CACHE_STORAGE_TEST(TRAITS_NAME, POSTFIX, ModifiedElementIsSavedToStorage) \
	MAKE_FLAT_CACHE_STORAGE_TEST(TRAITS_NAME, POSTFIX, DeletedElementIsRemovedFromStorage) \
	\
	MAKE_FLAT_CACHE_STORAGE_TEST(TRAITS_NAME, POSTFIX, CanSaveMultipleElements) \
	MAKE_FLAT_CACHE_STORAGE_TEST(TRAITS_NAME, POSTFIX, CanAddAndModifyAndDeleteMultipleElements) \
	MAKE_FLAT_CACHE_STORAGE_TEST(TRAITS_NAME, POSTFIX, ElementsBothAddedAndRemovedAreIgnored)
}}
