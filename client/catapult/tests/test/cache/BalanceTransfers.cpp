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

#include "BalanceTransfers.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/utils/HexFormatter.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	namespace {
		template<typename TCache, typename TAccountIdentifier>
		void AssertBalancesT(const TCache& cache, const TAccountIdentifier& accountIdentifier, const BalanceTransfers& expectedBalances) {
			// Assert:
			auto accountStateIter = cache.find(accountIdentifier);
			ASSERT_TRUE(!!accountStateIter.tryGet()) << accountIdentifier;

			const auto& accountState = accountStateIter.get();
			EXPECT_EQ(expectedBalances.size(), accountState.Balances.size());
			for (const auto& expectedBalance : expectedBalances) {
				CATAPULT_LOG(debug) << expectedBalance.MosaicId << " " << expectedBalance.Amount;
				EXPECT_EQ(expectedBalance.Amount, accountState.Balances.get(expectedBalance.MosaicId))
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
