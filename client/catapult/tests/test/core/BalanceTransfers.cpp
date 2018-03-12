#include "BalanceTransfers.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/utils/HexFormatter.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	namespace {
		template<typename TCache, typename TKey>
		void AssertBalancesT(const TCache& cache, const TKey& key, const BalanceTransfers& expectedBalances) {
			// Assert:
			const auto* pAccountState = cache.tryGet(key);
			ASSERT_TRUE(!!pAccountState) << utils::HexFormat(key);

			EXPECT_EQ(expectedBalances.size(), pAccountState->Balances.size());
			for (const auto& expectedBalance : expectedBalances) {
				CATAPULT_LOG(debug) << expectedBalance.MosaicId << " " << expectedBalance.Amount;
				EXPECT_EQ(expectedBalance.Amount, pAccountState->Balances.get(expectedBalance.MosaicId))
						<< "mosaic " << expectedBalance.MosaicId;
			}
		}
	}

	void AssertBalances(const cache::CatapultCacheDelta& cache, const Key& publicKey, const BalanceTransfers& expectedBalances) {
		// Assert:
		AssertBalancesT(cache.sub<cache::AccountStateCache>(), publicKey, expectedBalances);
	}

	void AssertBalances(const cache::CatapultCacheDelta& cache, const Address& address, const BalanceTransfers& expectedBalances) {
		// Assert:
		AssertBalancesT(cache.sub<cache::AccountStateCache>(), address, expectedBalances);
	}

	void AssertBalances(const cache::AccountStateCacheDelta& cache, const Key& publicKey, const BalanceTransfers& expectedBalances) {
		// Assert:
		AssertBalancesT(cache, publicKey, expectedBalances);
	}

	void AssertBalances(const cache::AccountStateCacheView& cache, const Key& publicKey, const BalanceTransfers& expectedBalances) {
		// Assert:
		AssertBalancesT(cache, publicKey, expectedBalances);
	}
}}
