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
#include "tests/TestHarness.h"
#include <unordered_set>

namespace catapult { namespace test {

	// region test utils

	namespace detail {
		/// Accessor for extracting a view from a cache.
		struct ViewAccessor {
			template<typename TCache>
			static auto CreateView(const TCache& cache) {
				return cache.createView();
			}
		};

		/// Accessor for extracting a delta from a cache.
		struct DeltaAccessor {
			template<typename TCache>
			static auto CreateView(TCache& cache) {
				return cache.createDelta();
			}
		};

		/// Inserts elements with \a ids into \a cache.
		template<typename TTraits>
		void InsertMultiple(typename TTraits::CacheType& cache, std::initializer_list<uint8_t> ids) {
			auto delta = cache.createDelta();
			for (auto id : ids)
				delta->insert(TTraits::CreateWithId(id));

			cache.commit();
		}
	}

	/// Possible cache ordering modes.
	enum class CacheOrderingMode {
		/// Cache is unordered.
		Unordered,

		/// Cache is ordered but has no insert / remove constraints.
		Ordered,

		/// Cache is strictly ordered and expects inserts and removes of consecutive elements.
		Strictly_Ordered
	};

	// endregion

	// region CacheContainsMixinTests

	/// Test suite for cache contains mixin.
	template<typename TTraits, typename TViewAccessor, CacheOrderingMode Mode>
	class CacheContainsMixinTests {
	private:
		using CacheType = typename TTraits::CacheType;

	private:
		static constexpr uint8_t Id_1 = CacheOrderingMode::Strictly_Ordered == Mode ? 1 : 9;
		static constexpr uint8_t Id_2 = CacheOrderingMode::Strictly_Ordered == Mode ? 2 : 25;
		static constexpr uint8_t Id_3 = CacheOrderingMode::Strictly_Ordered == Mode ? 3 : 16;

	public:
		static void AssertSizeReturnsCacheSizeWhenEmpty() {
			// Arrange:
			CacheType cache;
			auto view = TViewAccessor::CreateView(cache);

			// Act:
			auto size = view->size();

			// Assert:
			EXPECT_EQ(0u, size);
		}

		static void AssertSizeReturnsCacheSizeWhenNotEmpty() {
			// Arrange:
			CacheType cache;
			detail::InsertMultiple<TTraits>(cache, { Id_1, Id_2, Id_3 });
			auto view = TViewAccessor::CreateView(cache);

			// Act:
			auto size = view->size();

			// Assert:
			EXPECT_EQ(3u, size);
		}

		static void AssertContainsReturnsFalseForKeysNotInCache() {
			// Arrange:
			CacheType cache;
			detail::InsertMultiple<TTraits>(cache, { Id_1, Id_2, Id_3 });
			auto view = TViewAccessor::CreateView(cache);

			// Act:
			auto contains1 = view->contains(TTraits::MakeId(Id_3 * 2));
			auto contains2 = view->contains(TTraits::MakeId(0));

			// Assert:
			EXPECT_FALSE(contains1);
			EXPECT_FALSE(contains2);
		}

		static void AssertContainsReturnsTrueForKeysInCache() {
			// Arrange:
			CacheType cache;
			detail::InsertMultiple<TTraits>(cache, { Id_1, Id_2, Id_3 });
			auto view = TViewAccessor::CreateView(cache);

			// Act:
			auto contains1 = view->contains(TTraits::MakeId(Id_1));
			auto contains2 = view->contains(TTraits::MakeId(Id_2));

			// Assert:
			EXPECT_TRUE(contains1);
			EXPECT_TRUE(contains2);
		}
	};

#define MAKE_CACHE_CONTAINS_TEST(CACHE_TRAITS, VIEW_TRAITS, ORDERING, SUFFIX, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME##SUFFIX) { \
		test::CacheContainsMixinTests<CACHE_TRAITS, test::detail::VIEW_TRAITS, test::CacheOrderingMode::ORDERING>::Assert##TEST_NAME(); \
	}

#define DEFINE_CACHE_CONTAINS_TESTS_ORDERING(CACHE_TRAITS, VIEW_TRAITS, ORDERING, SUFFIX) \
	MAKE_CACHE_CONTAINS_TEST(CACHE_TRAITS, VIEW_TRAITS, ORDERING, SUFFIX, SizeReturnsCacheSizeWhenEmpty) \
	MAKE_CACHE_CONTAINS_TEST(CACHE_TRAITS, VIEW_TRAITS, ORDERING, SUFFIX, SizeReturnsCacheSizeWhenNotEmpty) \
	MAKE_CACHE_CONTAINS_TEST(CACHE_TRAITS, VIEW_TRAITS, ORDERING, SUFFIX, ContainsReturnsFalseForKeysNotInCache) \
	MAKE_CACHE_CONTAINS_TEST(CACHE_TRAITS, VIEW_TRAITS, ORDERING, SUFFIX, ContainsReturnsTrueForKeysInCache)

#define DEFINE_CACHE_CONTAINS_TESTS(CACHE_TRAITS, VIEW_TRAITS, SUFFIX) \
	DEFINE_CACHE_CONTAINS_TESTS_ORDERING(CACHE_TRAITS, VIEW_TRAITS, Unordered, SUFFIX)

