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
#include "plugins/txes/lock_shared/src/state/LockInfo.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	/// Creates \a count lock infos with increasing heights.
	template<typename TLockInfoTraits>
	auto CreateLockInfos(size_t count) {
		std::vector<typename TLockInfoTraits::ValueType> lockInfos;
		for (auto i = 0u; i < count; ++i) {
			lockInfos.emplace_back(TLockInfoTraits::CreateLockInfo(Height((i + 1) * 10)));
			if (0 == i % 2)
				lockInfos.back().Status = state::LockStatus::Used;
		}

		return lockInfos;
	}

	/// Asserts that \a lhs and \a rhs are equal.
	inline void AssertEqualLockInfo(const state::LockInfo& lhs, const state::LockInfo& rhs) {
		EXPECT_EQ(lhs.OwnerAddress, rhs.OwnerAddress);
		EXPECT_EQ(lhs.MosaicId, rhs.MosaicId);
		EXPECT_EQ(lhs.Amount, rhs.Amount);
		EXPECT_EQ(lhs.EndHeight, rhs.EndHeight);
		EXPECT_EQ(lhs.Status, rhs.Status);
	}

	/// Asserts that \a cache contains exactly all expected lock infos (\a expectedLockInfos).
	template<typename TLockInfoTraits>
	void AssertCacheContents(
			const typename TLockInfoTraits::CacheDeltaType& cache,
			const std::vector<typename TLockInfoTraits::ValueType>& expectedLockInfos) {
		EXPECT_EQ(expectedLockInfos.size(), cache.size());

		for (const auto& expectedLockInfo : expectedLockInfos) {
			auto key = TLockInfoTraits::ToKey(expectedLockInfo);
			ASSERT_TRUE(cache.contains(key));

			const auto& lockInfo = cache.get(key);
			TLockInfoTraits::AssertEqual(expectedLockInfo, lockInfo);
		}
	}

	/// Basic lock info cache factory.
	template<typename TCacheTraits, typename TCacheStorage>
	struct LockInfoCacheFactory {
	public:
		using LockInfoCache = typename TCacheTraits::CacheType;
		using LockInfoCacheStorage = TCacheStorage;

	private:
		static auto CreateSubCachesWithLockHashCache() {
			auto cacheId = LockInfoCache::Id;
			std::vector<std::unique_ptr<cache::SubCachePlugin>> subCaches(cacheId + 1);
			subCaches[cacheId] = MakeSubCachePlugin<LockInfoCache, LockInfoCacheStorage>();
			return subCaches;
		}

	public:
		/// Creates an empty catapult cache.
		static cache::CatapultCache Create() {
			return cache::CatapultCache(CreateSubCachesWithLockHashCache());
		}

		/// Creates an empty catapult cache around \a config.
		static cache::CatapultCache Create(const model::BlockChainConfiguration& config) {
			auto subCaches = CreateSubCachesWithLockHashCache();
			CoreSystemCacheFactory::CreateSubCaches(config, subCaches);
			return cache::CatapultCache(std::move(subCaches));
		}
	};
}}
