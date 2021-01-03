/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include "catapult/model/EntityInfo.h"
#include <vector>

namespace catapult { namespace cache {

	/// Interface for modifying an unconfirmed transactions cache.
	class PLUGIN_API_DEPENDENCY UtCacheModifier {
	public:
		virtual ~UtCacheModifier() noexcept(false) {}

	public:
		/// Gets the number of transactions in the cache.
		virtual size_t size() const = 0;

		/// Gets the memory size of all unconfirmed transactions in the cache.
		virtual utils::FileSize memorySize() const = 0;

		/// Adds the transaction info (\a transactionInfo) to the cache.
		/// Returns \c true if the transaction info was successfully added.
		virtual bool add(const model::TransactionInfo& transactionInfo) = 0;

		/// Removes the transaction identified by \a hash from the cache.
		virtual model::TransactionInfo remove(const Hash256& hash) = 0;

		/// Gets the memory size of transactions an account with public \a key has placed into the cache.
		virtual utils::FileSize memorySizeForAccount(const Key& key) const = 0;

		/// Removes all transactions from the cache.
		virtual std::vector<model::TransactionInfo> removeAll() = 0;
	};

	/// Delegating proxy around a UtCacheModifier.
	/// \note This is returned by value by UtCache::modifier in order to allow it to be consistent with other modifier functions.
	class UtCacheModifierProxy final : public BasicTransactionsCacheModifierProxy<model::TransactionInfo, UtCacheModifier> {
	private:
		using BasicTransactionsCacheModifierProxy<model::TransactionInfo, UtCacheModifier>::BasicTransactionsCacheModifierProxy;

	public:
		/// Gets the memory size of transactions an account with public \a key has placed into the cache.
		utils::FileSize memorySizeForAccount(const Key& key) const {
			return modifier().memorySizeForAccount(key);
		}

		/// Removes all transactions from the cache.
		std::vector<model::TransactionInfo> removeAll() {
			return modifier().removeAll();
		}
	};

	/// Interface (write only) for caching unconfirmed transactions.
	class PLUGIN_API_DEPENDENCY UtCache {
	public:
		using CacheModifierProxy = UtCacheModifierProxy;

	public:
		virtual ~UtCache() = default;

	public:
		/// Gets a write only view of the cache.
		virtual UtCacheModifierProxy modifier() = 0;
	};
}}
