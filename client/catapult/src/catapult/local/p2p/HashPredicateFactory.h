#pragma once
#include "catapult/consumers/TransactionConsumers.h"
#include "catapult/model/Elements.h"

namespace catapult {
	namespace cache {
		class CatapultCache;
		class MemoryUtCache;
	}
}

namespace catapult { namespace local { namespace p2p {

	/// A factory for creating hash predicates.
	class HashPredicateFactory {
	public:
		/// Creates a factory around \a unconfirmedTransactionsCache and \a catapultCache.
		HashPredicateFactory(const cache::MemoryUtCache& unconfirmedTransactionsCache, cache::CatapultCache& catapultCache)
				: m_unconfirmedTransactionsCache(unconfirmedTransactionsCache)
				, m_catapultCache(catapultCache)
		{}

	public:
		/// Creates an unknown transaction predicate for the block dispatcher that returns \c true if an entity is a transaction
		/// and isn't contained in any cache.
		model::MatchingEntityPredicate createUnknownTransactionPredicate() const;

		/// Creates a known hash predicate for the transaction dispatcher that returns \c true if a transaction is contained in
		/// any cache.
		consumers::KnownHashPredicate createKnownHashPredicate() const;

	private:
		const cache::MemoryUtCache& m_unconfirmedTransactionsCache;
		const cache::CatapultCache& m_catapultCache;
	};
}}}
