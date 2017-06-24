#pragma once
#include "TraitsStateDecorator.h"
#include "tests/test/cache/CacheSetUtils.h"

namespace catapult { namespace test {

	// region constructor

	template<typename TTraits>
	void AssertCacheIsInitiallyEmpty() {
		// Arrange:
		TTraits::RunEmptyCacheTest([](auto& cache) {
			// Assert:
			EXPECT_EQ(0u, cache.createView()->size());
			EXPECT_EQ(0u, cache.createDelta()->size());
			EXPECT_EQ(0u, cache.createDetachedDelta().lock()->size());
		});
	}

	// endregion

	// region contains

	template<typename TTraits, typename TEntityTraits>
	void AssertViewContainsCacheEntities() {
		// Arrange:
		TTraits::RunEmptyCacheTest([](auto& cache) {
			StateDecorator<TTraits, TEntityTraits> traitsForwarder;
			traitsForwarder.InsertMultiple(cache, { 1, 2, 3 });

			auto view = cache.createView();
			auto entity1 = traitsForwarder.CreateEntity(1); // is contained in cache
			auto entity2 = traitsForwarder.CreateEntity(123); // is not contained in cache

			// Act:
			auto contains1 = view->contains(traitsForwarder.ToKey(entity1));
			auto contains2 = view->contains(traitsForwarder.ToKey(entity2));

			// Assert:
			EXPECT_TRUE(contains1);
			EXPECT_FALSE(contains2);
		});
	}

	template<typename TTraits, typename TEntityTraits>
	void AssertDeltaContainsCacheEntities() {
		// Arrange:
		TTraits::RunEmptyCacheTest([](auto& cache) {
			StateDecorator<TTraits, TEntityTraits> traitsForwarder;
			traitsForwarder.InsertMultiple(cache, { 1, 2, 3 });
			auto delta = cache.createDelta();
			auto entity1 = traitsForwarder.CreateEntity(1); // is contained in cache
			auto entity2 = traitsForwarder.CreateEntity(123); // is not contained in cache

			// Act:
			auto contains1 = delta->contains(traitsForwarder.ToKey(entity1));
			auto contains2 = delta->contains(traitsForwarder.ToKey(entity2));

			// Assert:
			EXPECT_TRUE(contains1);
			EXPECT_FALSE(contains2);
		});
	}

	// endregion

	// region insert

	template<typename TTraits, typename TEntityTraits>
	void AssertCanInsertEntityIntoCache() {
		// Arrange:
		TTraits::RunEmptyCacheTest([](auto& cache) {
			StateDecorator<TTraits, TEntityTraits> traitsForwarder;
			traitsForwarder.InsertMultiple(cache, { 1, 2, 3, 4, 5, 6 });
			auto delta = cache.createDelta();
			auto entity = traitsForwarder.CreateEntity(7);

			// Sanity check:
			EXPECT_EQ(6u, delta->size());
			EXPECT_FALSE(delta->contains(traitsForwarder.ToKey(entity)));

			// Act:
			TTraits::Insert(*delta, entity);

			// Assert:
			EXPECT_EQ(7u, delta->size());
			EXPECT_TRUE(delta->contains(traitsForwarder.ToKey(entity)));
		});
	}

	// endregion

	// region remove

	template<typename TTraits, typename TEntityTraits>
	void AssertCanRemoveEntityFromCache() {
		// Arrange:
		TTraits::RunEmptyCacheTest([](auto& cache) {
			StateDecorator<TTraits, TEntityTraits> traitsForwarder;
			traitsForwarder.InsertMultiple(cache, { 1, 2, 3, 4, 5, 6 });
			auto delta = cache.createDelta();
			auto entity1 = traitsForwarder.CreateEntity(6); // is contained in cache
			auto entity2 = traitsForwarder.CreateEntity(5); // is contained in cache

			// Sanity check:
			EXPECT_EQ(6u, delta->size());
			EXPECT_TRUE(delta->contains(traitsForwarder.ToKey(entity1)));
			EXPECT_TRUE(delta->contains(traitsForwarder.ToKey(entity2)));

			// Act:
			traitsForwarder.Remove(*delta, entity1);
			traitsForwarder.Remove(*delta, entity2);

			// Assert:
			EXPECT_EQ(4u, delta->size());
			EXPECT_FALSE(delta->contains(traitsForwarder.ToKey(entity1)));
			EXPECT_FALSE(delta->contains(traitsForwarder.ToKey(entity2)));
		});
	}

