#pragma once
#include "catapult/consumers/TransactionConsumers.h"
#include "catapult/model/Elements.h"

namespace catapult { namespace sync {

	/// Converts a known hash predicate (\a knownHashPredicate) to an unknown transaction predicate.
	model::MatchingEntityPredicate ToUnknownTransactionPredicate(const chain::KnownHashPredicate& knownHashPredicate);
}}
