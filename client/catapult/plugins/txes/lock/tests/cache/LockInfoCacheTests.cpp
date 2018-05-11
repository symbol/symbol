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

#include "src/cache/HashLockInfoCache.h"
#include "src/cache/SecretLockInfoCache.h"
#include "tests/test/LockInfoCacheTestUtils.h"
#include "tests/test/cache/CacheBasicTests.h"
#include "tests/test/cache/CacheMixinsTests.h"
#include "tests/test/cache/CachePruneTests.h"
#include "tests/test/cache/DeltaElementsMixinTests.h"
#include "tests/TestHarness.h"
#include <unordered_set>

namespace catapult { namespace cache {

#define TEST_CLASS LockInfoCacheTests

	namespace {
		struct HashTraits : public test::BasicHashLockInfoTestTraits {
			static void SetKey(ValueType& lockInfo, const KeyType& key) {
				lockInfo.Hash = key;
			}
		};

		struct SecretTraits : public test::BasicSecretLockInfoTestTraits {
			static void SetKey(ValueType& lockInfo, const KeyType& key) {
				lockInfo.Secret = key;
			}
		};
	}

	// region mixin traits based tests

	namespace {
		template<typename TLockInfoTraits>
		struct DeltaMarkUsedModificationPolicy {
			template<typename TDelta, typename TValue>
			static void Modify(TDelta& delta, const TValue& value) {
				delta.get(TLockInfoTraits::ToKey(value)).Status = model::LockStatus::Used;
			}
		};

		template<typename TLockInfoTraits>
		struct DeltaElementsMixinTraits {
			class CacheType : public TLockInfoTraits::CacheType {
			public:
				CacheType() : TLockInfoTraits::CacheType(CacheConfiguration())
				{}
			};

