#pragma once
#include "catapult/builders/TransactionBuilder.h"
#include "catapult/model/Mosaic.h"
#include "catapult/state/TimestampedHash.h"
#include <vector>

namespace catapult { namespace model { struct Transaction; } }

namespace catapult { namespace tools {

	/// Sets \a fee and default deadline of 1h on \a builder.
	void SetDeadlineAndFee(builders::TransactionBuilder& builder, Amount fee);

	/// Creates name from a \a prefix and \a transactionId.
	std::string CreateName(const std::string& prefix, size_t transactionId);

	/// Creates transfer transaction, identified by \a transactionId with set of \a mosaics attached,
	/// from \a signer to recipient (\a recipientPublicKey) for network (\a networkId).
	std::unique_ptr<model::Transaction> CreateSignedTransferTransaction(
			model::NetworkIdentifier networkId,
			const crypto::KeyPair& signer,
			const Key& recipientPublicKey,
			uint64_t transactionId,
			std::initializer_list<model::Mosaic> mosaics);

	/// A vector of transaction shared pointers.
	using Transactions = std::vector<std::shared_ptr<const model::Transaction>>;

	/// Asserts that \a resultRange is empty. If not empty, outputs a warning for each entry.
	bool VerifyEmptyAndLogFailures(const state::TimestampedHashRange& resultRange);

	/// Asserts that each element in \a transactions has a corresponding entry in \a resultRange.
	/// Outputs a warning for each transaction that does not have an entry in \a resultRange.
	bool VerifyMatchingAndLogFailures(const state::TimestampedHashRange& resultRange, const Transactions& transactions);
}}
