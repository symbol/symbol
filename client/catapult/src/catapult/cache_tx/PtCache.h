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
#include "catapult/model/EntityInfo.h"
#include "catapult/functions.h"
#include <vector>

namespace catapult { namespace model { struct Cosignature; } }

namespace catapult { namespace cache {

	/// Interface for modifying a partial transactions cache.
	/// \note Cache assumes that added transactions are stripped of all cosignatures.
	class PtCacheModifier {
	public:
		virtual ~PtCacheModifier() noexcept(false) {}

	public:
		/// Gets the number of transactions in the cache.
		virtual size_t size() const = 0;

		/// Adds the transaction info (\a transactionInfo) to the cache.
		/// Returns \c true if the transaction info was successfully added.
		virtual bool add(const model::DetachedTransactionInfo& transactionInfo) = 0;

		/// Adds \a cosignature for a partial transaction with hash \a parentHash to the cache.
		virtual model::DetachedTransactionInfo add(const Hash256& parentHash, const model::Cosignature& cosignature) = 0;

		/// Removes the transaction identified by \a hash from the cache.
		virtual model::DetachedTransactionInfo remove(const Hash256& hash) = 0;

		/// Removes all partial transactions that have deadlines at or before the given \a timestamp.
		virtual std::vector<model::DetachedTransactionInfo> prune(Timestamp timestamp) = 0;

		/// Removes all partial transactions for which \a hashPredicate returns \c true.
		virtual std::vector<model::DetachedTransactionInfo> prune(const predicate<const Hash256&>& hashPredicate) = 0;
	};

	/// Delegating proxy around a PtCacheModifier.
	/// \note This is returned by value by PtCache::modifier in order to allow it to be consistent with other modifier functions.
	class PtCacheModifierProxy final : public BasicTransactionsCacheModifierProxy<model::DetachedTransactionInfo, PtCacheModifier> {
	public:
		using BaseType = BasicTransactionsCacheModifierProxy<model::DetachedTransactionInfo, PtCacheModifier>;
		using BaseType::BasicTransactionsCacheModifierProxy;
		using BaseType::add;

	public:
		/// Adds \a cosignature for a partial transaction with hash \a parentHash to the cache.
		model::DetachedTransactionInfo add(const Hash256& parentHash, const model::Cosignature& cosignature) {
			return modifier().add(parentHash, cosignature);
		}

		/// Removes all partial transactions that have deadlines at or before the given \a timestamp.
		std::vector<model::DetachedTransactionInfo> prune(Timestamp timestamp) {
			return modifier().prune(timestamp);
		}

		/// Removes all partial transactions for which \a hashPredicate returns \c true.
		std::vector<model::DetachedTransactionInfo> prune(const predicate<const Hash256&>& hashPredicate) {
			return modifier().prune(hashPredicate);
		}
	};

	/// Interface (write only) for caching partial transactions.
	class PtCache {
	public:
		using CacheModifierProxy = PtCacheModifierProxy;

	public:
		virtual ~PtCache() = default;

	public:
		/// Gets a write only view of the cache.
		virtual PtCacheModifierProxy modifier() = 0;
	};
}}