			using IdType = typename TLockInfoTraits::KeyType;
			using ValueType = typename TLockInfoTraits::ValueType;

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
	}

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
	DEFINE_DELTA_ELEMENTS_MIXIN_CUSTOM_TESTS(TRAITS, MODIFICATION_TRAITS, _Delta##SUFFIX) \
	\
	DEFINE_ACTIVE_PREDICATE_TESTS(TRAITS, ViewAccessor, _View##SUFFIX) \
	DEFINE_ACTIVE_PREDICATE_TESTS(TRAITS, DeltaAccessor, _Delta##SUFFIX) \
	\
	DEFINE_CACHE_BASIC_TESTS(TRAITS, SUFFIX)

	DEFINE_LOCK_INFO_CACHE_TESTS(DeltaElementsMixinTraits<HashTraits>, DeltaMarkUsedModificationPolicy<HashTraits>, _Hash)
	DEFINE_LOCK_INFO_CACHE_TESTS(DeltaElementsMixinTraits<SecretTraits>, DeltaMarkUsedModificationPolicy<SecretTraits>, _Secret)

	// endregion

	// *** custom tests ***

	// region prune

	DEFINE_CACHE_PRUNE_TESTS(DeltaElementsMixinTraits<HashTraits>, _Hash)
	DEFINE_CACHE_PRUNE_TESTS(DeltaElementsMixinTraits<SecretTraits>, _Secret)

	// endregion

	namespace {
		constexpr size_t Num_Default_Entries = 10;

		template<typename TLockInfoTraits>
		void PopulateCache(typename TLockInfoTraits::CacheType& cache, const std::vector<typename TLockInfoTraits::ValueType>& lockInfos) {
			auto delta = cache.createDelta();
			for (const auto& lockInfo : lockInfos)
				delta->insert(lockInfo);

			cache.commit();
		}

		template<typename TTraits>
		struct DeltaTraits {
			using ViewType = typename TTraits::CacheDeltaType;
		};
	}

#define LOCK_CACHE_TRAITS_TEST(TEST_CLASS, TEST_NAME, CACHE_TYPE, VALUE_TYPE) \
	TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<VALUE_TYPE##Traits, CACHE_TYPE##Traits<VALUE_TYPE##Traits>>();

#define DELTA_LOCK_TYPE_BASED_TEST(TEST_NAME) \
	template<typename TLockInfoTraits, typename TViewTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Delta_Hash) { LOCK_CACHE_TRAITS_TEST(TEST_CLASS, TEST_NAME, Delta, Hash) } \
	TEST(TEST_CLASS, TEST_NAME##_Delta_Secret) { LOCK_CACHE_TRAITS_TEST(TEST_CLASS, TEST_NAME, Delta, Secret) } \
	template<typename TLockInfoTraits, typename TViewTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region collectUnusedExpiredLocks

	DELTA_LOCK_TYPE_BASED_TEST(CollectUnusedExpiredLocksReturnsEmptyVectorIfNoLockExpired) {
		// Arrange:
		typename DeltaElementsMixinTraits<TLockInfoTraits>::CacheType cache;
		auto lockInfos = test::CreateLockInfos<TLockInfoTraits>(Num_Default_Entries);
		PopulateCache<TLockInfoTraits>(cache, lockInfos);

		// Sanity:
		EXPECT_EQ(Num_Default_Entries, cache.createView()->size());

		// Act: no lock expired at height 15
		auto delta = cache.createDelta();
		auto expiredLockInfos = delta->collectUnusedExpiredLocks(Height(15));

		// Assert:
		EXPECT_TRUE(expiredLockInfos.empty());
	}

	DELTA_LOCK_TYPE_BASED_TEST(CollectUnusedExpiredLocksReturnsEmptyVectorIfOnlyUsedLocksExpired) {
		// Arrange:
		typename DeltaElementsMixinTraits<TLockInfoTraits>::CacheType cache;
		auto lockInfos = test::CreateLockInfos<TLockInfoTraits>(Num_Default_Entries);
		PopulateCache<TLockInfoTraits>(cache, lockInfos);

		// Sanity:
		EXPECT_EQ(Num_Default_Entries, cache.createView()->size());

		// Act: locks at height 10, 30, ..., 90 are used
		auto delta = cache.createDelta();
		auto expiredLockInfos = delta->collectUnusedExpiredLocks(Height(30));

		// Assert:
		EXPECT_TRUE(expiredLockInfos.empty());
	}

	namespace {
		template<typename TLockInfoTraits>
		using KeySet = typename std::unordered_set<
			typename TLockInfoTraits::KeyType,
			utils::ArrayHasher<typename TLockInfoTraits::KeyType>>;

		template<typename TLockInfoTraits>
		using LockInfoPointers = std::vector<const typename TLockInfoTraits::ValueType*>;

		template<typename TLockInfoTraits>
		KeySet<TLockInfoTraits> CollectKeys(const std::vector<const typename TLockInfoTraits::ValueType*>& lockInfos) {
			KeySet<TLockInfoTraits> keys;
			for (const auto* pLockInfo : lockInfos)
				keys.insert(TLockInfoTraits::ToKey(*pLockInfo));

			return keys;
		}

		template<typename TLockInfoTraits>
		void AssertEqualLockInfos(
				const LockInfoPointers<TLockInfoTraits>& expectedLockInfos,
				const LockInfoPointers<TLockInfoTraits>& expiredLockInfos) {
			ASSERT_EQ(expectedLockInfos.size(), expiredLockInfos.size());

			auto keySet = CollectKeys<TLockInfoTraits>(expectedLockInfos);
			for (auto i = 0u; i < expiredLockInfos.size(); ++i)
				EXPECT_TRUE(keySet.cend() != keySet.find(TLockInfoTraits::ToKey(*expiredLockInfos[i]))) << "at index " << i;
		}
	}

	DELTA_LOCK_TYPE_BASED_TEST(CollectUnusedExpiredLocksReturnsUnusedExpiredLocks_SingleLock) {
		// Arrange:
		typename DeltaElementsMixinTraits<TLockInfoTraits>::CacheType cache;
		auto lockInfos = test::CreateLockInfos<TLockInfoTraits>(Num_Default_Entries);
		PopulateCache<TLockInfoTraits>(cache, lockInfos);

		// Sanity:
		EXPECT_EQ(Num_Default_Entries, cache.createView()->size());

		// Act: locks at height 20, 40, ..., 100 are unused
		auto delta = cache.createDelta();
		auto expiredLockInfos = delta->collectUnusedExpiredLocks(Height(40));

		// Assert:
		LockInfoPointers<TLockInfoTraits> expectedLockInfos{ &lockInfos[3] };
		AssertEqualLockInfos<TLockInfoTraits>(expectedLockInfos, expiredLockInfos);
	}

	DELTA_LOCK_TYPE_BASED_TEST(CollectUnusedExpiredLocksReturnsUnusedExpiredLocks_MultipleLocks) {
		// Arrange:
		typename DeltaElementsMixinTraits<TLockInfoTraits>::CacheType cache;
		auto lockInfos = test::CreateLockInfos<TLockInfoTraits>(Num_Default_Entries);
		PopulateCache<TLockInfoTraits>(cache, lockInfos);
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
		auto expiredLockInfos = delta->collectUnusedExpiredLocks(Height(40));

		// Assert:
		LockInfoPointers<TLockInfoTraits> expectedLockInfos{
			&lockInfos[3],
			&lockInfos[Num_Default_Entries],
			&lockInfos[Num_Default_Entries + 1]
		};
		AssertEqualLockInfos<TLockInfoTraits>(expectedLockInfos, expiredLockInfos);
	}

	DELTA_LOCK_TYPE_BASED_TEST(CollectUnusedExpiredLocksReturnsOnlyUnusedExpiredLocks_MultipleLocks) {
		// Arrange:
		typename DeltaElementsMixinTraits<TLockInfoTraits>::CacheType cache;
		auto lockInfos = test::CreateLockInfos<TLockInfoTraits>(Num_Default_Entries);
		PopulateCache<TLockInfoTraits>(cache, lockInfos);
		{
			// add another four lock infos that expire at height 40, two of which are used
			auto delta = cache.createDelta();
			for (auto i = 0u; i < 4; ++i) {
				auto lockInfo = TLockInfoTraits::CreateLockInfo(Height(40));
				if (1 == i % 2)
					lockInfo.Status = model::LockStatus::Used;

				lockInfos.push_back(lockInfo);
				delta->insert(lockInfos.back());
			}

			cache.commit();
		}

		// Sanity:
		EXPECT_EQ(Num_Default_Entries + 4, cache.createView()->size());

		// Act:
		auto delta = cache.createDelta();
		auto expiredLockInfos = delta->collectUnusedExpiredLocks(Height(40));

		// Assert:
		LockInfoPointers<TLockInfoTraits> expectedLockInfos{
			&lockInfos[3],
			&lockInfos[Num_Default_Entries],
			&lockInfos[Num_Default_Entries + 2]
		};
		AssertEqualLockInfos<TLockInfoTraits>(expectedLockInfos, expiredLockInfos);
	}

	// endregion
}}
