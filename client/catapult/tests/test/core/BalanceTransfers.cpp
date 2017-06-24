#include "BalanceTransfers.h"
#include "catapult/cache/AccountStateCache.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/utils/HexFormatter.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	namespace {
		template<typename TCache, typename TKey>
		void AssertBalancesT(const TCache& cache, const TKey& key, const BalanceTransfers& expectedBalances) {
			// Assert:
			auto pState = cache.findAccount(key);
			ASSERT_TRUE(!!pState) << utils::HexFormat(key);

			EXPECT_EQ(expectedBalances.size(), pState->Balances.size());
			for (const auto& expectedBalance : expectedBalances) {
				CATAPULT_LOG(debug) << expectedBalance.MosaicId << " " << expectedBalance.Amount;
				EXPECT_EQ(expectedBalance.Amount, pState->Balances.get(expectedBalance.MosaicId)) << "mosaic " << expectedBalance.MosaicId;
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

	void AssertBalances(const cache::AccountStateCacheView& cache, const Key& publicKey, const BalanceTransfers& expectedBalances) {
		// Assert:
		AssertBalancesT(cache, publicKey, expectedBalances);
	}
}}
