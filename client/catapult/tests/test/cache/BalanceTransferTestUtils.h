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
#include "BalanceTransfers.h"
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

	/// Seeds \a address account in \a cache with \a transfers.
	void SetCacheBalances(cache::CatapultCache& cache, const Address& address, const BalanceTransfers& transfers);

	/// Creates a cache and seeds \a publicKey account with \a transfers.
	cache::CatapultCache CreateCache(const Key& publicKey, const BalanceTransfers& transfers);

	/// Creates a cache and seeds \a address account with \a transfers.
	cache::CatapultCache CreateCache(const Address& address, const BalanceTransfers& transfers);
}}
