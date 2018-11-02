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
	struct LockInfoCacheDeltaMarkUsedModificationPolicy {
		template<typename TDelta, typename TValue>
		static void Modify(TDelta& delta, const TValue& value) {
			delta.find(TLockInfoTraits::ToKey(value)).get().Status = state::LockStatus::Used;
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
			return IdType{ { id } };
		}

		static ValueType CreateWithId(uint8_t id) {
			auto lockInfo = TLockInfoTraits::CreateLockInfo(Height(1));
			TLockInfoTraits::SetKey(lockInfo, MakeId(id));
			return lockInfo;
		}

		static ValueType CreateWithIdAndExpiration(uint8_t id, Height height) {
			auto lockInfo = CreateWithId(id);
			lockInfo.Height = height;
			return lockInfo;
		}
	};

	// endregion

	/// Lock info cache test suite.
	template<typename TLockInfoTraits>
	class LockInfoCacheTests {
	private:
		static constexpr size_t NumDefaultEntries() { return 10; }

		static void PopulateCache(
				typename TLockInfoTraits::CacheType& cache,
				const std::vector<typename TLockInfoTraits::ValueType>& lockInfos) {
			auto delta = cache.createDelta();
			for (const auto& lockInfo : lockInfos)
				delta->insert(lockInfo);

			cache.commit();
		}

	public:
		static void AssertCollectUnusedExpiredLocksReturnsEmptyVectorIfNoLockExpired() {
			// Arrange:
			typename LockInfoCacheDeltaElementsMixinTraits<TLockInfoTraits>::CacheType cache;
			auto lockInfos = test::CreateLockInfos<TLockInfoTraits>(NumDefaultEntries());
			PopulateCache(cache, lockInfos);

			// Sanity:
			EXPECT_EQ(NumDefaultEntries(), cache.createView()->size());

			// Act: no lock expired at height 15
			auto delta = cache.createDelta();
			auto expiredLockInfos = delta->collectUnusedExpiredLocks(Height(15));

			// Assert:
			EXPECT_TRUE(expiredLockInfos.empty());
		}

		static void AssertCollectUnusedExpiredLocksReturnsEmptyVectorIfOnlyUsedLocksExpired() {
			// Arrange:
			typename LockInfoCacheDeltaElementsMixinTraits<TLockInfoTraits>::CacheType cache;
			auto lockInfos = test::CreateLockInfos<TLockInfoTraits>(NumDefaultEntries());
			PopulateCache(cache, lockInfos);

			// Sanity:
			EXPECT_EQ(NumDefaultEntries(), cache.createView()->size());

			// Act: locks at height 10, 30, ..., 90 are used
			auto delta = cache.createDelta();
			auto expiredLockInfos = delta->collectUnusedExpiredLocks(Height(30));

			// Assert:
			EXPECT_TRUE(expiredLockInfos.empty());
		}

	private:
		using KeySet = typename std::unordered_set<
			typename TLockInfoTraits::KeyType,
			utils::ArrayHasher<typename TLockInfoTraits::KeyType>>;

		using LockInfoPointers = std::vector<const typename TLockInfoTraits::ValueType*>;

		static KeySet CollectKeys(const std::vector<const typename TLockInfoTraits::ValueType*>& lockInfos) {
			KeySet keys;
			for (const auto* pLockInfo : lockInfos)
				keys.insert(TLockInfoTraits::ToKey(*pLockInfo));

			return keys;
		}

		static void AssertEqualLockInfos(const LockInfoPointers& expectedLockInfos, const LockInfoPointers& expiredLockInfos) {
			ASSERT_EQ(expectedLockInfos.size(), expiredLockInfos.size());

			auto keySet = CollectKeys(expectedLockInfos);
			for (auto i = 0u; i < expiredLockInfos.size(); ++i)
				EXPECT_TRUE(keySet.cend() != keySet.find(TLockInfoTraits::ToKey(*expiredLockInfos[i]))) << "at index " << i;
		}

	public:
		static void AssertCollectUnusedExpiredLocksReturnsUnusedExpiredLocks_SingleLock() {
			// Arrange:
			typename LockInfoCacheDeltaElementsMixinTraits<TLockInfoTraits>::CacheType cache;
			auto lockInfos = test::CreateLockInfos<TLockInfoTraits>(NumDefaultEntries());
			PopulateCache(cache, lockInfos);

			// Sanity:
			EXPECT_EQ(NumDefaultEntries(), cache.createView()->size());

			// Act: locks at height 20, 40, ..., 100 are unused
			auto delta = cache.createDelta();
			auto expiredLockInfos = delta->collectUnusedExpiredLocks(Height(40));

			// Assert:
			LockInfoPointers expectedLockInfos{ &lockInfos[3] };
			AssertEqualLockInfos(expectedLockInfos, expiredLockInfos);
		}

		static void AssertCollectUnusedExpiredLocksReturnsUnusedExpiredLocks_MultipleLocks() {
			// Arrange:
			typename LockInfoCacheDeltaElementsMixinTraits<TLockInfoTraits>::CacheType cache;
			auto lockInfos = test::CreateLockInfos<TLockInfoTraits>(NumDefaultEntries());
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
			EXPECT_EQ(NumDefaultEntries() + 2, cache.createView()->size());

			// Act:
			auto delta = cache.createDelta();
			auto expiredLockInfos = delta->collectUnusedExpiredLocks(Height(40));

			// Assert:
			LockInfoPointers expectedLockInfos{ &lockInfos[3], &lockInfos[NumDefaultEntries()], &lockInfos[NumDefaultEntries() + 1] };
			AssertEqualLockInfos(expectedLockInfos, expiredLockInfos);
		}

		static void AssertCollectUnusedExpiredLocksReturnsOnlyUnusedExpiredLocks_MultipleLocks() {
			// Arrange:
			typename LockInfoCacheDeltaElementsMixinTraits<TLockInfoTraits>::CacheType cache;
			auto lockInfos = test::CreateLockInfos<TLockInfoTraits>(NumDefaultEntries());
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
			EXPECT_EQ(NumDefaultEntries() + 4, cache.createView()->size());

			// Act:
			auto delta = cache.createDelta();
			auto expiredLockInfos = delta->collectUnusedExpiredLocks(Height(40));

			// Assert:
			LockInfoPointers expectedLockInfos{ &lockInfos[3], &lockInfos[NumDefaultEntries()], &lockInfos[NumDefaultEntries() + 2] };
			AssertEqualLockInfos(expectedLockInfos, expiredLockInfos);
		}
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
	DEFINE_CACHE_MUTATION_TESTS(TRAITS, DeltaAccessor, _Delta##SUFFIX) \
	\
	DEFINE_ACTIVE_PREDICATE_TESTS(TRAITS, ViewAccessor, _View##SUFFIX) \
	DEFINE_ACTIVE_PREDICATE_TESTS(TRAITS, DeltaAccessor, _Delta##SUFFIX) \
	\
	DEFINE_CACHE_TOUCH_TESTS(TRAITS, _Delta##SUFFIX) \
	\
	DEFINE_DELTA_ELEMENTS_MIXIN_CUSTOM_TESTS(TRAITS, MODIFICATION_TRAITS, _Delta##SUFFIX) \
	\
	DEFINE_CACHE_BASIC_TESTS(TRAITS, SUFFIX) \
	\
	MAKE_LOCK_INFO_CACHE_TEST(TRAITS::LockInfoTraits, CollectUnusedExpiredLocksReturnsEmptyVectorIfNoLockExpired) \
	MAKE_LOCK_INFO_CACHE_TEST(TRAITS::LockInfoTraits, CollectUnusedExpiredLocksReturnsEmptyVectorIfOnlyUsedLocksExpired) \
	MAKE_LOCK_INFO_CACHE_TEST(TRAITS::LockInfoTraits, CollectUnusedExpiredLocksReturnsUnusedExpiredLocks_SingleLock) \
	MAKE_LOCK_INFO_CACHE_TEST(TRAITS::LockInfoTraits, CollectUnusedExpiredLocksReturnsUnusedExpiredLocks_MultipleLocks) \
	MAKE_LOCK_INFO_CACHE_TEST(TRAITS::LockInfoTraits, CollectUnusedExpiredLocksReturnsOnlyUnusedExpiredLocks_MultipleLocks)
