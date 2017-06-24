#pragma once
#include "TraitsStateDecorator.h"
#include "tests/test/nodeps/Conversions.h"

namespace catapult { namespace test {

	// region iteration

	template<typename TTraits, typename TEntityTraits>
	void AssertCanIterateOrdered(std::initializer_list<size_t> ids) {
		// Arrange:
		TTraits::RunEmptyCacheTest([ids](auto& cache) {
			// - seed the cache and get a view
			StateDecorator<TTraits, TEntityTraits> traitsForwarder;
			auto keys = traitsForwarder.InsertMultiple(cache, ids);
			auto view = cache.createView();

			// Sanity:
			CATAPULT_LOG(debug) << "cache is seeded with " << view->size() << " entities";
			EXPECT_EQ(ids.size(), view->size());

			// Act: iterate over all values and extract the keys
			using KeyType = typename decltype(keys)::value_type;
			std::vector<KeyType> iteratedKeys;
			for (auto iter = view->cbegin(); view->cend() != iter; ++iter)
				iteratedKeys.push_back(traitsForwarder.ToKey(*iter));

			// Assert: the entities can be iterated (in order)
			auto i = 0u;
			ASSERT_EQ(keys.size(), iteratedKeys.size());
			for (auto key : keys) {
				EXPECT_EQ(key, iteratedKeys[i]) << "entity at " << i;
				++i;
			}
		});
	}

	template<typename TTraits, typename TEntityTraits>
	void AssertCanIterateUnordered(std::initializer_list<size_t> ids) {
		// Arrange:
		TTraits::RunEmptyCacheTest([ids](auto& cache) {
			// - seed the cache and get a view
			StateDecorator<TTraits, TEntityTraits> traitsForwarder;
			auto keys = traitsForwarder.InsertMultiple(cache, ids);
			auto view = cache.createView();

			// Sanity:
			CATAPULT_LOG(debug) << "cache is seeded with " << view->size() << " entities";
			EXPECT_EQ(ids.size(), view->size());

			// Act: iterate over all values and extract the keys
			using KeyType = typename decltype(keys)::value_type;
			std::set<KeyType> iteratedKeys;
			for (auto iter = view->cbegin(); view->cend() != iter; ++iter)
				iteratedKeys.insert(traitsForwarder.ToKey(*iter));

			// Assert: the entities can be iterated
			auto i = 0u;
			ASSERT_EQ(keys.size(), iteratedKeys.size());
			for (auto key : keys) {
				EXPECT_TRUE(iteratedKeys.cend() != iteratedKeys.find(key)) << "entity " << test::ToString(key);
				++i;
			}
		});
	}

	// endregion
}}

#define MAKE_CACHE_ITERATION_TEST(TEST_CLASS, CACHE_TRAITS, ITERATION_TYPE, SUFFIX, TEST_NAME, VALUES) \
	TEST(TEST_CLASS, CanIterateThrough##TEST_NAME##SUFFIX) { \
		test::AssertCanIterate##ITERATION_TYPE<CACHE_TRAITS, CACHE_TRAITS::EntityTraits>(VALUES); \
	}

#define DEFINE_CACHE_ITERATION_TESTS_SUFFIX(TEST_CLASS, CACHE_TRAITS, ITERATION_TYPE, SUFFIX) \
	MAKE_CACHE_ITERATION_TEST(TEST_CLASS, CACHE_TRAITS, ITERATION_TYPE, SUFFIX, EmptyCache, {}) \
	MAKE_CACHE_ITERATION_TEST(TEST_CLASS, CACHE_TRAITS, ITERATION_TYPE, SUFFIX, SingleValueCache, { 1 }) \
	MAKE_CACHE_ITERATION_TEST(TEST_CLASS, CACHE_TRAITS, ITERATION_TYPE, SUFFIX, MultiValueCache, \
			(std::initializer_list<size_t>{ 1, 2, 3, 4, 5 }))

#define DEFINE_CACHE_ITERATION_TESTS(TEST_CLASS, CACHE_TRAITS, ITERATION_TYPE) \
	DEFINE_CACHE_ITERATION_TESTS_SUFFIX(TEST_CLASS, CACHE_TRAITS, ITERATION_TYPE,)
