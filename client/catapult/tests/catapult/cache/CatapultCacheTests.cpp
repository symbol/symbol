#include "catapult/cache/CatapultCache.h"
#include "catapult/cache/CacheStorage.h"
#include "catapult/cache/CatapultCacheBuilder.h"
#include "catapult/cache/ReadOnlyCatapultCache.h"
#include "tests/test/cache/CacheSynchronizationTests.h"
#include "tests/test/cache/SimpleCache.h"
#include "tests/test/core/mocks/MemoryStream.h"
#include "tests/TestHarness.h"

#define TEST_CLASS CatapultCacheTests

namespace catapult { namespace cache {
	namespace {
		void IncrementAllSubCaches(CatapultCacheDelta& delta) {
			// Act:
			delta.sub<test::SimpleCacheT<2>>().increment();
			delta.sub<test::SimpleCacheT<4>>().increment();
			delta.sub<test::SimpleCacheT<6>>().increment();
		}

		template<typename TView>
		void AssertSubCacheSizes(const TView& view, size_t expectedSize) {
			// Assert:
			EXPECT_EQ(expectedSize, view.template sub<test::SimpleCacheT<2>>().size());
			EXPECT_EQ(expectedSize, view.template sub<test::SimpleCacheT<4>>().size());
			EXPECT_EQ(expectedSize, view.template sub<test::SimpleCacheT<6>>().size());
		}

		template<size_t CacheId>
		void AddSubCacheWithId(CatapultCacheBuilder& builder) {
			builder.add<test::SimpleCacheStorageTraits>(std::make_unique<test::SimpleCacheT<CacheId>>());
		}

		CatapultCache CreateSimpleCatapultCache() {
			CatapultCacheBuilder builder;
			AddSubCacheWithId<2>(builder);
			AddSubCacheWithId<6>(builder);
			AddSubCacheWithId<4>(builder);
			return builder.build();
		}
	}

	// region ctor

	TEST(TEST_CLASS, CanCreateCatapultCache_View) {
		// Act:
		auto cache = CreateSimpleCatapultCache();
		auto view = cache.createView();

		// Assert:
		AssertSubCacheSizes(view, 0);
	}

	TEST(TEST_CLASS, CanCreateCatapultCache_Delta) {
		// Act:
		auto cache = CreateSimpleCatapultCache();
		auto delta = cache.createDelta();

		// Assert:
		AssertSubCacheSizes(delta, 0);
	}

	// endregion

	// region commit

	namespace {
		void CommitChangeToAllSubCaches(CatapultCache& cache) {
			// Arrange:
			auto delta = cache.createDelta();
			IncrementAllSubCaches(delta);

			// Act:
			cache.commit(Height());
		}
	}

	TEST(TEST_CLASS, CommitDelegatesToSubCaches_View) {
		// Arrange:
		auto cache = CreateSimpleCatapultCache();

		// Act:
		CommitChangeToAllSubCaches(cache);
		auto view = cache.createView();

		// Assert:
		AssertSubCacheSizes(view, 1);
	}

	TEST(TEST_CLASS, CommitDelegatesToSubCaches_Delta) {
		// Arrange:
		auto cache = CreateSimpleCatapultCache();

		// Act:
		CommitChangeToAllSubCaches(cache);
		auto delta = cache.createDelta();

		// Assert:
		AssertSubCacheSizes(delta, 1);
	}

	TEST(TEST_CLASS, CommitOfSubCacheInvalidatesDetachedDelta) {
		// Arrange:
		auto cache = CreateSimpleCatapultCache();
		auto lockableDelta = cache.createDetachableDelta().detach();
		{
			auto delta = cache.createDelta();
			delta.sub<test::SimpleCacheT<4>>().increment();

			// Sanity:
			std::thread([&lockableDelta]() {
				// - need to lock on a separate thread because test thread owns pDelta
				EXPECT_TRUE(!!lockableDelta.lock());
			}).join();

			// Act:
			cache.commit(Height());
		}

		// Assert:
		EXPECT_FALSE(!!lockableDelta.lock());
	}

	// endregion

	// region synchronization

	namespace {
		void CommitWithNoChanges(CatapultCache& cache) {
			auto delta = cache.createDelta();
			cache.commit(Height());
		}
	}

	TEST(TEST_CLASS, ReadLockBlocksCommitOfSubCache) {
		// Arrange:
		auto cache = CreateSimpleCatapultCache();

		// Assert:
		test::AssertExclusiveLocks(
				[&cache]() { return CatapultCacheView(cache.createView()); },
				[&cache]() { CommitWithNoChanges(cache); });
	}

	TEST(TEST_CLASS, DetachableDeltaBlocksCommitOfSubCache) {
		// Arrange:
		auto cache = CreateSimpleCatapultCache();

		// Assert:
		test::AssertExclusiveLocks(
				[&cache]() { return CatapultCacheDetachableDelta(cache.createDetachableDelta()); },
				[&cache]() { CommitWithNoChanges(cache); });
	}

	TEST(TEST_CLASS, DetachedDetachableDeltaBlocksCommitOfSubCache) {
		// Arrange:
		auto cache = CreateSimpleCatapultCache();

		// Assert:
		test::AssertExclusiveLocks(
				[&cache]() {
					auto detachableDelta = cache.createDetachableDelta();
					detachableDelta.detach();
					return CatapultCacheDetachableDelta(std::move(detachableDelta));
				},
				[&cache]() { CommitWithNoChanges(cache); });
	}

