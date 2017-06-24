#include "HashPredicateFactory.h"
#include "plugins/services/hashcache/src/cache/HashCachePredicates.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache/MemoryUtCache.h"

namespace catapult { namespace local { namespace p2p {

	namespace {
		consumers::KnownHashPredicate CreateKnownHashPredicate(
				const cache::MemoryUtCache& unconfirmedTransactionsCache,
				const cache::CatapultCache& catapultCache) {
			return [&unconfirmedTransactionsCache, &catapultCache](auto timestamp, const auto& hash) {
				return unconfirmedTransactionsCache.view().contains(hash) || cache::HashCacheContains(catapultCache, timestamp, hash);
			};
		}

		model::MatchingEntityPredicate CreateUnknownTransactionPredicate(
				const cache::MemoryUtCache& unconfirmedTransactionsCache,
				const cache::CatapultCache& catapultCache) {
			auto knownHashPredicate = CreateKnownHashPredicate(unconfirmedTransactionsCache, catapultCache);
			return [knownHashPredicate](auto entityType, auto timestamp, const auto& hash) {
				auto isTransaction = model::BasicEntityType::Transaction == entityType;
				return isTransaction && !knownHashPredicate(timestamp, hash);
			};
		}
	}

	model::MatchingEntityPredicate HashPredicateFactory::createUnknownTransactionPredicate() const {
		return CreateUnknownTransactionPredicate(m_unconfirmedTransactionsCache, m_catapultCache);
	}

	consumers::KnownHashPredicate HashPredicateFactory::createKnownHashPredicate() const {
		return CreateKnownHashPredicate(m_unconfirmedTransactionsCache, m_catapultCache);
	}
}}}
