#pragma once
#include "catapult/model/BlockUtils.h"

namespace catapult { namespace cache { class MemoryUtCache; } }

namespace catapult { namespace chain {

	/// Information about transactions.
	struct TransactionsInfo {
		/// The transactions.
		model::Transactions Transactions;

		/// The transactions hash.
		Hash256 TransactionsHash;
	};

	/// Supplies a transactions info composed of a maximum number of transactions.
	using TransactionsInfoSupplier = std::function<TransactionsInfo (uint32_t)>;

	/// Creates a default transactions info supplier around \a unconfirmedTransactionsCache.
	TransactionsInfoSupplier CreateTransactionsInfoSupplier(const cache::MemoryUtCache& unconfirmedTransactionsCache);
}}
