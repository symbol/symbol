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

		static ValueType CreateWithIdAndExpiration(uint8_t id, Height height, state::LockStatus status = state::LockStatus::Unused) {
			auto lockInfo = CreateWithId(id);
			lockInfo.Height = height;
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
		using LockInfoPointers = std::vector<const typename TLockInfoTraits::ValueType*>;
		using KeySet = typename std::unordered_set<
			typename TLockInfoTraits::KeyType,
			utils::ArrayHasher<typename TLockInfoTraits::KeyType>>;
		using KeyVector = typename std::vector<typename TLockInfoTraits::KeyType>;

		static constexpr size_t NumDefaultEntries() { return 10; }

		static void PopulateCache(
				typename TLockInfoTraits::CacheType& cache,
				const std::vector<typename TLockInfoTraits::ValueType>& lockInfos) {
			auto delta = cache.createDelta();
			for (const auto& lockInfo : lockInfos)
				delta->insert(lockInfo);

			cache.commit();
		}

		static KeyVector CollectUnusedExpiredLockKeys(typename CacheTraits::CacheType::CacheDeltaType& delta, Height height) {
			KeyVector keys;
			delta.processUnusedExpiredLocks(height, [&keys](const auto& lockInfo) {
				keys.push_back(TLockInfoTraits::ToKey(lockInfo));
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
		static void AssertOnlyUnusedValuesAreTouched() {
			// Arrange:
			typename CacheTraits::CacheType cache;
			auto delta = cache.createDelta();
			delta->insert(CacheTraits::CreateWithIdAndExpiration(11, Height(100)));
			delta->insert(CacheTraits::CreateWithIdAndExpiration(22, Height(200)));
			delta->insert(CacheTraits::CreateWithIdAndExpiration(33, Height(100)));
			delta->insert(CacheTraits::CreateWithIdAndExpiration(44, Height(200), state::LockStatus::Used));
			delta->insert(CacheTraits::CreateWithIdAndExpiration(55, Height(400)));
			delta->insert(CacheTraits::CreateWithIdAndExpiration(66, Height(200)));
			cache.commit();

			// Act: touch at a height with known identifiers
			auto expiryIds = delta->touch(Height(200));

			// Assert: three touched elements
			EXPECT_EQ(3u, delta->modifiedElements().size());
			EXPECT_EQ(std::unordered_set<uint8_t>({ 22, 44, 66 }), GetModifiedIds(*delta));

			// - 44 is not returned because it is already used
			EXPECT_EQ(2u, expiryIds.size());
			EXPECT_CONTAINS(expiryIds, CacheTraits::MakeId(22));
			EXPECT_CONTAINS(expiryIds, CacheTraits::MakeId(66));
		}

	public:
		static void AssertProcessUnusedExpiredLocksForwardsEmptyVectorIfNoLockExpired() {
			// Arrange:
			typename CacheTraits::CacheType cache;
			auto lockInfos = test::CreateLockInfos<TLockInfoTraits>(NumDefaultEntries());
			PopulateCache(cache, lockInfos);

			// Sanity:
			EXPECT_EQ(NumDefaultEntries(), cache.createView()->size());

			// Act: no lock expired at height 15
			auto delta = cache.createDelta();
			auto expiredLockInfoKeys = CollectUnusedExpiredLockKeys(*delta, Height(15));

			// Assert:
			EXPECT_TRUE(expiredLockInfoKeys.empty());
		}

		static void AssertProcessUnusedExpiredLocksForwardsEmptyVectorIfOnlyUsedLocksExpired() {
			// Arrange:
			typename CacheTraits::CacheType cache;
			auto lockInfos = test::CreateLockInfos<TLockInfoTraits>(NumDefaultEntries());
			PopulateCache(cache, lockInfos);

			// Sanity:
			EXPECT_EQ(NumDefaultEntries(), cache.createView()->size());

			// Act: locks at height 10, 30, ..., 90 are used
			auto delta = cache.createDelta();
			auto expiredLockInfoKeys = CollectUnusedExpiredLockKeys(*delta, Height(30));

			// Assert:
			EXPECT_TRUE(expiredLockInfoKeys.empty());
		}

	private:
		static KeySet CollectKeys(const std::vector<const typename TLockInfoTraits::ValueType*>& lockInfos) {
			KeySet keys;
			for (const auto* pLockInfo : lockInfos)
				keys.insert(TLockInfoTraits::ToKey(*pLockInfo));

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
			auto lockInfos = test::CreateLockInfos<TLockInfoTraits>(NumDefaultEntries());
			PopulateCache(cache, lockInfos);

			// Sanity:
			EXPECT_EQ(NumDefaultEntries(), cache.createView()->size());

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
			auto expiredLockInfoKeys = CollectUnusedExpiredLockKeys(*delta, Height(40));

			// Assert:
			LockInfoPointers expectedLockInfos{ &lockInfos[3], &lockInfos[NumDefaultEntries()], &lockInfos[NumDefaultEntries() + 1] };
			AssertEqualLockInfos(expectedLockInfos, expiredLockInfoKeys);
		}

		static void AssertProcessUnusedExpiredLocksForwardsOnlyUnusedExpiredLocks_MultipleLocks() {
			// Arrange:
			typename CacheTraits::CacheType cache;
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
			auto expiredLockInfoKeys = CollectUnusedExpiredLockKeys(*delta, Height(40));

			// Assert:
			LockInfoPointers expectedLockInfos{ &lockInfos[3], &lockInfos[NumDefaultEntries()], &lockInfos[NumDefaultEntries() + 2] };
			AssertEqualLockInfos(expectedLockInfos, expiredLockInfoKeys);
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
	MAKE_LOCK_INFO_CACHE_TEST(TRAITS::LockInfoTraits, OnlyUnusedValuesAreTouched) \
	MAKE_LOCK_INFO_CACHE_TEST(TRAITS::LockInfoTraits, ProcessUnusedExpiredLocksForwardsEmptyVectorIfNoLockExpired) \
	MAKE_LOCK_INFO_CACHE_TEST(TRAITS::LockInfoTraits, ProcessUnusedExpiredLocksForwardsEmptyVectorIfOnlyUsedLocksExpired) \
	MAKE_LOCK_INFO_CACHE_TEST(TRAITS::LockInfoTraits, ProcessUnusedExpiredLocksForwardsUnusedExpiredLocks_SingleLock) \
	MAKE_LOCK_INFO_CACHE_TEST(TRAITS::LockInfoTraits, ProcessUnusedExpiredLocksForwardsUnusedExpiredLocks_MultipleLocks) \
	MAKE_LOCK_INFO_CACHE_TEST(TRAITS::LockInfoTraits, ProcessUnusedExpiredLocksForwardsOnlyUnusedExpiredLocks_MultipleLocks)
