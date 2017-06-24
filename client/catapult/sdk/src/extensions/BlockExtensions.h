#pragma once
#include "catapult/model/Elements.h"

namespace catapult { namespace model { class TransactionRegistry; } }

namespace catapult { namespace extensions {

	/// Calculates and updates the block transactions hash of \a block.
	/// \note This function requires a full block and will calculate all transaction hashes.
	void UpdateBlockTransactionsHash(model::Block& block);

	/// Calculates the block transactions hash of \a block into \a blockTransactionsHash.
	/// \note This function requires a full block and will calculate all transaction hashes.
	void CalculateBlockTransactionsHash(const model::Block& block, Hash256& blockTransactionsHash);

	/// Cryptographically signs a full \a block with \a signer.
	void SignFullBlock(const crypto::KeyPair& signer, model::Block& block);

	/// Possible results of verifying a full block.
	enum class VerifyFullBlockResult {
		/// The block is valid.
		Success,
		/// The block signature is invalid.
		Invalid_Block_Signature,
		/// The block transactions hash is invalid.
		Invalid_Block_Transactions_Hash,
		/// A transaction signature is invalid.
		Invalid_Transaction_Signature,
	};

	/// Cryptographically verifies a full \a block by checking all signatures and hashes.
	VerifyFullBlockResult VerifyFullBlock(const model::Block& block);

	/// Converts \a block to a block element with the specified generation hash (\a generationHash).
	/// \note This function requires a full block and will calculate all block and transaction hashes assuming basic transactions.
	model::BlockElement ConvertBlockToBlockElement(const model::Block& block, const Hash256& generationHash);

	/// Converts \a block to a block element with the specified generation hash (\a generationHash).
	/// \note This function requires a full block and will calculate all block and transaction hashes using transaction
	///       information from \a transactionRegistry.
	model::BlockElement ConvertBlockToBlockElement(
			const model::Block& block,
			const Hash256& generationHash,
			const model::TransactionRegistry& transactionRegistry);
}}
