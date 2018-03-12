#pragma once
#include "MongoCacheStorageTestUtils.h"

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
			auto originalElement = TTraits::GenerateRandomElement(11);
			TTraits::Add(delta, originalElement);
			storage.get().saveDelta(delta);
			cache.commit(Height());

			// Sanity:
			EXPECT_EQ(1u, GetCollectionSize());
			AssertDbContents({ originalElement });

			// Act:
			auto newElement = TTraits::GenerateRandomElement(54321);
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
			auto element = TTraits::GenerateRandomElement(11);
			TTraits::Add(delta, element);
			storage.get().saveDelta(delta);
			cache.commit(Height());

			// Sanity:
			EXPECT_EQ(1u, GetCollectionSize());
			AssertDbContents({ element });

			// Act:
			TTraits::Mutate(delta, element);
			storage.get().saveDelta(delta);

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

		static void AssertCanSaveMultipleElements() {
			// Arrange:
			CacheStorageWrapper storage;
			auto cache = TTraits::CreateCache();
			auto delta = cache.createDelta();

			// Act:
			std::vector<ElementType> elements;
			for (auto i = 0u; i < 100u; ++i) {
				elements.push_back(TTraits::GenerateRandomElement(i));
				TTraits::Add(delta, elements.back());
			}

			storage.get().saveDelta(delta);
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
			for (auto i = 0u; i < 100u; ++i) {
				elements.push_back(TTraits::GenerateRandomElement(i));
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
					TTraits::Mutate(delta, element);
					++numModified;
#ifndef _MSC_VER
					[[clang::fallthrough]];
#endif

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

		static void AssertCanLoadFromNonEmptyDatabase() {
			// Arrange:
			CacheStorageWrapper storage;

			// - seed the database with 100 elements
			{
				auto cache1 = TTraits::CreateCache();
				auto delta1 = cache1.createDelta();
				for (auto i = 0u; i < 100u; ++i)
					TTraits::Add(delta1, TTraits::GenerateRandomElement(i));

				storage.get().saveDelta(delta1);
			}

			// Sanity:
			EXPECT_EQ(100u, GetCollectionSize());

			// Act: load into a second cache
			auto cache2 = TTraits::CreateCache();
			storage.get().loadAll(cache2, Height(1));
			auto elements = GetCacheContents(cache2);

			// Assert:
			EXPECT_EQ(100u, elements.size());
			AssertDbContents(elements);
		}

	private:
		static auto GetCacheContents(const cache::CatapultCache& cache) {
			std::vector<ElementType> contents;
			const auto& subCache = cache.sub<CacheType>();
			auto view = subCache.createView();
			for (const auto& pair : *view)
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
	\
	MAKE_FLAT_CACHE_STORAGE_TEST(TRAITS_NAME, POSTFIX, CanLoadFromEmptyDatabase) \
	MAKE_FLAT_CACHE_STORAGE_TEST(TRAITS_NAME, POSTFIX, CanLoadFromNonEmptyDatabase)
}}