	// endregion

	// region CacheIterationMixinTests

	/// Test suite for cache iteration mixin.
	template<typename TTraits, typename TViewAccessor, CacheOrderingMode Mode>
	class CacheIterationMixinTests {
	private:
		using CacheType = typename TTraits::CacheType;
		using IdType = typename TTraits::IdType;
		using ValueType = typename TTraits::ValueType;

	private:
		static constexpr uint8_t Id_1 = CacheOrderingMode::Strictly_Ordered == Mode ? 1 : 9;
		static constexpr uint8_t Id_2 = CacheOrderingMode::Strictly_Ordered == Mode ? 2 : 25;
		static constexpr uint8_t Id_3 = CacheOrderingMode::Strictly_Ordered == Mode ? 3 : 16;
		static constexpr uint8_t Id_4 = CacheOrderingMode::Strictly_Ordered == Mode ? 4 : 49;
		static constexpr uint8_t Id_5 = CacheOrderingMode::Strictly_Ordered == Mode ? 5 : 36;

	public:
		static void AssertCanIterateZeroElementsUsingIterators() {
			// Assert:
			AssertIterationWithIterators({});
		}

		static void AssertCanIterateSingleElementUsingIterators() {
			// Assert:
			AssertIterationWithIterators({ Id_1 });
		}

		static void AssertCanIterateMultipleElementsUsingIterators() {
			// Assert:
			AssertIterationWithIterators({ Id_1, Id_2, Id_3, Id_4, Id_5 });
		}

	private:
		// needs to be a vector so AssertIteration can properly check sorting
		using IdPairsContainer = std::vector<std::pair<uint8_t, uint8_t>>;

		template<typename TCacheKeysAccessor>
		static void AssertIteration(std::initializer_list<uint8_t> ids, TCacheKeysAccessor cacheKeysAccessor) {
			// Arrange: seed the cache
			CacheType cache;
			detail::InsertMultiple<TTraits>(cache, ids);
			auto view = TViewAccessor::CreateView(cache);

			// Sanity:
			CATAPULT_LOG(debug) << "cache is seeded with " << view->size() << " elements";
			EXPECT_EQ(ids.size(), view->size());

			// Act: iterate over all values and extract the ids
			auto idPairs = cacheKeysAccessor(*view);

			// Assert:
			// - if appropriate, check sorting before modifying idPairs
			if (ShouldCheckSorting()) {
				uint8_t lastId = 0;
				for (const auto& pair : idPairs) {
					EXPECT_LT(lastId, pair.first) << "cache is expected to be sorted";
					lastId = pair.first;
				}
			}

			// - check contents
			ASSERT_EQ(ids.size(), idPairs.size());
			for (const auto& id : ids) {
				auto idPairsIter = std::find_if(idPairs.cbegin(), idPairs.cend(), [id](const auto& pair) {
					return id == pair.first;
				});

				auto message = "id: " + std::to_string(static_cast<uint16_t>(id));
				ASSERT_TRUE(idPairs.cend() != idPairsIter) << message;
				EXPECT_EQ(id, idPairsIter->first) << message;
				EXPECT_EQ(id, idPairsIter->second) << message;

				idPairs.erase(idPairsIter);
			}

			// Sanity:
			EXPECT_TRUE(idPairs.empty());
		}

		static void AssertIterationWithIterators(std::initializer_list<uint8_t> ids) {
			AssertIteration(ids, [](const auto& view) {
				// Act: iterate over all values and extract the id pairs (to check that keys and values are matched)
				//      in order to unify map and set processing, pairs are extracted from both
				IdPairsContainer idPairs;
				auto pIterableView = view.tryMakeIterableView();
				for (const auto& valueOrPair : *pIterableView)
					idPairs.push_back(GetIdPair(valueOrPair));

				return idPairs;
			});
		}

