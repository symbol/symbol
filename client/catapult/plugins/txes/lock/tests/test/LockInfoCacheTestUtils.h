#pragma once
#include "src/cache/HashLockInfoCache.h"
#include "src/cache/HashLockInfoCacheTypes.h"
#include "src/cache/LockInfoCacheStorage.h"
#include "src/cache/SecretLockInfoCache.h"
#include "src/cache/SecretLockInfoCacheTypes.h"
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
				lockInfos.back().Status = model::LockStatus::Used;
		}

		return lockInfos;
	}

	/// Asserts that the lock infos \a lhs and \a rhs are equal.
	void AssertEqualLockInfo(const model::LockInfo& lhs, const model::LockInfo& rhs);

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

	/// Basic traits for a hash lock info.
	struct BasicHashLockInfoTestTraits : public cache::HashLockInfoCacheDescriptor {
		using cache::HashLockInfoCacheDescriptor::ValueType;

		static constexpr auto ToKey = cache::HashLockInfoCacheDescriptor::GetKeyFromValue;

		/// Creates a hash lock info with given \a height.
		static ValueType CreateLockInfo(Height height);

		/// Creates a random hash lock info.
		static ValueType CreateLockInfo();

		/// Asserts that the hash lock infos \a lhs and \a rhs are equal.
		static void AssertEqual(const ValueType& lhs, const ValueType& rhs);
	};

	/// Basic traits for a secret lock info.
	struct BasicSecretLockInfoTestTraits : public cache::SecretLockInfoCacheDescriptor {
		using cache::SecretLockInfoCacheDescriptor::ValueType;

		static constexpr auto ToKey = cache::SecretLockInfoCacheDescriptor::GetKeyFromValue;

		/// Creates a secret lock info with given \a height.
		static ValueType CreateLockInfo(Height height);

		/// Creates a random secret lock info.
		static ValueType CreateLockInfo();

		/// Asserts that the secret lock infos \a lhs and \a rhs are equal.
		static void AssertEqual(const ValueType& lhs, const ValueType& rhs);
	};

	/// Basic lock info cache factory.
	template<typename TCacheTraits>
	struct LockInfoCacheFactory {
	public:
		using LockInfoCache = typename TCacheTraits::CacheType;
		using LockInfoCacheStorage = cache::LockInfoCacheStorage<TCacheTraits>;

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

	/// Cache factory for creating a catapult cache containing lock hash cache.
	using HashLockInfoCacheFactory = LockInfoCacheFactory<cache::HashLockInfoCacheDescriptor>;

	/// Cache factory for creating a catapult cache containing lock secret cache.
	using SecretLockInfoCacheFactory = LockInfoCacheFactory<cache::SecretLockInfoCacheDescriptor>;
}}