	// endregion

	// region commit

	template<typename TTraits, typename TEntityTraits>
	void AssertCanCommitToCache(size_t count) {
		// Arrange:
		TTraits::RunEmptyCacheTest([count](auto& cache) {
			StateDecorator<TTraits, TEntityTraits> traitsForwarder;
			traitsForwarder.InsertMultiple(cache, { 1, 2, 3, 4 });
			auto entity2 = traitsForwarder.CreateEntity(2);
			auto entity3 = traitsForwarder.CreateEntity(3);
			auto entity4 = traitsForwarder.CreateEntity(4);
			{
				auto delta = cache.createDelta();
				traitsForwarder.Remove(*delta, entity4);
				traitsForwarder.Remove(*delta, entity3);
				traitsForwarder.Remove(*delta, entity2);
				TTraits::Insert(*delta, entity2);

				// Act:
				for (auto i = 0u; i < count; ++i)
					cache.commit();
			}

			// Assert:
			auto view = cache.createView();
			EXPECT_EQ(2u, view->size());
			EXPECT_TRUE(view->contains(traitsForwarder.ToKey(traitsForwarder.CreateEntity(1))));
			EXPECT_TRUE(view->contains(traitsForwarder.ToKey(entity2)));
			EXPECT_FALSE(view->contains(traitsForwarder.ToKey(entity3)));
			EXPECT_FALSE(view->contains(traitsForwarder.ToKey(entity4)));
		});
	}

	template<typename TTraits>
	void AssertCommitWithNoPendingChangesHasNoEffect() {
		// Arrange:
		TTraits::RunCacheTest([](auto& cache, const auto& entities) {
			// Act:
			{
				auto delta = cache.createDelta();
				cache.commit();
			}

			// Assert:
			auto view = cache.createView();
			test::AssertCacheContents(*view, entities);
		});
	}

	template<typename TTraits>
	void AssertCommitThrowsIfOnlyDetachedDeltasAreOutstanding() {
		// Arrange:
		TTraits::RunEmptyCacheTest([](auto& cache) {
			// Act:
			auto lockableDelta = cache.createDetachedDelta();

			// Assert:
			EXPECT_THROW(cache.commit(), catapult_runtime_error);
		});
	}

	// endregion

	// region asReadOnly

	template<typename TTraits, typename TEntityTraits, typename TReadOnlyView>
	void AssertDefaultReadOnlyView(StateDecorator<TTraits, TEntityTraits>& traitsForwarder, const TReadOnlyView& readOnlyView) {
		// Arrange:
		auto entity1 = traitsForwarder.CreateEntity(1); // is contained in cache
		auto entity2 = traitsForwarder.CreateEntity(123); // is not contained in cache

		// Act:
		auto contains1 = readOnlyView.contains(traitsForwarder.ToKey(entity1));
		auto contains2 = readOnlyView.contains(traitsForwarder.ToKey(entity2));

		// Assert:
		EXPECT_EQ(3u, readOnlyView.size());
		EXPECT_TRUE(contains1);
		EXPECT_FALSE(contains2);
	}

	template<typename TTraits, typename TEntityTraits>
	void AssertCanOverlayReadOnlyViewOnView() {
		// Arrange:
		TTraits::RunEmptyCacheTest([](auto& cache) {
			StateDecorator<TTraits, TEntityTraits> traitsForwarder;
			traitsForwarder.InsertMultiple(cache, { 1, 2, 3 });
			auto view = cache.createView();

			// Act:
			auto readOnlyView = view->asReadOnly();

			// Assert:
			AssertDefaultReadOnlyView(traitsForwarder, readOnlyView);
		});
	}