	private:
		static constexpr bool ShouldCheckSorting() {
			return CacheOrderingMode::Unordered != Mode;
		}

		static std::pair<uint8_t, uint8_t> GetIdPair(const std::pair<IdType, ValueType>& pair) {
			return std::make_pair(TTraits::GetRawId(pair.first), TTraits::GetRawId(TTraits::GetId(pair.second)));
		}

		static std::pair<uint8_t, uint8_t> GetIdPair(const ValueType& value) {
			auto id = TTraits::GetRawId(TTraits::GetId(value));
			return std::make_pair(id, id);
		}
	};

#define MAKE_CACHE_ITERATION_TEST(CACHE_TRAITS, VIEW_TRAITS, ORDERING, SUFFIX, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME##SUFFIX) { \
		test::CacheIterationMixinTests<CACHE_TRAITS, test::detail::VIEW_TRAITS, test::CacheOrderingMode::ORDERING>::Assert##TEST_NAME(); \
	}

#define DEFINE_CACHE_ITERATION_TESTS_ORDERING(CACHE_TRAITS, VIEW_TRAITS, ORDERING, SUFFIX) \
	MAKE_CACHE_ITERATION_TEST(CACHE_TRAITS, VIEW_TRAITS, ORDERING, SUFFIX, CanIterateZeroElementsUsingIterators) \
	MAKE_CACHE_ITERATION_TEST(CACHE_TRAITS, VIEW_TRAITS, ORDERING, SUFFIX, CanIterateSingleElementUsingIterators) \
	MAKE_CACHE_ITERATION_TEST(CACHE_TRAITS, VIEW_TRAITS, ORDERING, SUFFIX, CanIterateMultipleElementsUsingIterators)

#define DEFINE_CACHE_ITERATION_TESTS(CACHE_TRAITS, VIEW_TRAITS, SUFFIX) \
	DEFINE_CACHE_ITERATION_TESTS_ORDERING(CACHE_TRAITS, VIEW_TRAITS, Unordered, SUFFIX)

	// endregion

	// region CacheAccessorMixinTests

	namespace detail {
		/// Accessor for const-qualifying an object.
		struct ConstAccessor {
			template<typename T>
			static const auto& Get(const T& object) {
				return object;
			}
		};

		/// Accessor for not changing the constness of an object.
		struct MutableAccessor {
			template<typename T>
			static auto& Get(T& object) {
				return object;
			}
		};
	}

	/// Test suite for cache accessor mixins.
	template<typename TTraits, typename TViewAccessor, typename TAccessor>
	class CacheAccessorMixinTests {
	private:
		using CacheType = typename TTraits::CacheType;

	public:
		static void AssertGetThrowsForKeysNotInCache() {
			// Arrange:
			CacheType cache;
			detail::InsertMultiple<TTraits>(cache, { 9, 25, 16 });
			auto view = TViewAccessor::CreateView(cache);

			// Act + Assert:
			EXPECT_THROW(TAccessor::Get(*view).find(TTraits::MakeId(0)).get(), catapult_invalid_argument);
			EXPECT_THROW(TAccessor::Get(*view).find(TTraits::MakeId(36)).get(), catapult_invalid_argument);
		}

		static void AssertGetReturnsValuesForKeysInCache() {
			// Arrange:
			CacheType cache;
			detail::InsertMultiple<TTraits>(cache, { 9, 25, 16 });
			auto view = TViewAccessor::CreateView(cache);

			// Act:
			const auto& value1 = TAccessor::Get(*view).find(TTraits::MakeId(9)).get();
			const auto& value2 = TAccessor::Get(*view).find(TTraits::MakeId(25)).get();

			// Act + Assert:
			EXPECT_EQ(TTraits::MakeId(9), TTraits::GetId(value1));
			EXPECT_EQ(TTraits::MakeId(25), TTraits::GetId(value2));
		}

		static void AssertTryGetReturnsNullptrForKeysNotInCache() {
			// Arrange:
			CacheType cache;
			detail::InsertMultiple<TTraits>(cache, { 9, 25, 16 });
			auto view = TViewAccessor::CreateView(cache);

			// Act:
			const auto* pValue1 = TAccessor::Get(*view).find(TTraits::MakeId(0)).tryGet();
			const auto* pValue2 = TAccessor::Get(*view).find(TTraits::MakeId(36)).tryGet();

			// Assert:
			EXPECT_FALSE(!!pValue1);
			EXPECT_FALSE(!!pValue2);
		}

		static void AssertTryGetReturnsValuesForKeysInCache() {
			// Arrange:
			CacheType cache;
			detail::InsertMultiple<TTraits>(cache, { 9, 25, 16 });
			auto view = TViewAccessor::CreateView(cache);

			// Act:
			const auto* pValue1 = TAccessor::Get(*view).find(TTraits::MakeId(9)).tryGet();
			const auto* pValue2 = TAccessor::Get(*view).find(TTraits::MakeId(25)).tryGet();

			// Assert:
			ASSERT_TRUE(!!pValue1);
			ASSERT_TRUE(!!pValue2);

			EXPECT_EQ(TTraits::MakeId(9), TTraits::GetId(*pValue1));
			EXPECT_EQ(TTraits::MakeId(25), TTraits::GetId(*pValue2));
		}
	};

