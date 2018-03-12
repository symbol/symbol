#include "PredicateUtils.h"

namespace catapult { namespace sync {

	model::MatchingEntityPredicate ToUnknownTransactionPredicate(const chain::KnownHashPredicate& knownHashPredicate) {
		return [knownHashPredicate](auto entityType, auto timestamp, const auto& hash) {
			auto isTransaction = model::BasicEntityType::Transaction == entityType;
			return isTransaction && !knownHashPredicate(timestamp, hash);
		};
	}
}}
