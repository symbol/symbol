#pragma once
#include "catapult/model/Mosaic.h"
#include <vector>

namespace catapult {
	namespace cache {
		class AccountStateCacheDelta;
		class AccountStateCacheView;
		class CatapultCacheDelta;
	}
}

namespace catapult { namespace test {

	/// A vector of balance transfers.
	using BalanceTransfers = std::vector<model::Mosaic>;

	/// Asserts that \a cache contains an account with \a publicKey that has all balances in \a expectedBalances.
	void AssertBalances(const cache::CatapultCacheDelta& cache, const Key& publicKey, const BalanceTransfers& expectedBalances);

	/// Asserts that \a cache contains an account with \a address that has all balances in \a expectedBalances.
	void AssertBalances(const cache::CatapultCacheDelta& cache, const Address& address, const BalanceTransfers& expectedBalances);

	/// Asserts that \a cache contains an account with \a publicKey that has all balances in \a expectedBalances.
	void AssertBalances(const cache::AccountStateCacheDelta& cache, const Key& publicKey, const BalanceTransfers& expectedBalances);

	/// Asserts that \a cache contains an account with \a publicKey that has all balances in \a expectedBalances.
	void AssertBalances(const cache::AccountStateCacheView& cache, const Key& publicKey, const BalanceTransfers& expectedBalances);
}}
