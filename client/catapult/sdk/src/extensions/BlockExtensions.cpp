#include "BlockExtensions.h"
#include "TransactionExtensions.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/crypto/MerkleHashBuilder.h"
#include "catapult/model/Block.h"
#include "catapult/model/BlockUtils.h"
#include "catapult/model/EntityHasher.h"

namespace catapult { namespace extensions {

	void UpdateBlockTransactionsHash(model::Block& block) {
		CalculateBlockTransactionsHash(block, block.BlockTransactionsHash);
	}

	void CalculateBlockTransactionsHash(const model::Block& block, Hash256& blockTransactionsHash) {
		crypto::MerkleHashBuilder builder;
		for (const auto& transaction : block.Transactions()) {
			auto txHash = model::CalculateHash(transaction);
			builder.update(txHash);
		}

		builder.final(blockTransactionsHash);
	}

	void SignFullBlock(const crypto::KeyPair& signer, model::Block& block) {
		// calculate the block transactions hash
		UpdateBlockTransactionsHash(block);

		// sign the block
		model::SignBlockHeader(signer, block);
	}

	VerifyFullBlockResult VerifyFullBlock(const model::Block& block) {
		// check block header signature
		if (!model::VerifyBlockHeaderSignature(block))
			return VerifyFullBlockResult::Invalid_Block_Signature;

		// check block transactions hash
		Hash256 expectedBlockTransactionsHash;
		CalculateBlockTransactionsHash(block, expectedBlockTransactionsHash);
		if (expectedBlockTransactionsHash != block.BlockTransactionsHash)
			return VerifyFullBlockResult::Invalid_Block_Transactions_Hash;

		// check transaction signatures
		for (const auto& transaction : block.Transactions()) {
			if (!VerifyTransactionSignature(transaction))
				return VerifyFullBlockResult::Invalid_Transaction_Signature;
		}

		return VerifyFullBlockResult::Success;
	}

	namespace {
		template<typename THashUpdater>
		model::BlockElement ConvertBlockToBlockElementT(
				const model::Block& block,
				const Hash256& generationHash,
				THashUpdater hashUpdater) {
			model::BlockElement blockElement(block);
			blockElement.EntityHash = model::CalculateHash(block);
			blockElement.GenerationHash = generationHash;

			for (const auto& transaction : block.Transactions()) {
				blockElement.Transactions.push_back(model::TransactionElement(transaction));
				hashUpdater(blockElement.Transactions.back());
			}

			return blockElement;
		}
	}

	model::BlockElement ConvertBlockToBlockElement(const model::Block& block, const Hash256& generationHash) {
		return ConvertBlockToBlockElementT(block, generationHash, [](auto& txElement) {
			txElement.EntityHash = model::CalculateHash(txElement.Transaction);
			txElement.MerkleComponentHash = txElement.EntityHash;
		});
	}

	model::BlockElement ConvertBlockToBlockElement(
			const model::Block& block,
			const Hash256& generationHash,
			const model::TransactionRegistry& transactionRegistry) {
		return ConvertBlockToBlockElementT(block, generationHash, [&transactionRegistry](auto& txElement) {
			model::UpdateHashes(transactionRegistry, txElement);
		});
	}
}}
