#pragma once
#include "catapult/utils/BitwiseEnum.h"
#include "catapult/functions.h"
#include "catapult/types.h"

namespace catapult { namespace extensions {

	/// Possible transaction events.
	enum class TransactionEvent {
		/// Transaction dependency was removed.
		Dependency_Removed = 1
	};

	MAKE_BITWISE_ENUM(TransactionEvent);

	/// Data associated with a transaction event.
	struct TransactionEventData {
	public:
		/// Creates transaction event data around \a transactionHash and \a event.
		TransactionEventData(const Hash256& transactionHash, TransactionEvent event)
				: TransactionHash(transactionHash)
				, Event(event)
		{}

	public:
		/// The transaction hash.
		const Hash256& TransactionHash;

		/// The transaction event.
		TransactionEvent Event;
	};

	/// Handler that is called when a transaction event is raised.
	using TransactionEventHandler = consumer<const TransactionEventData&>;
}}
