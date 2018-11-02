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

#include "catapult/cache/CatapultCache.h"
#include "catapult/cache/CacheStorage.h"
#include "catapult/cache/CatapultCacheBuilder.h"
#include "catapult/cache/ReadOnlyCatapultCache.h"
#include "catapult/crypto/Hashes.h"
#include "tests/test/cache/CacheBasicTests.h"
#include "tests/test/cache/SimpleCache.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS CatapultCacheTests

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
		void AddSubCacheWithId(CatapultCacheBuilder& builder, test::SimpleCacheViewMode viewMode = test::SimpleCacheViewMode::Iterable) {
			builder.add<test::SimpleCacheStorageTraits>(std::make_unique<test::SimpleCacheT<CacheId>>(viewMode));
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

	// region state hash

	namespace {
		struct ViewTraits {
			static auto CreateView(const CatapultCache& cache) {
				return cache.createView();
			}

			template<typename TView>
			static Hash256 GetMerkleRoot(const TView& view) {
				return view.tryGetMerkleRoot().first;
			}

			static auto CalculateStateHash(const CatapultCacheView& view) {
				return view.calculateStateHash();
			}
		};

		struct DeltaTraits {
			static auto CreateView(CatapultCache& cache) {
				return cache.createDelta();
			}

			template<typename TView>
			static Hash256 GetMerkleRoot(const TView& view) {
				auto merkleRoot = view.tryGetMerkleRoot().first;
				merkleRoot[0] = 123; // SimpleCache updateMerkleRoot changes the first byte of the merkle root
				return merkleRoot;
			}

			static auto CalculateStateHash(const CatapultCacheDelta& view) {
				return view.calculateStateHash(Height(123));
			}
		};
	}

#define VIEW_DELTA_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_View) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ViewTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Delta) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<DeltaTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	VIEW_DELTA_TEST(StateHashIsZeroWhenStateCalculationIsDisabled) {
		// Arrange:
		auto cache = CreateSimpleCatapultCache();
		auto view = TTraits::CreateView(cache);

		// Act + Assert:
		EXPECT_EQ(Hash256{}, TTraits::CalculateStateHash(view).StateHash);
	}

	VIEW_DELTA_TEST(SubCacheMerkleRootsAreEmptyWhenStateCalculationIsDisabled) {
		// Arrange:
		auto cache = CreateSimpleCatapultCache();
		auto view = TTraits::CreateView(cache);

		// Act + Assert:
		EXPECT_TRUE(TTraits::CalculateStateHash(view).SubCacheMerkleRoots.empty());
	}

	namespace {
		CatapultCache CreateSimpleCatapultCacheForStateHashTests() {
			// Arrange: two of the four sub caches support merkle roots
			CatapultCacheBuilder builder;
			AddSubCacheWithId<6>(builder, test::SimpleCacheViewMode::Merkle_Root);
			AddSubCacheWithId<8>(builder);
			AddSubCacheWithId<2>(builder, test::SimpleCacheViewMode::Merkle_Root);
			AddSubCacheWithId<4>(builder);
			return builder.build();
		}
	}

	VIEW_DELTA_TEST(StateHashIsNonZeroWhenStateCalculationIsEnabled) {
		// Arrange:
		auto cache = CreateSimpleCatapultCacheForStateHashTests();
		auto view = TTraits::CreateView(cache);

		Hash256 expectedStateHash;
		crypto::Sha3_256_Builder stateHashBuilder;
		stateHashBuilder.update(TTraits::GetMerkleRoot(view.template sub<test::SimpleCacheT<2>>()));
		stateHashBuilder.update(TTraits::GetMerkleRoot(view.template sub<test::SimpleCacheT<6>>()));
		stateHashBuilder.final(expectedStateHash);

		// Act + Assert:
		EXPECT_EQ(expectedStateHash, TTraits::CalculateStateHash(view).StateHash);
	}

	VIEW_DELTA_TEST(SubCacheMerkleRootsAreNotEmptyWhenStateCalculationIsEnabled) {
		// Arrange:
		auto cache = CreateSimpleCatapultCacheForStateHashTests();
		auto view = TTraits::CreateView(cache);

		std::vector<Hash256> expectedSubCacheMerkleRoots{
			TTraits::GetMerkleRoot(view.template sub<test::SimpleCacheT<2>>()),
			TTraits::GetMerkleRoot(view.template sub<test::SimpleCacheT<6>>())
		};

		// Act + Assert:
		EXPECT_EQ(expectedSubCacheMerkleRoots, TTraits::CalculateStateHash(view).SubCacheMerkleRoots);
	}

	namespace {
		void AssertCannotSetWrongNumberOfSubCacheMerkleRoots(uint32_t numHashes) {
			// Arrange:
			auto cache = CreateSimpleCatapultCacheForStateHashTests();
			auto view = cache.createDelta();
			auto hashes = test::GenerateRandomDataVector<Hash256>(numHashes);

			// Act + Assert:
			EXPECT_THROW(view.setSubCacheMerkleRoots(hashes), catapult_invalid_argument);
		}
	}

	TEST(TEST_CLASS, CannotSetTooFewSubCacheMerkleRoots) {
		// Assert:
		AssertCannotSetWrongNumberOfSubCacheMerkleRoots(1);
	}

	TEST(TEST_CLASS, CannotSetTooManySubCacheMerkleHashes) {
		// Assert:
		AssertCannotSetWrongNumberOfSubCacheMerkleRoots(3);
	}

	TEST(TEST_CLASS, CanSetExactNumberSubCacheMerkleHashes) {
		// Arrange:
		auto cache = CreateSimpleCatapultCacheForStateHashTests();
		auto view = cache.createDelta();
		auto hashes = test::GenerateRandomDataVector<Hash256>(2);

		// Act:
		view.setSubCacheMerkleRoots(hashes);

		// Assert:
		const auto& subCacheMerkleRoots = view.calculateStateHash(Height(123)).SubCacheMerkleRoots;
		EXPECT_EQ(2u, subCacheMerkleRoots.size());

		// - adjust expected hashes because SimpleCache updateMerkleRoot changes the first byte of the merkle root
		for (auto& hash : hashes)
			hash[0] = 123u;

		EXPECT_EQ(hashes, subCacheMerkleRoots);
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
				for (auto i = 1u; i <= 9; ++i)
					IncrementAllSubCaches(delta);

				cache.commit(Height());
			}

			// Act: save all data
			for (const auto& pStorage : const_cast<const CatapultCache&>(cache).storages()) {
				std::vector<uint8_t> buffer;
				mocks::MockMemoryStream stream("", buffer);
				pStorage->saveAll(stream);
				serializedSubCaches.push_back(buffer);
			}
		}

		// - load all data
		auto i = 0u;
		auto cache = CreateSimpleCatapultCache();
		for (const auto& pStorage : cache.storages()) {
			mocks::MockMemoryStream stream("", serializedSubCaches[i++]);
			pStorage->loadAll(stream, 5);
		}

		// Assert: the cache data was loaded successfully
		auto view = cache.createView();
		AssertSubCacheSizes(view, 9);
	}

	namespace {
		CatapultCache CreateSimpleCatapultCacheWithSomeNonIterableSubCaches() {
			CatapultCacheBuilder builder;
			AddSubCacheWithId<2>(builder);
			AddSubCacheWithId<4>(builder, test::SimpleCacheViewMode::Basic);
			AddSubCacheWithId<6>(builder);
			return builder.build();
		}
	}

	TEST(TEST_CLASS, CanRoundTripCacheViaStoragesWhenSomeSubCachesDoNotSupportStorage) {
		// Arrange: seed the cache with 9 items per subcache
		std::vector<std::vector<uint8_t>> serializedSubCaches;
		{
			// - configure only 2/3 caches to support storage
			auto cache = CreateSimpleCatapultCacheWithSomeNonIterableSubCaches();
			{
				auto delta = cache.createDelta();
				for (auto i = 1u; i <= 9; ++i)
					IncrementAllSubCaches(delta);

				cache.commit(Height());
			}

			// Act: save all data
			for (const auto& pStorage : const_cast<const CatapultCache&>(cache).storages()) {
				std::vector<uint8_t> buffer;
				mocks::MockMemoryStream stream("", buffer);
				pStorage->saveAll(stream);
				serializedSubCaches.push_back(buffer);
			}
		}

		// - load all data
		auto i = 0u;
		auto cache = CreateSimpleCatapultCacheWithSomeNonIterableSubCaches();
		for (const auto& pStorage : cache.storages()) {
			mocks::MockMemoryStream stream("", serializedSubCaches[i++]);
			pStorage->loadAll(stream, 5);
		}

		// Assert: the cache data was loaded successfully for all caches that support storage
		auto view = cache.createView();
		EXPECT_EQ(9u, view.template sub<test::SimpleCacheT<2>>().size());
		EXPECT_EQ(0u, view.template sub<test::SimpleCacheT<4>>().size());
		EXPECT_EQ(9u, view.template sub<test::SimpleCacheT<6>>().size());
	}

	// endregion

	// region general cache synchronization tests

	namespace {
		class CatapultCacheProxy {
		public:
			CatapultCacheProxy() : m_cache(CreateSimpleCatapultCache())
			{}

		public:
			auto createView() const {
				return ViewProxy<CatapultCacheView, test::SimpleCacheView>(m_cache.createView());
			}

			auto createDelta() {
				return ViewProxy<CatapultCacheDelta, test::SimpleCacheDelta>(m_cache.createDelta());
			}

			auto createDetachedDelta() const {
				return DetachedDeltaProxy(m_cache.createDetachableDelta().detach());
			}

			void commit() {
				m_cache.commit(Height());
			}

		private:
			template<typename TView, typename TRawView>
			class ViewProxy {
			public:
				ViewProxy()
						: m_view({})
						, m_isValid(false)
				{}

				explicit ViewProxy(TView&& view)
						: m_view(std::move(view))
						, m_isValid(true)
				{}

			public:
				const auto& operator*() const {
					return static_cast<const TRawView&>(m_view.template sub<test::SimpleCacheT<4>>());
				}

				auto* operator->() {
					return &static_cast<TRawView&>(m_view.template sub<test::SimpleCacheT<4>>());
				}

				explicit operator bool() const {
					return m_isValid;
				}

			private:
				TView m_view;
				bool m_isValid;
			};

			class DetachedDeltaProxy {
			public:
				explicit DetachedDeltaProxy(CatapultCacheDetachedDelta&& detachedDelta) : m_detachedDelta(std::move(detachedDelta))
				{}

			public:
				auto lock() {
					auto pLockedDelta = m_detachedDelta.lock();
					return pLockedDelta
							? ViewProxy<CatapultCacheDelta, test::SimpleCacheDelta>(std::move(*pLockedDelta))
							: ViewProxy<CatapultCacheDelta, test::SimpleCacheDelta>();
				}

			private:
				CatapultCacheDetachedDelta m_detachedDelta;
			};

		private:
			CatapultCache m_cache;
		};

		struct CatapultCacheTraits {
		public:
			using CacheType = CatapultCacheProxy;

		public:
			static size_t MakeId(uint8_t id) {
				return id;
			}

			static size_t CreateWithId(uint8_t id) {
				return id;
			}
		};
	}

	DEFINE_CACHE_SYNC_TESTS(CatapultCacheTraits,)

	// endregion
}}