	// endregion

	// region cache height

	TEST(TEST_CLASS, CacheHeightIsInitiallyZero) {
		// Act:
		auto cache = CreateSimpleCatapultCache();

		// Assert:
		EXPECT_EQ(Height(0), cache.createView().height());
		EXPECT_EQ(Height(0), cache.createDetachableDelta().height());
	}

	TEST(TEST_CLASS, CommitUpdatesCacheHeight) {
		// Arrange:
		auto cache = CreateSimpleCatapultCache();
		{
			// Act:
			auto delta = cache.createDelta();
			cache.commit(Height(123));
		}

		// Assert:
		EXPECT_EQ(Height(123), cache.createView().height());
		EXPECT_EQ(Height(123), cache.createDetachableDelta().height());
	}

	// endregion

	// region toReadOnly

	TEST(TEST_CLASS, CanAcquireReadOnlyViewOfView) {
		// Arrange:
		auto cache = CreateSimpleCatapultCache();
		{
			auto delta = cache.createDelta();
			IncrementAllSubCaches(delta); // committed
			cache.commit(Height());
			IncrementAllSubCaches(delta); // uncommitted
		}

		// Act:
		auto view = cache.createView();
		auto readOnlyView = view.toReadOnly();

		// Assert: only committed changes are present
		AssertSubCacheSizes(readOnlyView, 1);
	}

	TEST(TEST_CLASS, CanAcquireReadOnlyViewOfDelta) {
		// Arrange:
		auto cache = CreateSimpleCatapultCache();
		auto delta = cache.createDelta();
		IncrementAllSubCaches(delta); // committed
		cache.commit(Height());
		IncrementAllSubCaches(delta); // uncommitted

		// Act:
		auto readOnlyView = delta.toReadOnly();

		// Assert: both committed and uncommitted changes are present
		AssertSubCacheSizes(readOnlyView, 2);
	}

	// endregion

	// region storages

	TEST(TEST_CLASS, CanRoundTripCacheViaStorages) {
		// Arrange: seed the cache with 9 items per subcache
		std::vector<std::vector<uint8_t>> serializedSubCaches;
		{
			auto cache = CreateSimpleCatapultCache();
			{
				auto delta = cache.createDelta();
				for (auto i = 1u; i <= 9u; ++i)
					IncrementAllSubCaches(delta);

				cache.commit(Height());
			}

			// Act: save all data
			for (const auto& pStorage : const_cast<const CatapultCache&>(cache).storages()) {
				std::vector<uint8_t> buffer;
				mocks::MemoryStream stream("", buffer);
				pStorage->saveAll(stream);
				serializedSubCaches.push_back(buffer);
			}
		}

		// - load all data
		auto i = 0u;
		auto cache = CreateSimpleCatapultCache();
		for (const auto& pStorage : cache.storages()) {
			mocks::MemoryStream stream("", serializedSubCaches[i++]);
			pStorage->loadAll(stream, 5);
		}

		// Assert: the cache data was loaded successfully
		auto view = cache.createView();
		AssertSubCacheSizes(view, 9);
	}

	// endregion
}}

namespace catapult { namespace test {
	template<>
	void AssertCacheContents(const cache::CatapultCacheView& view, const std::vector<size_t>& expectedEntities) {
		test::AssertCacheContents(view.sub<test::SimpleCacheT<4>>(), expectedEntities);
	}

	template<>
	void AssertCacheContents(const cache::CatapultCacheDelta& delta, const std::vector<size_t>& expectedEntities) {
		test::AssertCacheContents(delta.sub<test::SimpleCacheT<4>>(), expectedEntities);
	}
}}

namespace catapult { namespace cache {
	// region general cache tests

	namespace {
		class CatapultCacheTestAdapter {
		public:
			CatapultCacheTestAdapter() : m_cache(CreateSimpleCatapultCache())
			{}

		public:
			auto createView() const {
				// unique_ptr to allow one level of indirection as required by DEFINE_CACHE_SYNC_TESTS
				return std::make_unique<CatapultCacheView>(m_cache.createView());
			}

			auto createDelta() {
				// unique_ptr to allow one level of indirection as required by DEFINE_CACHE_SYNC_TESTS
				return std::make_unique<CatapultCacheDelta>(m_cache.createDelta());
			}

			auto createDetachedDelta() const {
				return m_cache.createDetachableDelta().detach();
			}

			void commit() {
				m_cache.commit(Height());
			}

		private:
			CatapultCache m_cache;
		};

		struct CatapultCacheTraits {
		public:
			using EntityVector = std::vector<size_t>;

		public:
			template<typename TAction>
			static void RunCacheTest(TAction action) {
				// Arrange: use the test::SimpleCacheT<4> as a marker cache
				CatapultCacheTestAdapter cacheAdapter;
				EntityVector entities;
				{
					auto pDelta = cacheAdapter.createDelta();
					auto& subCacheDelta = pDelta->sub<test::SimpleCacheT<4>>();
					for (auto id = 1u; id <= 3u; ++id) {
						subCacheDelta.increment();
						entities.push_back(id);
					}

					cacheAdapter.commit();
				}

				// Act:
				action(cacheAdapter, entities);
			}
		};
	}

	DEFINE_CACHE_SYNC_TESTS(TEST_CLASS, CatapultCacheTraits)

	// endregion
}}
