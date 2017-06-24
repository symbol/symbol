#pragma once
#include "tests/test/cache/CacheSetUtils.h"
#include "tests/test/nodeps/LockTestUtils.h"

namespace catapult { namespace test {

	// region createView

	template<typename TTraits>
	void AssertCanCreateView() {
		// Arrange:
		TTraits::RunCacheTest([](auto& cache, const auto& entities) {
			// Act:
			auto view = cache.createView();

			// Assert:
			test::AssertCacheContents(*view, entities);
		});
	}

	template<typename TTraits>
	void AssertCanCreateMultipleViews() {
		// Arrange:
		TTraits::RunCacheTest([](auto& cache, const auto& entities) {
			// Assert:
			test::CanCreateSubObjectOnMultipleThreads(
					cache,
					[&cache](const auto&) { return cache.createView(); },
					[&entities](const auto& view) { test::AssertCacheContents(*view, entities); });
		});
	}

	template<typename TTraits>
	void AssertCommitIsBlockedByView() {
		// Arrange:
		TTraits::RunCacheTest([](auto& cache, const auto&) {
			// Assert:
			test::AssertExclusiveLocks(
					[&cache]() { return cache.createView(); },
					[&cache]() {
						auto delta = cache.createDelta();
						return cache.commit();
					});
		});
	}

	// endregion

	// region createDelta

	template<typename TTraits>
	void AssertCanCreateSingleDelta() {
		// Arrange:
		TTraits::RunCacheTest([](auto& cache, const auto& entities) {
			// Act:
			auto delta = cache.createDelta();

			// Assert:
			test::AssertCacheContents(*delta, entities);
		});
	}

	template<typename TTraits>
	void AssertCannotCreateMultipleDeltas() {
		// Arrange:
		TTraits::RunCacheTest([](auto& cache, const auto& entities) {
			{
				auto delta = cache.createDelta();

				// Act + Assert:
				EXPECT_THROW(cache.createDelta(), catapult_runtime_error);
			}

			// Act: cache delta went out of scope, another delta is allowed
			auto delta = cache.createDelta();

			// Assert:
			test::AssertCacheContents(*delta, entities);
		});
	}

	// endregion

	// region createDetachedDelta

	template<typename TTraits>
	void AssertCanCreateDetachedDelta() {
		// Arrange:
		TTraits::RunCacheTest([](auto& cache, const auto& entities) {
			// Act:
			auto lockableDelta = cache.createDetachedDelta();

			// Assert:
			test::AssertCacheContents(*lockableDelta.lock(), entities);
		});
	}

	template<typename TTraits>
	void AssertCanCreateMultipleDetachedDeltas() {
		// Arrange:
		TTraits::RunCacheTest([](auto& cache, const auto& entities) {
			std::vector<decltype(cache.createDetachedDelta())> deltas;

			// Act:
			for (auto i = 0u; i < 10; ++i)
				deltas.push_back(cache.createDetachedDelta());

			// Assert:
			for (auto& lockableDelta : deltas)
				test::AssertCacheContents(*lockableDelta.lock(), entities);
		});
	}

	// endregion

	// region commit locking

	template<typename TTraits>
	void AssertCommitIsBlockedByLockedDetachedDelta() {
		// Arrange:
		TTraits::RunCacheTest([](auto& cache, const auto&) {
			using LockableCacheDeltaType = decltype(cache.createDetachedDelta());
			using LockedCacheDeltaType = decltype(cache.createDetachedDelta().lock());

			struct DetachedDeltaGuard {
			public:
				explicit DetachedDeltaGuard(LockableCacheDeltaType&& lockableDelta)
						: m_lockableDelta(std::move(lockableDelta))
						, m_delta(m_lockableDelta.lock())
				{}

			private:
				LockableCacheDeltaType m_lockableDelta;
				LockedCacheDeltaType m_delta;
			};

			// Assert:
			test::AssertExclusiveLocks(
					[&cache]() { return DetachedDeltaGuard(cache.createDetachedDelta()); },
					[&cache]() {
						auto delta = cache.createDelta();
						return cache.commit();
					});
		});
	}

	template<typename TTraits>
	void AssertCommitIsNotBlockedByDetachedDelta() {
		// Arrange:
		TTraits::RunCacheTest([](auto& cache, const auto&) {
			auto detachedDelta = cache.createDetachedDelta();
			{
				auto delta = cache.createDelta();

				// Sanity:
				std::thread([&detachedDelta]() {
					// - need to lock on a separate thread because test thread owns pDelta
					EXPECT_TRUE(detachedDelta.lock());
				}).join();

				// Act:
				cache.commit();
			}

			// Assert:
			EXPECT_FALSE(detachedDelta.lock());
		});
	}

	// endregion
}}

#define DEFINE_CACHE_SYNC_TESTS(TEST_CLASS, CACHE_TRAITS) \
	TEST(TEST_CLASS, CanCreateView) { test::AssertCanCreateView<CACHE_TRAITS>(); } \
	TEST(TEST_CLASS, CanCreateMultipleViews) { test::AssertCanCreateMultipleViews<CACHE_TRAITS>(); } \
	TEST(TEST_CLASS, CommitIsBlockedByView) { test::AssertCommitIsBlockedByView<CACHE_TRAITS>(); } \
	\
	TEST(TEST_CLASS, CanCreateSingleDelta) { test::AssertCanCreateSingleDelta<CACHE_TRAITS>(); } \
	TEST(TEST_CLASS, CannotCreateMultipleDeltas) { test::AssertCannotCreateMultipleDeltas<CACHE_TRAITS>(); } \
	\
	TEST(TEST_CLASS, CanCreateDetachedDelta) { test::AssertCanCreateDetachedDelta<CACHE_TRAITS>(); } \
	TEST(TEST_CLASS, CanCreateMultipleDetachedDeltas) { test::AssertCanCreateMultipleDetachedDeltas<CACHE_TRAITS>(); } \
	\
	TEST(TEST_CLASS, CommitIsBlockedByLockedDetachedDelta) { test::AssertCommitIsBlockedByLockedDetachedDelta<CACHE_TRAITS>(); } \
	TEST(TEST_CLASS, CommitIsNotBlockedByDetachedDelta) { test::AssertCommitIsNotBlockedByDetachedDelta<CACHE_TRAITS>(); }
