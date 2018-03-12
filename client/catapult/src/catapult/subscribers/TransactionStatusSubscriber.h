#pragma once
#include "catapult/validators/ValidationResult.h"
#include "catapult/types.h"

namespace catapult { namespace model { struct Transaction; } }

namespace catapult { namespace subscribers {

	/// Transaction status subscriber.
	class TransactionStatusSubscriber {
	public:
		virtual ~TransactionStatusSubscriber() {}

	public:
		/// Indicates \a transaction with \a hash completed with \a status.
		virtual void notifyStatus(const model::Transaction& transaction, const Hash256& hash, uint32_t status) = 0;

		/// Flushes all queued data.
		virtual void flush() = 0;
	};
}}
