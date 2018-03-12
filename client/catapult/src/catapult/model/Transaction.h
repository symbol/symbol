#pragma once
#include "EmbeddedTransaction.h"
#include "VerifiableEntity.h"

namespace catapult { namespace model { class TransactionRegistry; } }

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for a transaction.
	struct Transaction : public VerifiableEntity {
		/// The transaction fee.
		Amount Fee;

		/// The transaction deadline.
		Timestamp Deadline;
	};

#pragma pack(pop)

	/// Checks the real size of \a transaction against its reported size and returns \c true if the sizes match.
	/// \a registry contains all known transaction types.
	bool IsSizeValid(const Transaction& transaction, const TransactionRegistry& registry);

/// Defines a transaction with \a NAME that supports embedding.
#define DEFINE_EMBEDDABLE_TRANSACTION(NAME) \
	struct Embedded##NAME##Transaction : public NAME##TransactionBody<model::EmbeddedTransaction> {}; \
	struct NAME##Transaction : public NAME##TransactionBody<model::Transaction> {};
}}
