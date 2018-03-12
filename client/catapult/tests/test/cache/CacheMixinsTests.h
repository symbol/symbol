#pragma once
#include "tests/TestHarness.h"

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
		/// The cache is unordered.
		Unordered,
		/// The cache is ordered but has no insert / remove constraints.
		Ordered,
		/// The cache is strictly ordered and expects inserts and removes of consecutive elements.
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

		static void AssertCanIterateZeroElementsUsingForEach() {
			// Assert:
			AssertIterationWithForEach({});
		}

		static void AssertCanIterateSingleElementUsingForEach() {
			// Assert:
			AssertIterationWithForEach({ Id_1 });
		}

		static void AssertCanIterateMultipleElementsUsingForEach() {
			// Assert:
			AssertIterationWithForEach({ Id_1, Id_2, Id_3, Id_4, Id_5 });
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
				for (const auto& valueOrPair : view)
					idPairs.push_back(GetIdPair(valueOrPair));

				return idPairs;
			});
		}

		static void AssertIterationWithForEach(std::initializer_list<uint8_t> ids) {
			AssertIteration(ids, [](const auto& view) {
				// Act: iterate over all values and extract the id pairs (to check that keys and values are matched)
				//      in order to unify map and set processing, pairs are extracted from both
				IdPairsContainer idPairs;
				CollectForEach(view, idPairs, ContainerPolicy<IdType, ValueType>());
				return idPairs;
			});
		}

	private:
		// assume a set if IdType and ValueType are the same
		enum class UnderlyingContainerType { Set, Map };
		using SetContainerType = std::integral_constant<UnderlyingContainerType, UnderlyingContainerType::Set>;
		using MapContainerType = std::integral_constant<UnderlyingContainerType, UnderlyingContainerType::Map>;

		template<typename TKey, typename TValue, typename = void>
		struct ContainerPolicy
				: MapContainerType
		{};

		template<typename TKey, typename TValue>
		struct ContainerPolicy<TKey, TValue, typename std::enable_if<std::is_same<TKey, TValue>::value>::type>
				: SetContainerType
		{};

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

		template<typename TView>
		static void CollectForEach(const TView& view, IdPairsContainer& idPairs, MapContainerType) {
			view.forEach([&idPairs](const auto& key, const auto& value) {
				idPairs.push_back(GetIdPair({ key, value }));
			});
		}

		template<typename TView>
		static void CollectForEach(const TView& view, IdPairsContainer& idPairs, SetContainerType) {
			view.forEach([&idPairs](const auto& value) {
				idPairs.push_back(GetIdPair(value));
			});
		}
	};

#define MAKE_CACHE_ITERATION_TEST(CACHE_TRAITS, VIEW_TRAITS, ORDERING, SUFFIX, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME##SUFFIX) { \
		test::CacheIterationMixinTests<CACHE_TRAITS, test::detail::VIEW_TRAITS, test::CacheOrderingMode::ORDERING>::Assert##TEST_NAME(); \
	}

#define DEFINE_CACHE_ITERATION_TESTS_ORDERING(CACHE_TRAITS, VIEW_TRAITS, ORDERING, SUFFIX) \
	MAKE_CACHE_ITERATION_TEST(CACHE_TRAITS, VIEW_TRAITS, ORDERING, SUFFIX, CanIterateZeroElementsUsingIterators) \
	MAKE_CACHE_ITERATION_TEST(CACHE_TRAITS, VIEW_TRAITS, ORDERING, SUFFIX, CanIterateSingleElementUsingIterators) \
	MAKE_CACHE_ITERATION_TEST(CACHE_TRAITS, VIEW_TRAITS, ORDERING, SUFFIX, CanIterateMultipleElementsUsingIterators) \
	MAKE_CACHE_ITERATION_TEST(CACHE_TRAITS, VIEW_TRAITS, ORDERING, SUFFIX, CanIterateZeroElementsUsingForEach) \
	MAKE_CACHE_ITERATION_TEST(CACHE_TRAITS, VIEW_TRAITS, ORDERING, SUFFIX, CanIterateSingleElementUsingForEach) \
	MAKE_CACHE_ITERATION_TEST(CACHE_TRAITS, VIEW_TRAITS, ORDERING, SUFFIX, CanIterateMultipleElementsUsingForEach)

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

	// Test suite for cache accessor mixins.
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
			EXPECT_THROW(TAccessor::Get(*view).get(TTraits::MakeId(0)), catapult_invalid_argument);
			EXPECT_THROW(TAccessor::Get(*view).get(TTraits::MakeId(36)), catapult_invalid_argument);
		}

		static void AssertGetReturnsValuesForKeysInCache() {
			// Arrange:
			CacheType cache;
			detail::InsertMultiple<TTraits>(cache, { 9, 25, 16 });
			auto view = TViewAccessor::CreateView(cache);

			// Act:
			const auto& value1 = TAccessor::Get(*view).get(TTraits::MakeId(9));
			const auto& value2 = TAccessor::Get(*view).get(TTraits::MakeId(25));

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
			const auto* pValue1 = TAccessor::Get(*view).tryGet(TTraits::MakeId(0));
			const auto* pValue2 = TAccessor::Get(*view).tryGet(TTraits::MakeId(36));

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
			const auto* pValue1 = TAccessor::Get(*view).tryGet(TTraits::MakeId(9));
			const auto* pValue2 = TAccessor::Get(*view).tryGet(TTraits::MakeId(25));

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

	// Test suite for basic insert/remove mixin.
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

		static void AssertCannotRemoveNonExistentValueFromCache() {
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

#define DEFINE_CACHE_MUTATION_TESTS(CACHE_TRAITS, VIEW_TRAITS, SUFFIX) \
	MAKE_CACHE_MUTATION_TEST(CACHE_TRAITS, VIEW_TRAITS, SUFFIX, CanInsertValueIntoCache) \
	MAKE_CACHE_MUTATION_TEST(CACHE_TRAITS, VIEW_TRAITS, SUFFIX, CannotInsertExistingValueIntoCache) \
	MAKE_CACHE_MUTATION_TEST(CACHE_TRAITS, VIEW_TRAITS, SUFFIX, CanRemoveValueFromCache) \
	MAKE_CACHE_MUTATION_TEST(CACHE_TRAITS, VIEW_TRAITS, SUFFIX, CannotRemoveNonExistentValueFromCache)

	// endregion
}}
