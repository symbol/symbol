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
#include "CacheMixinsTests.h"
#include "tests/test/nodeps/LockTestUtils.h"

namespace catapult { namespace test {

	/// Test suite for basic cache functionality.
	template<typename TTraits>
	class CacheBasicTests {
	private:
		using CacheType = typename TTraits::CacheType;

	public:
		// region createView

		static void AssertCanCreateView() {
			// Arrange:
			CacheType cache;
			detail::InsertMultiple<TTraits>(cache, { 1, 2, 3 });

			// Act:
			auto view = cache.createView();

			// Assert:
			AssertDefaultCacheContents(*view);
		}

		static void AssertCanCreateMultipleViews() {
			// Arrange:
			CacheType cache;
			detail::InsertMultiple<TTraits>(cache, { 1, 2, 3 });

			// Act + Assert:
			auto createView = [&cache](const auto&) { return cache.createView(); };
			test::CanCreateSubObjectOnMultipleThreads(cache, createView, [](const auto& view) {
				AssertDefaultCacheContents(*view);
			});
		}

		static void AssertCommitIsBlockedByView() {
			// Arrange:
			CacheType cache;
			detail::InsertMultiple<TTraits>(cache, { 1, 2, 3 });

			// Act + Assert:
			auto createView = [&cache]() { return cache.createView(); };
			test::AssertExclusiveLocks(createView, [&cache]() {
				auto delta = cache.createDelta();
				return cache.commit();
			});
		}

		// endregion

		// region createDelta

		static void AssertCanCreateSingleDelta() {
			// Arrange:
			CacheType cache;
			detail::InsertMultiple<TTraits>(cache, { 1, 2, 3 });

			// Act:
			auto delta = cache.createDelta();

			// Assert:
			AssertDefaultCacheContents(*delta);
		}

		static void AssertCannotCreateMultipleDeltas() {
			// Arrange:
			CacheType cache;
			detail::InsertMultiple<TTraits>(cache, { 1, 2, 3 });

			{
				auto delta = cache.createDelta();

				// Act + Assert:
				EXPECT_THROW(cache.createDelta(), catapult_runtime_error);
			}

			// Act: cache delta went out of scope, another delta is allowed
			auto delta = cache.createDelta();

			// Assert:
			AssertDefaultCacheContents(*delta);
		}

		// endregion

		// region createDetachedDelta

		static void AssertCanCreateDetachedDelta() {
			// Arrange:
			CacheType cache;
			detail::InsertMultiple<TTraits>(cache, { 1, 2, 3 });

			// Act:
			auto lockableDelta = cache.createDetachedDelta();

			// Assert:
			AssertDefaultCacheContents(*lockableDelta.tryLock());
		}

		static void AssertCanCreateMultipleDetachedDeltas() {
			// Arrange:
			CacheType cache;
			detail::InsertMultiple<TTraits>(cache, { 1, 2, 3 });
			std::vector<decltype(cache.createDetachedDelta())> deltas;

			// Act:
			for (auto i = 0u; i < 10; ++i)
				deltas.push_back(cache.createDetachedDelta());

			// Assert:
			for (auto& lockableDelta : deltas)
				AssertDefaultCacheContents(*lockableDelta.tryLock());
		}

		// endregion

		// region commit (locking)

		static void AssertCommitIsBlockedByLockedDetachedDelta() {
			// Arrange:
			CacheType cache;
			detail::InsertMultiple<TTraits>(cache, { 1, 2, 3 });

			using LockableCacheDeltaType = decltype(cache.createDetachedDelta());
			using LockedCacheDeltaType = decltype(cache.createDetachedDelta().tryLock());

			struct DetachedDeltaGuard {
			public:
				explicit DetachedDeltaGuard(LockableCacheDeltaType&& lockableDelta)
						: m_lockableDelta(std::move(lockableDelta))
						, m_delta(m_lockableDelta.tryLock())
				{}

			private:
				LockableCacheDeltaType m_lockableDelta;
				LockedCacheDeltaType m_delta;
			};

			// Assert:
			auto createDetachedDelta = [&cache]() { return DetachedDeltaGuard(cache.createDetachedDelta()); };
			test::AssertExclusiveLocks(createDetachedDelta, [&cache]() {
				auto delta = cache.createDelta();
				return cache.commit();
			});
		}

		static void AssertCommitIsNotBlockedByDetachedDelta() {
			// Arrange:
			CacheType cache;
			detail::InsertMultiple<TTraits>(cache, { 1, 2, 3 });

			auto detachedDelta = cache.createDetachedDelta();
			{
				auto delta = cache.createDelta();

				// Sanity:
				std::thread([&detachedDelta]() {
					// - need to lock on a separate thread because test thread owns pDelta
					EXPECT_TRUE(detachedDelta.tryLock());
				}).join();

				// Act:
				cache.commit();
			}

			// Assert:
			EXPECT_FALSE(detachedDelta.tryLock());
		}

		// endregion

		// region commit (other)

		static void AssertCanCommitToUnderlyingCache() {
			AssertCanCommitToCache(1);
		}

		static void AssertCommitIsIdempotent() {
			AssertCanCommitToCache(10);
		}

		static void AssertCommitWithNoPendingChangesHasNoEffect() {
			// Arrange:
			CacheType cache;
			detail::InsertMultiple<TTraits>(cache, { 1, 2, 3 });

			// Act:
			{
				auto delta = cache.createDelta();
				cache.commit();
			}

			// Assert:
			auto view = cache.createView();
			AssertDefaultCacheContents(*view);
		}