#define MAKE_CACHE_ACCESSOR_TEST(CACHE_TRAITS, VIEW_TRAITS, ACCESS_TRAITS, SUFFIX, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME##SUFFIX) { \
		test::CacheAccessorMixinTests<CACHE_TRAITS, test::detail::VIEW_TRAITS, test::detail::ACCESS_TRAITS>::Assert##TEST_NAME(); \
	}

#define DEFINE_CACHE_ACCESSOR_TESTS(CACHE_TRAITS, VIEW_TRAITS, ACCESS_TRAITS, SUFFIX) \
	MAKE_CACHE_ACCESSOR_TEST(CACHE_TRAITS, VIEW_TRAITS, ACCESS_TRAITS, SUFFIX, GetThrowsForKeysNotInCache) \
	MAKE_CACHE_ACCESSOR_TEST(CACHE_TRAITS, VIEW_TRAITS, ACCESS_TRAITS, SUFFIX, GetReturnsValuesForKeysInCache) \
	MAKE_CACHE_ACCESSOR_TEST(CACHE_TRAITS, VIEW_TRAITS, ACCESS_TRAITS, SUFFIX, TryGetReturnsNullptrForKeysNotInCache) \
	MAKE_CACHE_ACCESSOR_TEST(CACHE_TRAITS, VIEW_TRAITS, ACCESS_TRAITS, SUFFIX, TryGetReturnsValuesForKeysInCache)

	// endregion

	// region CacheActivePredicateMixinTests

	template<typename TTraits, typename TViewAccessor>
	class CacheActivePredicateMixinTests {
	private:
		using CacheType = typename TTraits::CacheType;

	public:
		static void AssertIsActiveReturnsTrueWhenValueIsPresentAndActive() {
			// Act:
			RunActivePredicateMixinTest([](const auto& view) {
				// Assert:
				EXPECT_TRUE(view.isActive(TTraits::MakeId(111), Height(1)));
				EXPECT_TRUE(view.isActive(TTraits::MakeId(222), Height(122)));
			});
		}

		static void AssertIsActiveReturnsFalseWhenValueIsPresentAndInactive() {
			// Act:
			RunActivePredicateMixinTest([](const auto& view) {
				// Assert:
				EXPECT_FALSE(view.isActive(TTraits::MakeId(111), Height(999)));
				EXPECT_FALSE(view.isActive(TTraits::MakeId(222), Height(123)));
			});
		}

		static void AssertIsActiveReturnsFalseWhenValueIsNotPresent() {
			// Act:
			RunActivePredicateMixinTest([](const auto& view) {
				// Assert:
				EXPECT_FALSE(view.isActive(TTraits::MakeId(150), Height(1)));
				EXPECT_FALSE(view.isActive(TTraits::MakeId(250), Height(100)));
			});
		}

	private:
		template<typename TAction>
		static void RunActivePredicateMixinTest(TAction action) {
			// Arrange:
			CacheType cache;
			{
				auto delta = cache.createDelta();
				delta->insert(TTraits::CreateWithIdAndExpiration(111, Height(111)));
				delta->insert(TTraits::CreateWithIdAndExpiration(222, Height(123)));
				delta->insert(TTraits::CreateWithIdAndExpiration(233, Height(222)));
				cache.commit();
			}

			// Act:
			action(*TViewAccessor::CreateView(cache));
		}
	};

