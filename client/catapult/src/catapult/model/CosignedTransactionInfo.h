#pragma once
#include "Cosignature.h"
#include "Transaction.h"
#include <vector>

namespace catapult { namespace model {

	/// Information about a cosigned transaction.
	/// \note In order to allow efficient aggregation, only some information may be specified.
	struct CosignedTransactionInfo {
		/// The transaction entity hash.
		Hash256 EntityHash;

		/// The (optional) transaction.
		std::shared_ptr<const Transaction> pTransaction;

		/// The (optional) cosignatures.
		std::vector<Cosignature> Cosignatures;
	};
}}

