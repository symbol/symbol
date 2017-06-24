#pragma once
#include "catapult/cache/CatapultCache.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "tests/test/cache/CacheTestUtils.h"
#include <memory>

namespace catapult {
	namespace crypto { class KeyPair; }
	namespace model { struct Block; }
}

namespace catapult { namespace test {

	/// Creates a catapult cache for use in dispatcher tests.
	template<typename TCacheFactory>
	cache::CatapultCache CreateCatapultCacheForDispatcherTests() {
		// - importance grouping must be non-zero
		auto config = model::BlockChainConfiguration::Uninitialized();
		config.ImportanceGrouping = 1;

		// - create the cache
		return CreateEmptyCatapultCache<TCacheFactory>(config);
	}

	/// Initializes \a cache by adding \a signer to account state cache and seeding difficulty cache.
	void InitializeCatapultCacheForDispatcherTests(cache::CatapultCache& cache, const crypto::KeyPair& signer);

	/// Creates a block at height 1 with \a signer.
	std::shared_ptr<model::Block> CreateValidBlockForDispatcherTests(const crypto::KeyPair& signer);
}}