#define MAKE_ACTIVE_PREDICATE_TEST(CACHE_TRAITS, VIEW_TRAITS, SUFFIX, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME##SUFFIX) { \
		test::CacheActivePredicateMixinTests<CACHE_TRAITS, test::detail::VIEW_TRAITS>::Assert##TEST_NAME(); \
	}

#define DEFINE_ACTIVE_PREDICATE_TESTS(CACHE_TRAITS, VIEW_TRAITS, SUFFIX) \
	MAKE_ACTIVE_PREDICATE_TEST(CACHE_TRAITS, VIEW_TRAITS, SUFFIX, IsActiveReturnsTrueWhenValueIsPresentAndActive) \
	MAKE_ACTIVE_PREDICATE_TEST(CACHE_TRAITS, VIEW_TRAITS, SUFFIX, IsActiveReturnsFalseWhenValueIsPresentAndInactive) \
	MAKE_ACTIVE_PREDICATE_TEST(CACHE_TRAITS, VIEW_TRAITS, SUFFIX, IsActiveReturnsFalseWhenValueIsNotPresent)

	// endregion

	// region CacheBasicInsertRemoveMixinTests

	/// Test suite for basic insert/remove mixin.
	template<typename TTraits, typename TViewAccessor>
	class CacheBasicInsertRemoveMixinTests {
	private:
		using CacheType = typename TTraits::CacheType;

	public:
		static void AssertCanInsertValueIntoCache() {
			// Arrange:
			CacheType cache;
			detail::InsertMultiple<TTraits>(cache, { 9, 25, 16 });
			auto view = TViewAccessor::CreateView(cache);

			// Sanity:
			EXPECT_EQ(3u, view->size());
			EXPECT_FALSE(view->contains(TTraits::MakeId(21)));

			// Act:
			view->insert(TTraits::CreateWithId(21));

			// Assert:
			EXPECT_EQ(4u, view->size());
			EXPECT_TRUE(view->contains(TTraits::MakeId(21)));
		}

		static void AssertCannotInsertExistingValueIntoCache() {
			// Arrange:
			CacheType cache;
			detail::InsertMultiple<TTraits>(cache, { 9, 25, 16 });
			auto view = TViewAccessor::CreateView(cache);

			// Sanity:
			EXPECT_EQ(3u, view->size());
			EXPECT_TRUE(view->contains(TTraits::MakeId(16)));

			// Act + Assert:
			EXPECT_THROW(view->insert(TTraits::CreateWithId(16)), catapult_invalid_argument);

			// Sanity:
			EXPECT_EQ(3u, view->size());
			EXPECT_TRUE(view->contains(TTraits::MakeId(16)));
		}

		static void AssertCanRemoveValueFromCache() {
			// Arrange:
			CacheType cache;
			detail::InsertMultiple<TTraits>(cache, { 9, 25, 16 });
			auto view = TViewAccessor::CreateView(cache);

			// Sanity:
			EXPECT_EQ(3u, view->size());
			EXPECT_TRUE(view->contains(TTraits::MakeId(16)));

			// Act:
			view->remove(TTraits::MakeId(16));

			// Assert:
			EXPECT_EQ(2u, view->size());
			EXPECT_FALSE(view->contains(TTraits::MakeId(16)));
		}

		static void AssertCannotRemoveNonexistentValueFromCache() {
			// Arrange:
			CacheType cache;
			detail::InsertMultiple<TTraits>(cache, { 9, 25, 16 });
			auto view = TViewAccessor::CreateView(cache);

			// Sanity:
			EXPECT_EQ(3u, view->size());
			EXPECT_FALSE(view->contains(TTraits::MakeId(21)));

			// Act + Assert:
			EXPECT_THROW(view->remove(TTraits::MakeId(21)), catapult_invalid_argument);

			// Sanity:
			EXPECT_EQ(3u, view->size());
			EXPECT_FALSE(view->contains(TTraits::MakeId(21)));
		}
	};

#define MAKE_CACHE_MUTATION_TEST(CACHE_TRAITS, VIEW_TRAITS, SUFFIX, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME##SUFFIX) { \
		test::CacheBasicInsertRemoveMixinTests<CACHE_TRAITS, test::detail::VIEW_TRAITS>::Assert##TEST_NAME(); \
	}

