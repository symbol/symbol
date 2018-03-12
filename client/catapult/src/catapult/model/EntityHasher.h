#pragma once
#include "Block.h"

namespace catapult {
	namespace model {
		struct TransactionElement;
		class TransactionRegistry;
	}
}

namespace catapult { namespace model {

	/// Calculates the hash for the given \a block header.
	Hash256 CalculateHash(const Block& block);

	/// Calculates the hash for the given \a transaction.
	Hash256 CalculateHash(const Transaction& transaction);

	/// Calculates the hash for the given \a entity with data \a buffer.
	Hash256 CalculateHash(const VerifiableEntity& entity, const RawBuffer& buffer);

	/// Calculates the merkle component hash for the given \a transaction with \a transactionHash
	/// using transaction information from \a transactionRegistry.
	Hash256 CalculateMerkleComponentHash(
			const Transaction& transaction,
			const Hash256& transactionHash,
			const TransactionRegistry& transactionRegistry);

	/// Calculates the hashes for \a transactionElement in place using transaction information from \a transactionRegistry.
	void UpdateHashes(const TransactionRegistry& transactionRegistry, TransactionElement& transactionElement);
}}
