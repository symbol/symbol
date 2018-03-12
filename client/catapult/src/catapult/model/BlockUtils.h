#pragma once
#include "Block.h"
#include "Elements.h"
#include "EntityInfo.h"

namespace catapult { namespace crypto { class KeyPair; } }

namespace catapult { namespace model {

	// region hashes

	/// Calculates the block transactions hash of \a transactionInfos into \a blockTransactionsHash.
	void CalculateBlockTransactionsHash(const std::vector<const TransactionInfo*>& transactionInfos, Hash256& blockTransactionsHash);

	/// Calculates the generation hash from a previous generation hash (\a previousGenerationHash)
	/// and a public key (\a publicKey).
	Hash256 CalculateGenerationHash(const Hash256& previousGenerationHash, const Key& publicKey);

	// endregion

	// region sign / verify

	/// Signs \a block header as \a signer.
	/// \note All header data is assumed to be present and valid.
	void SignBlockHeader(const crypto::KeyPair& signer, Block& block);

	/// Validates signature of \a block header.
	bool VerifyBlockHeaderSignature(const Block& block);

	// endregion

	// region create

	/// Container of transactions.
	using Transactions = std::vector<std::shared_ptr<const Transaction>>;

	/// Context passed when creating new block.
	struct PreviousBlockContext {
		/// Creates an empty context.
		PreviousBlockContext()
				: BlockHash{}
				, GenerationHash{}
				, BlockHeight(0)
				, Timestamp(0)
		{}

		/// Creates a context with \a blockElement as the previous block.
		explicit PreviousBlockContext(const BlockElement& blockElement)
				: BlockHash(blockElement.EntityHash)
				, GenerationHash(blockElement.GenerationHash)
				, BlockHeight(blockElement.Block.Height)
				, Timestamp(blockElement.Block.Timestamp)
		{}

		/// Hash of previous block.
		Hash256 BlockHash;

		/// Generation hash of previous block.
		Hash256 GenerationHash;

		/// Height of previous block.
		Height BlockHeight;

		/// Timestamp of previous block.
		catapult::Timestamp Timestamp;
	};

	/// Creates an unsigned Block given a \a context, signer public key (\a signerPublicKey) and \a transactions
	/// for a network with identifier \a networkIdentifier.
	std::unique_ptr<Block> CreateBlock(
			const PreviousBlockContext& context,
			NetworkIdentifier networkIdentifier,
			const Key& signerPublicKey,
			const Transactions& transactions);

	// endregion
}}
