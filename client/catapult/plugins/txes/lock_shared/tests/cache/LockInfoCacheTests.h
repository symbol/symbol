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
#include "plugins/txes/lock_shared/tests/test/LockInfoCacheTestUtils.h"
#include "tests/test/cache/CacheBasicTests.h"
#include "tests/test/cache/CacheMixinsTests.h"
#include "tests/test/cache/CachePruneTests.h"
#include "tests/test/cache/DeltaElementsMixinTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

	// region DeltaElementsMixinTraits

	template<typename TLockInfoTraits>
	struct LockInfoCacheDeltaModificationPolicy : public test:: DeltaInsertModificationPolicy {
		template<typename TDelta, typename TValue>
		static void Modify(TDelta& delta, const TValue& value) {
			delta.find(GetLockIdentifier(value)).get().back().Status = state::LockStatus::Used;
		}
	};

	template<typename TLockInfoTraits>
	struct LockInfoCacheDeltaElementsMixinTraits {
		class CacheType : public TLockInfoTraits::CacheType {
		public:
			CacheType() : TLockInfoTraits::CacheType(CacheConfiguration())
			{}
		};

		using LockInfoTraits = TLockInfoTraits;
		using IdType = typename LockInfoTraits::KeyType;
		using ValueType = typename LockInfoTraits::ValueType;

		static uint8_t GetRawId(const IdType& id) {
			return id[0];
		}

		static IdType GetId(const ValueType& value) {
			return TLockInfoTraits::ToKey(value);
		}

		static IdType MakeId(uint8_t id) {
			return { { id } };
		}

		static auto CreateWithId(uint8_t id) {
			auto lockInfo = TLockInfoTraits::CreateLockInfo(Height(1));
			TLockInfoTraits::SetKey(lockInfo, MakeId(id));
			return lockInfo;
		}

		static auto CreateWithIdAndExpiration(uint8_t id, Height height, state::LockStatus status = state::LockStatus::Unused) {
			auto lockInfo = CreateWithId(id);
			lockInfo.EndHeight = height;
			lockInfo.Status = status;
			return lockInfo;
		}
	};

	// endregion

	/// Lock info cache test suite.
	template<typename TLockInfoTraits>
	class LockInfoCacheTests {
	private:
		using CacheTraits = LockInfoCacheDeltaElementsMixinTraits<TLockInfoTraits>;
		using LockInfoPointers = std::vector<const typename TLockInfoTraits::LockInfoType*>;
		using KeySet = typename std::unordered_set<
			typename TLockInfoTraits::KeyType,
			utils::ArrayHasher<typename TLockInfoTraits::KeyType>>;
		using KeyVector = typename std::vector<typename TLockInfoTraits::KeyType>;

		static constexpr size_t Num_Default_Entries = 10;

		static void PopulateCache(
				typename TLockInfoTraits::CacheType& cache,
				const std::vector<typename TLockInfoTraits::LockInfoType>& lockInfos) {
			auto delta = cache.createDelta();
			for (const auto& lockInfo : lockInfos)
				delta->insert(lockInfo);

			cache.commit();
		}

		static KeyVector CollectUnusedExpiredLockKeys(typename CacheTraits::CacheType::CacheDeltaType& delta, Height height) {
			KeyVector keys;
			delta.processUnusedExpiredLocks(height, [&keys](const auto& lockInfo) {
				keys.push_back(GetLockIdentifier(lockInfo));
			});
			return keys;
		}

		static std::unordered_set<uint8_t> GetModifiedIds(const typename CacheTraits::CacheType::CacheDeltaType& delta) {
			std::unordered_set<uint8_t> modifiedIds;
			for (const auto* pElement : delta.modifiedElements())
				modifiedIds.insert(CacheTraits::GetRawId(CacheTraits::GetId(*pElement)));

			return modifiedIds;
		}

	public:
		// region insert and remove

		static void AssertCanInsertExistingValueIntoCache() {
			// Arrange:
			auto lockIdentifier = CacheTraits::MakeId(20);
			typename CacheTraits::CacheType cache;

			// Act: add three lock histories with a total of four locks
			std::vector<typename TLockInfoTraits::LockInfoType> lockInfos;
			lockInfos.push_back(CacheTraits::CreateWithIdAndExpiration(20, Height(40)));
			lockInfos.push_back(CacheTraits::CreateWithIdAndExpiration(30, Height(40)));
			lockInfos.push_back(CacheTraits::CreateWithIdAndExpiration(40, Height(50)));
			lockInfos.push_back(CacheTraits::CreateWithIdAndExpiration(20, Height(50)));
			PopulateCache(cache, lockInfos);

			// Assert:
			auto view = cache.createView();
			EXPECT_EQ(3u, view->size());
			EXPECT_TRUE(view->contains(lockIdentifier));

			auto iter = view->find(lockIdentifier);
			ASSERT_EQ(2u, iter.get().historyDepth());
			EXPECT_EQ(Height(40), iter.get().begin()->EndHeight);
			EXPECT_EQ(Height(50), (++iter.get().begin())->EndHeight);
		}

		static void AssertCanRemoveHistoryLevelFromCache() {
			// Arrange:
			auto lockIdentifier = CacheTraits::MakeId(20);
			typename CacheTraits::CacheType cache;

			// - add three lock histories with a total of four locks
			std::vector<typename TLockInfoTraits::LockInfoType> lockInfos;
			lockInfos.push_back(CacheTraits::CreateWithIdAndExpiration(20, Height(40)));
			lockInfos.push_back(CacheTraits::CreateWithIdAndExpiration(30, Height(40)));
			lockInfos.push_back(CacheTraits::CreateWithIdAndExpiration(40, Height(50)));
			lockInfos.push_back(CacheTraits::CreateWithIdAndExpiration(20, Height(50)));
			PopulateCache(cache, lockInfos);

			// Sanity:
			{
				auto view = cache.createView();
				EXPECT_EQ(3u, view->size());
				EXPECT_TRUE(view->contains(lockIdentifier));
				EXPECT_EQ(2u, view->find(lockIdentifier).get().historyDepth());
			}

			// Act:
			{
				auto delta = cache.createDelta();
				delta->remove(lockIdentifier);
				cache.commit();
			}

			// Assert:
			auto view = cache.createView();
			EXPECT_EQ(3u, view->size());
			EXPECT_TRUE(view->contains(lockIdentifier));

			auto iter = view->find(lockIdentifier);
			ASSERT_EQ(1u, iter.get().historyDepth());
			EXPECT_EQ(Height(40), iter.get().begin()->EndHeight);
		}

		static void AssertRemoveRemovesLockFromHeightBasedMap() {
			// Arrange:
			typename CacheTraits::CacheType cache;
			auto lockInfos = test::CreateLockInfos<TLockInfoTraits>(Num_Default_Entries);
			PopulateCache(cache, lockInfos);

			// Sanity:
			EXPECT_EQ(Num_Default_Entries, cache.createView()->size());
			EXPECT_TRUE(cache.createView()->contains(GetLockIdentifier(lockInfos[1])));

			// Act: remove unused lock at height 20
			{
				auto delta = cache.createDelta();
				delta->remove(GetLockIdentifier(lockInfos[1]));
				cache.commit();
			}

			// Sanity:
			EXPECT_FALSE(cache.createView()->contains(GetLockIdentifier(lockInfos[1])));

			// - reinsert lock at height 25
			lockInfos[1].EndHeight = Height(25);
			auto delta = cache.createDelta();
			delta->insert(lockInfos[1]);
			cache.commit();

			auto expiredLockInfoKeys1 = CollectUnusedExpiredLockKeys(*delta, Height(20));
			auto expiredLockInfoKeys2 = CollectUnusedExpiredLockKeys(*delta, Height(25));

			// Assert:
			EXPECT_TRUE(expiredLockInfoKeys1.empty());
			AssertEqualLockInfos({ &lockInfos[1] }, expiredLockInfoKeys2);
		}

		// endregion

	public:
		// region processUnusedExpiredLocks

		static void AssertProcessUnusedExpiredLocksForwardsEmptyVectorWhenNoLockExpired() {
			// Arrange:
			typename CacheTraits::CacheType cache;
			auto lockInfos = test::CreateLockInfos<TLockInfoTraits>(Num_Default_Entries);
			PopulateCache(cache, lockInfos);

			// Sanity:
			EXPECT_EQ(Num_Default_Entries, cache.createView()->size());

			// Act: no lock expired at height 15
			auto delta = cache.createDelta();
			auto expiredLockInfoKeys = CollectUnusedExpiredLockKeys(*delta, Height(15));

			// Assert:
			EXPECT_TRUE(expiredLockInfoKeys.empty());
		}

		static void AssertProcessUnusedExpiredLocksForwardsEmptyVectorWhenOnlyUsedLocksExpired() {
			// Arrange:
			typename CacheTraits::CacheType cache;
			auto lockInfos = test::CreateLockInfos<TLockInfoTraits>(Num_Default_Entries);
			PopulateCache(cache, lockInfos);

			// Sanity:
			EXPECT_EQ(Num_Default_Entries, cache.createView()->size());

			// Act: locks at height 10, 30, ..., 90 are used
			auto delta = cache.createDelta();
			auto expiredLockInfoKeys = CollectUnusedExpiredLockKeys(*delta, Height(30));

			// Assert:
			EXPECT_TRUE(expiredLockInfoKeys.empty());
		}

	private:
		static KeySet CollectKeys(const std::vector<const typename TLockInfoTraits::LockInfoType*>& lockInfos) {
			KeySet keys;
			for (const auto* pLockInfo : lockInfos)
				keys.insert(GetLockIdentifier(*pLockInfo));

			return keys;
		}

		static void AssertEqualLockInfos(const LockInfoPointers& expectedLockInfos, const KeyVector& expiredLockInfoKeys) {
			ASSERT_EQ(expectedLockInfos.size(), expiredLockInfoKeys.size());

			auto keySet = CollectKeys(expectedLockInfos);
			for (auto i = 0u; i < expiredLockInfoKeys.size(); ++i) {
				auto message = "at index " + std::to_string(i);
				EXPECT_CONTAINS_MESSAGE(keySet, expiredLockInfoKeys[i], message);
			}
		}

	public:
		static void AssertProcessUnusedExpiredLocksForwardsUnusedExpiredLocks_SingleLock() {
			// Arrange:
			typename CacheTraits::CacheType cache;
			auto lockInfos = test::CreateLockInfos<TLockInfoTraits>(Num_Default_Entries);
			PopulateCache(cache, lockInfos);

			// Sanity:
			EXPECT_EQ(Num_Default_Entries, cache.createView()->size());

			// Act: locks at height 20, 40, ..., 100 are unused
			auto delta = cache.createDelta();
			auto expiredLockInfoKeys = CollectUnusedExpiredLockKeys(*delta, Height(40));

			// Assert:
			LockInfoPointers expectedLockInfos{ &lockInfos[3] };
			AssertEqualLockInfos(expectedLockInfos, expiredLockInfoKeys);
		}

		static void AssertProcessUnusedExpiredLocksForwardsUnusedExpiredLocks_MultipleLocks() {
			// Arrange:
			typename CacheTraits::CacheType cache;
			auto lockInfos = test::CreateLockInfos<TLockInfoTraits>(Num_Default_Entries);
			PopulateCache(cache, lockInfos);
			{
				// add another two lock infos that expire at height 40
				auto delta = cache.createDelta();
				for (auto i = 0u; i < 2; ++i) {
					lockInfos.push_back(TLockInfoTraits::CreateLockInfo(Height(40)));
					delta->insert(lockInfos.back());
				}

				cache.commit();
			}

			// Sanity:
			EXPECT_EQ(Num_Default_Entries + 2, cache.createView()->size());

			// Act:
			auto delta = cache.createDelta();
			auto expiredLockInfoKeys = CollectUnusedExpiredLockKeys(*delta, Height(40));

			// Assert:
			LockInfoPointers expectedLockInfos{ &lockInfos[3], &lockInfos[Num_Default_Entries], &lockInfos[Num_Default_Entries + 1] };
			AssertEqualLockInfos(expectedLockInfos, expiredLockInfoKeys);
		}

		static void AssertProcessUnusedExpiredLocksForwardsOnlyUnusedExpiredLocks_MultipleLocks() {
			// Arrange:
			typename CacheTraits::CacheType cache;
			auto lockInfos = test::CreateLockInfos<TLockInfoTraits>(Num_Default_Entries);
			PopulateCache(cache, lockInfos);
			{
				// add another four lock infos that expire at height 40, two of which are used
				auto delta = cache.createDelta();
				for (auto i = 0u; i < 4; ++i) {
					auto lockInfo = TLockInfoTraits::CreateLockInfo(Height(40));
					if (1 == i % 2)
						lockInfo.Status = state::LockStatus::Used;

					lockInfos.push_back(lockInfo);
					delta->insert(lockInfos.back());
				}

				cache.commit();
			}

			// Sanity:
			EXPECT_EQ(Num_Default_Entries + 4, cache.createView()->size());

			// Act:
			auto delta = cache.createDelta();
			auto expiredLockInfoKeys = CollectUnusedExpiredLockKeys(*delta, Height(40));

			// Assert:
			LockInfoPointers expectedLockInfos{ &lockInfos[3], &lockInfos[Num_Default_Entries], &lockInfos[Num_Default_Entries + 2] };
			AssertEqualLockInfos(expectedLockInfos, expiredLockInfoKeys);
		}

		static void AssertProcessUnusedExpiredLocksIsHistoryAware() {
			// Arrange:
			typename CacheTraits::CacheType cache;

			// - add three lock histories with a total of four locks
			std::vector<typename TLockInfoTraits::LockInfoType> lockInfos;
			lockInfos.push_back(CacheTraits::CreateWithIdAndExpiration(20, Height(40)));
			lockInfos.push_back(CacheTraits::CreateWithIdAndExpiration(30, Height(40)));
			lockInfos.push_back(CacheTraits::CreateWithIdAndExpiration(40, Height(50)));
			lockInfos.push_back(CacheTraits::CreateWithIdAndExpiration(20, Height(50)));
			PopulateCache(cache, lockInfos);

			// Sanity:
			EXPECT_EQ(3u, cache.createView()->size());

			// Act:
			auto delta = cache.createDelta();
			auto expiredLockInfoKeysAt40 = CollectUnusedExpiredLockKeys(*delta, Height(40));
			auto expiredLockInfoKeysAt50 = CollectUnusedExpiredLockKeys(*delta, Height(50));

			// Assert: lock info with id 20 is returned at both heights
			AssertEqualLockInfos({ &lockInfos[0], &lockInfos[1] }, expiredLockInfoKeysAt40);
			AssertEqualLockInfos({ &lockInfos[2], &lockInfos[3] }, expiredLockInfoKeysAt50);
		}

		// endregion

	public:
		// region prune

		static void AssertPruneDoesNotRemoveLockHistoriesWithSomeExpiredAndSomeUnexpiredLockInfos() {
			// Arrange:
			typename CacheTraits::CacheType cache;

			// Act: add two lock histories with a total of four locks
			std::vector<typename TLockInfoTraits::LockInfoType> lockInfos;
			lockInfos.push_back(CacheTraits::CreateWithIdAndExpiration(20, Height(40)));
			lockInfos.push_back(CacheTraits::CreateWithIdAndExpiration(30, Height(40)));
			lockInfos.push_back(CacheTraits::CreateWithIdAndExpiration(20, Height(60)));
			lockInfos.push_back(CacheTraits::CreateWithIdAndExpiration(20, Height(80)));
			PopulateCache(cache, lockInfos);

			// Sanity:
			{
				auto view = cache.createView();
				EXPECT_EQ(2u, view->size());
				EXPECT_TRUE(view->contains(CacheTraits::MakeId(20)));
				EXPECT_TRUE(view->contains(CacheTraits::MakeId(30)));
			}

			// Act: prune locks expiring at height 40
			{
				auto delta = cache.createDelta();
				delta->prune(Height(40));
				cache.commit();
			}

			// Assert: only lock histories with all lock infos ending at height 40 are pruned
			auto view = cache.createView();
			EXPECT_EQ(1u, view->size());
			EXPECT_TRUE(view->contains(CacheTraits::MakeId(20)));
			EXPECT_FALSE(view->contains(CacheTraits::MakeId(30)));

			auto iter = view->find(CacheTraits::MakeId(20));
			ASSERT_EQ(2u, iter.get().historyDepth());
			EXPECT_EQ(Height(60), iter.get().begin()->EndHeight);
			EXPECT_EQ(Height(80), (++iter.get().begin())->EndHeight);
		}

		// endregion
	};
}}