#define DEFINE_CACHE_MUTATION_TESTS_SKIP_STRICT_INSERT(CACHE_TRAITS, VIEW_TRAITS, SUFFIX) \
	MAKE_CACHE_MUTATION_TEST(CACHE_TRAITS, VIEW_TRAITS, SUFFIX, CanInsertValueIntoCache) \
	MAKE_CACHE_MUTATION_TEST(CACHE_TRAITS, VIEW_TRAITS, SUFFIX, CanRemoveValueFromCache) \
	MAKE_CACHE_MUTATION_TEST(CACHE_TRAITS, VIEW_TRAITS, SUFFIX, CannotRemoveNonexistentValueFromCache)

#define DEFINE_CACHE_MUTATION_TESTS(CACHE_TRAITS, VIEW_TRAITS, SUFFIX) \
	MAKE_CACHE_MUTATION_TEST(CACHE_TRAITS, VIEW_TRAITS, SUFFIX, CanInsertValueIntoCache) \
	MAKE_CACHE_MUTATION_TEST(CACHE_TRAITS, VIEW_TRAITS, SUFFIX, CannotInsertExistingValueIntoCache) \
	MAKE_CACHE_MUTATION_TEST(CACHE_TRAITS, VIEW_TRAITS, SUFFIX, CanRemoveValueFromCache) \
	MAKE_CACHE_MUTATION_TEST(CACHE_TRAITS, VIEW_TRAITS, SUFFIX, CannotRemoveNonexistentValueFromCache)

	// endregion

	// region CacheHeightBasedTouchMixinTests

	/// Test suite for height based touch mixin.
	template<typename TTraits>
	class CacheHeightBasedTouchMixinTests {
	private:
		using CacheType = typename TTraits::CacheType;

	public:
		static void AssertNothingIsTouchedWhenNoValuesAtHeight() {
			// Arrange:
			CacheType cache;
			SeedCache(cache);
			auto delta = cache.createDelta();

			// Act: touch at a height without any identifiers
			auto expiryIds = delta->touch(Height(150));

			// Assert: no touched elements
			EXPECT_TRUE(delta->modifiedElements().empty());
			EXPECT_EQ(std::unordered_set<uint8_t>(), GetModifiedIds(*delta));

			EXPECT_TRUE(expiryIds.empty());
		}

		static void AssertAllValuesAtHeightAreTouched() {
			// Arrange:
			CacheType cache;
			SeedCache(cache);
			auto delta = cache.createDelta();

			// Act: touch at a height with known identifiers
			auto expiryIds = delta->touch(Height(200));

			// Assert: two touched elements
			EXPECT_EQ(2u, delta->modifiedElements().size());
			EXPECT_EQ(std::unordered_set<uint8_t>({ 22, 44 }), GetModifiedIds(*delta));

			EXPECT_EQ(2u, expiryIds.size());
			EXPECT_CONTAINS(expiryIds, TTraits::MakeId(22));
			EXPECT_CONTAINS(expiryIds, TTraits::MakeId(44));
		}

	private:
		static void SeedCache(CacheType& cache) {
			auto delta = cache.createDelta();
			delta->insert(TTraits::CreateWithIdAndExpiration(11, Height(100)));
			delta->insert(TTraits::CreateWithIdAndExpiration(22, Height(200)));
			delta->insert(TTraits::CreateWithIdAndExpiration(33, Height(100)));
			delta->insert(TTraits::CreateWithIdAndExpiration(44, Height(200)));
			delta->insert(TTraits::CreateWithIdAndExpiration(55, Height(400)));
			cache.commit();
		}

		static std::unordered_set<uint8_t> GetModifiedIds(const typename CacheType::CacheDeltaType& delta) {
			std::unordered_set<uint8_t> modifiedIds;
			for (const auto* pElement : delta.modifiedElements())
				modifiedIds.insert(TTraits::GetRawId(TTraits::GetId(*pElement)));

			return modifiedIds;
		}
	};

#define MAKE_CACHE_TOUCH_TEST(CACHE_TRAITS, SUFFIX, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME##SUFFIX) { \
		test::CacheHeightBasedTouchMixinTests<CACHE_TRAITS>::Assert##TEST_NAME(); \
	}

#define DEFINE_CACHE_TOUCH_TESTS(CACHE_TRAITS, SUFFIX) \
	MAKE_CACHE_TOUCH_TEST(CACHE_TRAITS, SUFFIX, NothingIsTouchedWhenNoValuesAtHeight) \
	MAKE_CACHE_TOUCH_TEST(CACHE_TRAITS, SUFFIX, AllValuesAtHeightAreTouched) \

	// endregion

	// HeightBasedPruningMixin is tested via pruning tests in DEFINE_CACHE_PRUNE_TESTS (CachePruneTests.h)
}}
