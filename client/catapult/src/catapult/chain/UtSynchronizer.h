#pragma once
#include "RemoteNodeSynchronizer.h"
#include "catapult/handlers/HandlerTypes.h"
#include "catapult/model/RangeTypes.h"

namespace catapult { namespace api { class RemoteTransactionApi; } }

namespace catapult { namespace chain {

	/// Function signature for supplying a range of short hashes.
	using ShortHashesSupplier = supplier<model::ShortHashRange>;

	/// Creates an unconfirmed transactions synchronizer around the specified short hashes supplier (\a shortHashesSupplier)
	/// and transaction range consumer (\a transactionRangeConsumer).
	RemoteNodeSynchronizer<api::RemoteTransactionApi> CreateUtSynchronizer(
			const ShortHashesSupplier& shortHashesSupplier,
			const handlers::TransactionRangeHandler& transactionRangeConsumer);
}}