#define MAKE_LOCK_INFO_CACHE_TEST(TRAITS_NAME, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { LockInfoCacheTests<TRAITS_NAME>::Assert##TEST_NAME(); }

#define DEFINE_LOCK_INFO_CACHE_TESTS(TRAITS, MODIFICATION_TRAITS, SUFFIX) \
	DEFINE_CACHE_CONTAINS_TESTS(TRAITS, ViewAccessor, _View##SUFFIX) \
	DEFINE_CACHE_CONTAINS_TESTS(TRAITS, DeltaAccessor, _Delta##SUFFIX) \
	\
	DEFINE_CACHE_ITERATION_TESTS(TRAITS, ViewAccessor, _View##SUFFIX) \
	\
	DEFINE_CACHE_ACCESSOR_TESTS(TRAITS, ViewAccessor, MutableAccessor, _ViewMutable##SUFFIX) \
	DEFINE_CACHE_ACCESSOR_TESTS(TRAITS, ViewAccessor, ConstAccessor, _ViewConst##SUFFIX) \
	DEFINE_CACHE_ACCESSOR_TESTS(TRAITS, DeltaAccessor, MutableAccessor, _DeltaMutable##SUFFIX) \
	DEFINE_CACHE_ACCESSOR_TESTS(TRAITS, DeltaAccessor, ConstAccessor, _DeltaConst##SUFFIX) \
	\
	DEFINE_CACHE_MUTATION_TESTS_SKIP_STRICT_INSERT(TRAITS, DeltaAccessor, _Delta##SUFFIX) \
	\
	DEFINE_ACTIVE_PREDICATE_TESTS(TRAITS, ViewAccessor, _View##SUFFIX) \
	DEFINE_ACTIVE_PREDICATE_TESTS(TRAITS, DeltaAccessor, _Delta##SUFFIX) \
	\
	DEFINE_DELTA_ELEMENTS_MIXIN_CUSTOM_TESTS(TRAITS, MODIFICATION_TRAITS, _Delta##SUFFIX) \
	\
	DEFINE_CACHE_BASIC_TESTS(TRAITS, SUFFIX) \
	\
	MAKE_LOCK_INFO_CACHE_TEST(TRAITS::LockInfoTraits, CanInsertExistingValueIntoCache) \
	MAKE_LOCK_INFO_CACHE_TEST(TRAITS::LockInfoTraits, CanRemoveHistoryLevelFromCache) \
	MAKE_LOCK_INFO_CACHE_TEST(TRAITS::LockInfoTraits, RemoveRemovesLockFromHeightBasedMap) \
	\
	MAKE_LOCK_INFO_CACHE_TEST(TRAITS::LockInfoTraits, ProcessUnusedExpiredLocksForwardsEmptyVectorWhenNoLockExpired) \
	MAKE_LOCK_INFO_CACHE_TEST(TRAITS::LockInfoTraits, ProcessUnusedExpiredLocksForwardsEmptyVectorWhenOnlyUsedLocksExpired) \
	MAKE_LOCK_INFO_CACHE_TEST(TRAITS::LockInfoTraits, ProcessUnusedExpiredLocksForwardsUnusedExpiredLocks_SingleLock) \
	MAKE_LOCK_INFO_CACHE_TEST(TRAITS::LockInfoTraits, ProcessUnusedExpiredLocksForwardsUnusedExpiredLocks_MultipleLocks) \
	MAKE_LOCK_INFO_CACHE_TEST(TRAITS::LockInfoTraits, ProcessUnusedExpiredLocksForwardsOnlyUnusedExpiredLocks_MultipleLocks) \
	MAKE_LOCK_INFO_CACHE_TEST(TRAITS::LockInfoTraits, ProcessUnusedExpiredLocksIsHistoryAware) \
	\
	MAKE_LOCK_INFO_CACHE_TEST(TRAITS::LockInfoTraits, PruneDoesNotRemoveLockHistoriesWithSomeExpiredAndSomeUnexpiredLockInfos)
