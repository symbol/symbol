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
#include "BasicTransactionsCache.h"
#include <vector>

namespace catapult { namespace cache {

	/// An interface for modifying an unconfirmed transactions cache.
	class UtCacheModifier : public BasicTransactionsCacheModifier<model::TransactionInfo> {
	public:
		/// Gets the number of transactions an account with public \a key has placed into the cache.
		virtual size_t count(const Key& key) const = 0;

		/// Removes all transactions from the cache.
		virtual std::vector<model::TransactionInfo> removeAll() = 0;
	};

	/// A delegating proxy around a UtCacheModifier.
	/// \note This is returned by value by UtCache::modifier in order to allow it to be consistent with other modifier functions.
	class UtCacheModifierProxy final : public BasicTransactionsCacheModifierProxy<model::TransactionInfo, UtCacheModifier> {
	private:
		using BasicTransactionsCacheModifierProxy<model::TransactionInfo, UtCacheModifier>::BasicTransactionsCacheModifierProxy;

	public:
		/// Gets the number of transactions an account with public \a key has placed into the cache.
		size_t count(const Key& key) const {
			return modifier().count(key);
		}

		/// Removes all transactions from the cache.
		std::vector<model::TransactionInfo> removeAll() {
			return modifier().removeAll();
		}
	};

	/// An interface for caching unconfirmed transactions.
	class UtCache : public BasicTransactionsCache<UtCacheModifierProxy> {};
}}
