#pragma once
#include "catapult/functions.h"
#include "catapult/types.h"

namespace catapult {
	namespace model { struct Transaction; }
	namespace validators { enum class ValidationResult : uint32_t; }
}

namespace catapult { namespace chain {

	/// Indicates a transaction with the specified hash failed validation.
	using FailedTransactionSink = consumer<const model::Transaction&, const Hash256&, validators::ValidationResult>;

	/// Predicate for determining if a hash is known.
	using KnownHashPredicate = predicate<Timestamp, const Hash256&>;

	/// Supplies a timestamp.
	using TimeSupplier = supplier<Timestamp>;
}}