	template<typename TTraits, typename TEntityTraits>
	void AssertCanOverlayReadOnlyViewOnDelta() {
		// Arrange:
		TTraits::RunEmptyCacheTest([](auto& cache) {
			StateDecorator<TTraits, TEntityTraits> traitsForwarder;
			traitsForwarder.InsertMultiple(cache, { 1, 2, 3 });
			auto delta = cache.createDelta();

			// Act:
			auto readOnlyView = delta->asReadOnly();

			// Assert:
			AssertDefaultReadOnlyView(traitsForwarder, readOnlyView);
		});
	}

	// endregion
}}

#define MAKE_CACHE_CONTENTS_NON_ENTITY_TEST(TEST_CLASS, CACHE_TRAITS, SUFFIX, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME##SUFFIX) { test::Assert##TEST_NAME<CACHE_TRAITS>(); }

#define MAKE_CACHE_CONTENTS_ENTITY_TEST(TEST_CLASS, CACHE_TRAITS, SUFFIX, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME##SUFFIX) { test::Assert##TEST_NAME<CACHE_TRAITS, CACHE_TRAITS::EntityTraits>(); }

#define DEFINE_CACHE_CONTENTS_TESTS_SUFFIX(TEST_CLASS, CACHE_TRAITS, SUFFIX) \
	MAKE_CACHE_CONTENTS_NON_ENTITY_TEST(TEST_CLASS, CACHE_TRAITS, SUFFIX, CacheIsInitiallyEmpty) \
	\
	MAKE_CACHE_CONTENTS_ENTITY_TEST(TEST_CLASS, CACHE_TRAITS, SUFFIX, ViewContainsCacheEntities) \
	MAKE_CACHE_CONTENTS_ENTITY_TEST(TEST_CLASS, CACHE_TRAITS, SUFFIX, DeltaContainsCacheEntities) \
	\
	MAKE_CACHE_CONTENTS_ENTITY_TEST(TEST_CLASS, CACHE_TRAITS, SUFFIX, CanInsertEntityIntoCache) \
	MAKE_CACHE_CONTENTS_ENTITY_TEST(TEST_CLASS, CACHE_TRAITS, SUFFIX, CanRemoveEntityFromCache) \
	\
	TEST(TEST_CLASS, CanCommitToUnderlyingCache##SUFFIX) { test::AssertCanCommitToCache<CACHE_TRAITS, CACHE_TRAITS::EntityTraits>(1); } \
	TEST(TEST_CLASS, CommitIsIdempotent##SUFFIX) { test::AssertCanCommitToCache<CACHE_TRAITS, CACHE_TRAITS::EntityTraits>(10); } \
	\
	MAKE_CACHE_CONTENTS_NON_ENTITY_TEST(TEST_CLASS, CACHE_TRAITS, SUFFIX, CommitWithNoPendingChangesHasNoEffect) \
	MAKE_CACHE_CONTENTS_NON_ENTITY_TEST(TEST_CLASS, CACHE_TRAITS, SUFFIX, CommitThrowsIfOnlyDetachedDeltasAreOutstanding) \
	\
	MAKE_CACHE_CONTENTS_ENTITY_TEST(TEST_CLASS, CACHE_TRAITS, SUFFIX, CanOverlayReadOnlyViewOnView) \
	MAKE_CACHE_CONTENTS_ENTITY_TEST(TEST_CLASS, CACHE_TRAITS, SUFFIX, CanOverlayReadOnlyViewOnDelta)

#define DEFINE_CACHE_CONTENTS_TESTS(TEST_CLASS, CACHE_TRAITS) DEFINE_CACHE_CONTENTS_TESTS_SUFFIX(TEST_CLASS, CACHE_TRAITS,)
