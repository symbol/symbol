#pragma once
#include "catapult/model/Elements.h"

namespace catapult { namespace model { class TransactionRegistry; } }

namespace catapult { namespace extensions {

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

	/// Extensions for working with blocks.
	class BlockExtensions {
	public:
		/// Creates extensions for a block containing only basic transactions.
		BlockExtensions();

		/// Creates extensions for a block containing transactions registered in \a transactionRegistry.
		explicit BlockExtensions(const model::TransactionRegistry& transactionRegistry);

	public:
		/// Calculates and updates the block transactions hash of \a block.
		/// \note This function requires a full block and will calculate all transaction hashes.
		void updateBlockTransactionsHash(model::Block& block) const;

		/// Calculates the block transactions hash of \a block into \a blockTransactionsHash.
		/// \note This function requires a full block and will calculate all transaction hashes.
		void calculateBlockTransactionsHash(const model::Block& block, Hash256& blockTransactionsHash) const;

		/// Cryptographically signs a full \a block with \a signer.
		void signFullBlock(const crypto::KeyPair& signer, model::Block& block) const;

		/// Cryptographically verifies a full \a block by checking all signatures and hashes.
		VerifyFullBlockResult verifyFullBlock(const model::Block& block) const;

		/// Converts \a block to a block element with the specified generation hash (\a generationHash).
		/// \note This function requires a full block and will calculate all block and transaction hashes.
		model::BlockElement convertBlockToBlockElement(const model::Block& block, const Hash256& generationHash) const;

	private:
		std::function<Hash256 (const model::Transaction&)> m_calculateTransactionEntityHash;
		std::function<Hash256 (const model::Transaction&, const Hash256&)> m_calculateTransactionMerkleComponentHash;
	};
}}
