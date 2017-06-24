#include "BalanceTransferTestUtils.h"
#include "CacheTestUtils.h"
#include "catapult/cache/AccountStateCache.h"
#include "catapult/cache/CatapultCache.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	namespace {
		template<typename TKey>
		void SetCacheBalancesT(cache::CatapultCacheDelta& cache, const TKey& key, const BalanceTransfers& transfers) {
			auto& accountStateCache = cache.sub<cache::AccountStateCache>();
			auto pAccount = accountStateCache.addAccount(key, Height(123));
			for (const auto& transfer : transfers)
				pAccount->Balances.credit(transfer.MosaicId, transfer.Amount);
		}
	}

	void SetCacheBalances(cache::CatapultCacheDelta& cache, const Key& publicKey, const BalanceTransfers& transfers) {
		SetCacheBalancesT(cache, publicKey, transfers);
	}

	void SetCacheBalances(cache::CatapultCacheDelta& cache, const Address& address, const BalanceTransfers& transfers) {
		SetCacheBalancesT(cache, address, transfers);
	}

	void SetCacheBalances(cache::CatapultCache& cache, const Key& publicKey, const BalanceTransfers& transfers) {
		auto delta = cache.createDelta();
		SetCacheBalances(delta, publicKey, transfers);
		cache.commit(Height());
	}

	cache::CatapultCache CreateCache(const Key& publicKey, const BalanceTransfers& transfers) {
		auto cache = CreateEmptyCatapultCache();
		SetCacheBalances(cache, publicKey, transfers);
		return cache;
	}
}}