		static void AssertCommitThrowsWhenOnlyDetachedDeltasAreOutstanding() {
			// Arrange:
			CacheType cache;
			auto lockableDelta = cache.createDetachedDelta();

			// Act + Assert:
			EXPECT_THROW(cache.commit(), catapult_runtime_error);
		}

		// endregion

		// region asReadOnly

		static void AssertCanOverlayReadOnlyViewOnView() {
			// Arrange:
			CacheType cache;
			detail::InsertMultiple<TTraits>(cache, { 1, 2, 3 });
			auto view = cache.createView();

			// Act:
			auto readOnlyView = view->asReadOnly();

			// Assert:
			AssertDefaultReadOnlyView(readOnlyView);
		}

		static void AssertCanOverlayReadOnlyViewOnDelta() {
			// Arrange:
			CacheType cache;
			detail::InsertMultiple<TTraits>(cache, { 1, 2, 3 });
			auto delta = cache.createDelta();

			// Act:
			auto readOnlyView = delta->asReadOnly();

			// Assert:
			AssertDefaultReadOnlyView(readOnlyView);
		}

		// endregion

	private:
		template<typename TView>
		static void AssertDefaultCacheContents(const TView& view) {
			EXPECT_EQ(3u, view.size());
			EXPECT_TRUE(view.contains(TTraits::MakeId(1)));
			EXPECT_TRUE(view.contains(TTraits::MakeId(2)));
			EXPECT_TRUE(view.contains(TTraits::MakeId(3)));
		}

		static void AssertCanCommitToCache(size_t count) {
			// Arrange: seed the cache with four elements
			CacheType cache;
			detail::InsertMultiple<TTraits>(cache, { 1, 2, 3, 4 });

			// - remove three and add one
			auto element2 = TTraits::CreateWithId(2);
			{
				auto delta = cache.createDelta();
				delta->remove(TTraits::MakeId(4));
				delta->remove(TTraits::MakeId(3));
				delta->remove(TTraits::MakeId(2));

				delta->insert(element2);

				// Act:
				for (auto i = 0u; i < count; ++i)
					cache.commit();
			}

			// Assert:
			auto view = cache.createView();
			EXPECT_EQ(2u, view->size());
			EXPECT_TRUE(view->contains(TTraits::MakeId(1)));
			EXPECT_TRUE(view->contains(TTraits::MakeId(2)));
			EXPECT_FALSE(view->contains(TTraits::MakeId(3)));
			EXPECT_FALSE(view->contains(TTraits::MakeId(4)));
		}

		template<typename TReadOnlyView>
		static void AssertDefaultReadOnlyView(const TReadOnlyView& readOnlyView) {
			// Act:
			auto contains1 = readOnlyView.contains(TTraits::MakeId(1)); // is contained in cache
			auto contains2 = readOnlyView.contains(TTraits::MakeId(123)); // is not contained in cache

			// Assert:
			EXPECT_EQ(3u, readOnlyView.size());
			EXPECT_TRUE(contains1);
			EXPECT_FALSE(contains2);
		}
	};

#define MAKE_CACHE_BASIC_TEST(CACHE_TRAITS, SUFFIX, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME##SUFFIX) { test::CacheBasicTests<CACHE_TRAITS>::Assert##TEST_NAME(); }

#define DEFINE_CACHE_SYNC_TESTS(CACHE_TRAITS, SUFFIX) \
	MAKE_CACHE_BASIC_TEST(CACHE_TRAITS, SUFFIX, CanCreateView) \
	MAKE_CACHE_BASIC_TEST(CACHE_TRAITS, SUFFIX, CanCreateMultipleViews) \
	MAKE_CACHE_BASIC_TEST(CACHE_TRAITS, SUFFIX, CommitIsBlockedByView) \
	\
	MAKE_CACHE_BASIC_TEST(CACHE_TRAITS, SUFFIX, CanCreateSingleDelta) \
	MAKE_CACHE_BASIC_TEST(CACHE_TRAITS, SUFFIX, CannotCreateMultipleDeltas) \
	\
	MAKE_CACHE_BASIC_TEST(CACHE_TRAITS, SUFFIX, CanCreateDetachedDelta) \
	MAKE_CACHE_BASIC_TEST(CACHE_TRAITS, SUFFIX, CanCreateMultipleDetachedDeltas) \
	\
	MAKE_CACHE_BASIC_TEST(CACHE_TRAITS, SUFFIX, CommitIsBlockedByLockedDetachedDelta) \
	MAKE_CACHE_BASIC_TEST(CACHE_TRAITS, SUFFIX, CommitIsNotBlockedByDetachedDelta)

#define DEFINE_CACHE_BASIC_TESTS(CACHE_TRAITS, SUFFIX) \
	DEFINE_CACHE_SYNC_TESTS(CACHE_TRAITS, SUFFIX) \
	\
	MAKE_CACHE_BASIC_TEST(CACHE_TRAITS, SUFFIX, CanCommitToUnderlyingCache) \
	MAKE_CACHE_BASIC_TEST(CACHE_TRAITS, SUFFIX, CommitIsIdempotent) \
	MAKE_CACHE_BASIC_TEST(CACHE_TRAITS, SUFFIX, CommitWithNoPendingChangesHasNoEffect) \
	MAKE_CACHE_BASIC_TEST(CACHE_TRAITS, SUFFIX, CommitThrowsWhenOnlyDetachedDeltasAreOutstanding) \
	\
	MAKE_CACHE_BASIC_TEST(CACHE_TRAITS, SUFFIX, CanOverlayReadOnlyViewOnView) \
	MAKE_CACHE_BASIC_TEST(CACHE_TRAITS, SUFFIX, CanOverlayReadOnlyViewOnDelta)
}}
