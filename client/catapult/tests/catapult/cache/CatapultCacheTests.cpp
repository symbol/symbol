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
#include "catapult/state/CatapultState.h"
#include "tests/test/cache/CacheBasicTests.h"
#include "tests/test/cache/SimpleCache.h"
#include "tests/test/core/StateTestUtils.h"
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
		EXPECT_EQ(Hash256(), TTraits::CalculateStateHash(view).StateHash);
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
			AddSubCacheWithId<10>(builder, test::SimpleCacheViewMode::Merkle_Root);
			return builder.build();
		}
	}

	VIEW_DELTA_TEST(StateHashIsNonzeroWhenStateCalculationIsEnabled) {
		// Arrange:
		auto cache = CreateSimpleCatapultCacheForStateHashTests();
		auto view = TTraits::CreateView(cache);

		Hash256 expectedStateHash;
		crypto::Sha3_256_Builder stateHashBuilder;
		stateHashBuilder.update(TTraits::GetMerkleRoot(view.template sub<test::SimpleCacheT<2>>()));
		stateHashBuilder.update(TTraits::GetMerkleRoot(view.template sub<test::SimpleCacheT<6>>()));
		stateHashBuilder.update(TTraits::GetMerkleRoot(view.template sub<test::SimpleCacheT<10>>()));
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
			TTraits::GetMerkleRoot(view.template sub<test::SimpleCacheT<6>>()),
			TTraits::GetMerkleRoot(view.template sub<test::SimpleCacheT<10>>())
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
		AssertCannotSetWrongNumberOfSubCacheMerkleRoots(2);
	}

	TEST(TEST_CLASS, CannotSetTooManySubCacheMerkleHashes) {
		AssertCannotSetWrongNumberOfSubCacheMerkleRoots(4);
	}

	TEST(TEST_CLASS, CanSetExactNumberSubCacheMerkleHashes) {
		// Arrange:
		auto cache = CreateSimpleCatapultCacheForStateHashTests();
		auto view = cache.createDelta();
		auto hashes = test::GenerateRandomDataVector<Hash256>(3);

		// Act:
		view.setSubCacheMerkleRoots(hashes);

		// Assert:
		const auto& subCacheMerkleRoots = view.calculateStateHash(Height(123)).SubCacheMerkleRoots;
		EXPECT_EQ(3u, subCacheMerkleRoots.size());

		// - adjust expected hashes because SimpleCache::updateMerkleRoot changes the first byte of the merkle root
		for (auto& hash : hashes)
			hash[0] = 123;

		EXPECT_EQ(hashes, subCacheMerkleRoots);
	}

	// endregion

	// region prune

	TEST(TEST_CLASS, CanPruneAllSubCachesAtHeight) {
		// Arrange:
		auto cache = CreateSimpleCatapultCacheForStateHashTests();
		auto view = cache.createDelta();
		auto hashes = test::GenerateRandomDataVector<Hash256>(3);
		view.setSubCacheMerkleRoots(hashes);

		// Act:
		view.prune(Height(101));

		// Assert:
		const auto& subCacheMerkleRoots = view.calculateStateHash(Height(123)).SubCacheMerkleRoots;
		EXPECT_EQ(3u, subCacheMerkleRoots.size());

		// - adjust expected hashes because SimpleCache::updateMerkleRoot changes the first byte of the merkle root
		//   and SimpleCache::prune changes the second
		for (auto& hash : hashes) {
			hash[0] = 123;
			hash[1] = 101;
		}

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
		CatapultCacheDetachedDelta cacheDetachedDelta(state::CatapultState(), {});
		{
			// - create a detachable delta and release it (to release read lock it holds)
			auto cacheDetachableDelta = cache.createDetachableDelta();
			cacheDetachedDelta = cacheDetachableDelta.detach();
		}

		{
			// - create a delta and make some changes
			auto delta = cache.createDelta();
			delta.sub<test::SimpleCacheT<4>>().increment();

			// Sanity: detached delta can still be locked (no commits)
			std::thread([&cacheDetachedDelta]() {
				// - need to lock on a separate thread because test thread owns delta
				EXPECT_TRUE(!!cacheDetachedDelta.tryLock());
			}).join();

			// Act:
			cache.commit(Height());
		}

		// Assert:
		EXPECT_FALSE(!!cacheDetachedDelta.tryLock());
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

	// region dependent state

	namespace {
		template<typename TAssertState>
		void AssertDependentState(cache::CatapultCache& cache, TAssertState assertState) {
			assertState(cache.createView().dependentState(), "view");
			assertState(cache.createDelta().dependentState(), "delta");
			assertState(const_cast<const CatapultCacheDelta&&>(cache.createDelta()).dependentState(), "delta (const)");

			auto cacheDetachableDelta = cache.createDetachableDelta();
			assertState(cacheDetachableDelta.detach().tryLock()->dependentState(), "detachable delta");
		}
	}

	TEST(TEST_CLASS, DependentStateIsInitiallyZero) {
		// Arrange:
		auto cache = CreateSimpleCatapultCache();

		// Assert:
		AssertDependentState(cache, [](const auto& state, const auto& message) {
			test::AssertEqual(state::CatapultState(), state, message);
		});
	}

	TEST(TEST_CLASS, DependentStateChangesAreDiscardedWhenDeltaIsNotCommitted) {
		// Arrange:
		auto cache = CreateSimpleCatapultCache();

		// Act: modify dependent state but throw it away
		{
			auto cacheDelta = cache.createDelta();
			cacheDelta.dependentState().NumTotalTransactions = 100;
		}

		// Assert:
		AssertDependentState(cache, [](const auto& state, const auto& message) {
			test::AssertEqual(state::CatapultState(), state, message);
		});
	}

	TEST(TEST_CLASS, DependentStateChangesAreCommittedWhenDeltaIsCommitted) {
		// Arrange:
		auto cache = CreateSimpleCatapultCache();

		// Act: modify and commit dependent state
		{
			auto cacheDelta = cache.createDelta();
			cacheDelta.dependentState().NumTotalTransactions = 100;
			cache.commit(Height(2));
		}

		// Assert:
		AssertDependentState(cache, [](const auto& state, const auto& message) {
			auto expectedState = state::CatapultState();
			expectedState.NumTotalTransactions = 100;
			test::AssertEqual(expectedState, state, message);
		});
	}

	TEST(TEST_CLASS, DependentStateChangesAreReflectedInDeltaAfterCommit) {
		// Arrange:
		auto cache = CreateSimpleCatapultCache();

		// Act: modify and commit dependent state
		auto cacheDelta = cache.createDelta();
		cacheDelta.dependentState().NumTotalTransactions = 100;
		cache.commit(Height(2));

		// Assert:
		auto expectedState = state::CatapultState();
		expectedState.NumTotalTransactions = 100;
		test::AssertEqual(expectedState, cacheDelta.dependentState());
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
		EXPECT_EQ(&view.dependentState(), &readOnlyView.dependentState());
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
		EXPECT_EQ(&delta.dependentState(), &readOnlyView.dependentState());
		AssertSubCacheSizes(readOnlyView, 2);
	}

	// endregion

	// region storages

	TEST(TEST_CLASS, CanRoundtripCacheViaStorages) {
		// Arrange: seed the cache with 9 items per sub cache
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
			auto storages = const_cast<const CatapultCache&>(cache).storages();
			auto view = cache.createView();
			for (const auto& pStorage : storages) {
				std::vector<uint8_t> buffer;
				mocks::MockMemoryStream stream(buffer);
				pStorage->saveAll(view, stream);
				serializedSubCaches.push_back(buffer);
			}
		}

		// - load all data
		auto i = 0u;
		auto cache = CreateSimpleCatapultCache();
		for (const auto& pStorage : cache.storages()) {
			mocks::MockMemoryStream stream(serializedSubCaches[i++]);
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

	TEST(TEST_CLASS, CanRoundtripCacheViaStoragesWhenSomeSubCachesDoNotSupportStorage) {
		// Arrange: seed the cache with 9 items per sub cache
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
			auto storages = const_cast<const CatapultCache&>(cache).storages();
			auto view = cache.createView();
			for (const auto& pStorage : storages) {
				std::vector<uint8_t> buffer;
				mocks::MockMemoryStream stream(buffer);
				pStorage->saveAll(view, stream);
				serializedSubCaches.push_back(buffer);
			}
		}

		// - load all data
		auto i = 0u;
		auto cache = CreateSimpleCatapultCacheWithSomeNonIterableSubCaches();
		for (const auto& pStorage : cache.storages()) {
			mocks::MockMemoryStream stream(serializedSubCaches[i++]);
			pStorage->loadAll(stream, 5);
		}

		// Assert: the cache data was loaded successfully for all caches that support storage
		auto view = cache.createView();
		EXPECT_EQ(9u, view.template sub<test::SimpleCacheT<2>>().size());
		EXPECT_EQ(0u, view.template sub<test::SimpleCacheT<4>>().size());
		EXPECT_EQ(9u, view.template sub<test::SimpleCacheT<6>>().size());
	}

	// endregion

	// region changesStorages

	namespace {
		void AssertEquivalent(
				const MemoryCacheChangesT<uint64_t>& changes,
				const std::unordered_set<uint64_t>& expectedAdded,
				const std::unordered_set<uint64_t>& expectedRemoved,
				const std::unordered_set<uint64_t>& expectedModified,
				const std::string& message) {
			EXPECT_EQ(expectedAdded.size(), changes.Added.size()) << message;
			EXPECT_EQ(expectedRemoved.size(), changes.Removed.size()) << message;
			EXPECT_EQ(expectedModified.size(), changes.Copied.size()) << message;

			EXPECT_EQ(expectedAdded, std::unordered_set<uint64_t>(changes.Added.cbegin(), changes.Added.cend())) << message;
			EXPECT_EQ(expectedRemoved, std::unordered_set<uint64_t>(changes.Removed.cbegin(), changes.Removed.cend())) << message;
			EXPECT_EQ(expectedModified, std::unordered_set<uint64_t>(changes.Copied.cbegin(), changes.Copied.cend())) << message;
		}
	}

	TEST(TEST_CLASS, CanRoundtripCacheChangesViaChangesStorages) {
		// Arrange: seed the cache with custom elements such that: A { 0 }, R { 1, 3 }, M { 2 }
		std::vector<std::vector<uint8_t>> serializedSubCacheChanges;
		{
			auto cache = CreateSimpleCatapultCache();
			auto delta = cache.createDelta();
			delta.sub<test::SimpleCacheT<2>>().setElements({ { 0, 5, 7, 1 } });
			delta.sub<test::SimpleCacheT<4>>().setElements({ { 1, 4, 9, 3 } });
			delta.sub<test::SimpleCacheT<6>>().setElements({ { 2, 3, 8, 6 } });

			// Act: save all data
			for (const auto& pStorage : cache.changesStorages()) {
				std::vector<uint8_t> buffer;
				mocks::MockMemoryStream stream(buffer);
				pStorage->saveAll(CacheChanges(delta), stream);
				serializedSubCacheChanges.push_back(buffer);
			}
		}

		// - load all data
		auto i = 0u;
		std::vector<std::unique_ptr<const MemoryCacheChanges>> loadedChanges;
		auto cache = CreateSimpleCatapultCache();
		for (const auto& pStorage : cache.changesStorages()) {
			mocks::MockMemoryStream stream(serializedSubCacheChanges[i++]);
			loadedChanges.push_back(pStorage->loadAll(stream));
		}

		// Assert: the cache data was loaded successfully
		ASSERT_EQ(3u, loadedChanges.size());

		using TypedMemoryCacheChanges = MemoryCacheChangesT<uint64_t>;
		const auto& loadedChanges1 = static_cast<const TypedMemoryCacheChanges&>(*loadedChanges[0]);
		const auto& loadedChanges2 = static_cast<const TypedMemoryCacheChanges&>(*loadedChanges[1]);
		const auto& loadedChanges3 = static_cast<const TypedMemoryCacheChanges&>(*loadedChanges[2]);

		AssertEquivalent(loadedChanges1, { 0 }, { 5, 1 }, { 7 }, "loadedChanges1");
		AssertEquivalent(loadedChanges2, { 1 }, { 4, 3 }, { 9 }, "loadedChanges2");
		AssertEquivalent(loadedChanges3, { 2 }, { 3, 6 }, { 8 }, "loadedChanges3");
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
				auto cacheDetachableDelta = m_cache.createDetachableDelta();
				return DetachedDeltaProxy(cacheDetachableDelta.detach());
			}

			void commit() {
				m_cache.commit(Height());
			}

		private:
			template<typename TView, typename TRawView>
			class ViewProxy {
			public:
				ViewProxy()
						: m_view(m_dependentState, {})
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
				state::CatapultState m_dependentState;
				TView m_view;
				bool m_isValid;
			};

			class DetachedDeltaProxy {
			public:
				explicit DetachedDeltaProxy(CatapultCacheDetachedDelta&& detachedDelta) : m_detachedDelta(std::move(detachedDelta))
				{}

			public:
				auto tryLock() {
					auto pLockedDelta = m_detachedDelta.tryLock();
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
