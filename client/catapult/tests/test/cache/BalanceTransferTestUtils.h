#pragma once
#include "tests/test/core/BalanceTransfers.h"
#include <memory>

namespace catapult {
	namespace cache {
		class CatapultCache;
		class CatapultCacheDelta;
	}
}

namespace catapult { namespace test {

	/// Seeds \a publicKey account in \a cache with \a transfers.
	void SetCacheBalances(cache::CatapultCacheDelta& cache, const Key& publicKey, const BalanceTransfers& transfers);

	/// Seeds \a address account in \a cache with \a transfers.
	void SetCacheBalances(cache::CatapultCacheDelta& cache, const Address& address, const BalanceTransfers& transfers);

	/// Seeds \a publicKey account in \a cache with \a transfers.
	void SetCacheBalances(cache::CatapultCache& cache, const Key& publicKey, const BalanceTransfers& transfers);

	/// Creates a cache and seeds \a publicKey account with \a transfers.
	cache::CatapultCache CreateCache(const Key& publicKey, const BalanceTransfers& transfers);
}}
