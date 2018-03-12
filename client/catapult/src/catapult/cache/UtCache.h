#pragma once
#include "BasicTransactionsCache.h"
#include <vector>

namespace catapult { namespace cache {

	/// An interface for modifying an unconfirmed transactions cache.
	class UtCacheModifier : public BasicTransactionsCacheModifier<model::TransactionInfo> {
	public:
		/// Removes all transactions from the cache.
		virtual std::vector<model::TransactionInfo> removeAll() = 0;
	};

	/// A delegating proxy around a UtCacheModifier.
	/// \note This is returned by value by UtCache::modifier in order to allow it to be consistent with other modifier functions.
	class UtCacheModifierProxy final : public BasicTransactionsCacheModifierProxy<model::TransactionInfo, UtCacheModifier> {
	private:
		using BasicTransactionsCacheModifierProxy<model::TransactionInfo, UtCacheModifier>::BasicTransactionsCacheModifierProxy;

	public:
		/// Removes all transactions from the cache.
		std::vector<model::TransactionInfo> removeAll() {
			return modifier().removeAll();
		}
	};

	/// An interface for caching unconfirmed transactions.
	class UtCache : public BasicTransactionsCache<UtCacheModifierProxy>
	{};
}}
