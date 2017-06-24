#pragma once
#include "catapult/model/EntityInfo.h"
#include "catapult/types.h"
#include <memory>
#include <vector>

namespace catapult { namespace io {

	/// An interface for storing transactions.
	class TransactionStorage {
	public:
		virtual ~TransactionStorage() {}

	public:
		/// Adds the transaction info (\a transactionInfo) to the storage.
		/// Returns \c true if the transaction info was successfully added.
		virtual bool saveTransaction(const model::TransactionInfo& transactionInfo) = 0;

		/// Removes a transaction info identified by \a hash from the storage.
		virtual void removeTransaction(const Hash256& hash) = 0;

		/// Removes all transactions identified by \a transactionInfos from the storage.
		virtual void removeTransactions(const std::vector<model::TransactionInfo>& transactionInfos) = 0;

		/// Prunes transactions with deadlines prior to \a timestamp.
		virtual void pruneTransactions(Timestamp timestamp) = 0;

		/// Commits prior changes.
		virtual void commit() = 0;
	};
}}
