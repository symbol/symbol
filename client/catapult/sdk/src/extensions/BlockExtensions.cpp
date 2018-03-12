#include "BlockExtensions.h"
#include "TransactionExtensions.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/crypto/MerkleHashBuilder.h"
#include "catapult/model/Block.h"
#include "catapult/model/BlockUtils.h"
#include "catapult/model/EntityHasher.h"
#include "catapult/model/TransactionPlugin.h"

namespace catapult { namespace extensions {

	BlockExtensions::BlockExtensions()
			: m_calculateTransactionEntityHash([](const auto& transaction) {
				return model::CalculateHash(transaction);
			})
			, m_calculateTransactionMerkleComponentHash([](const auto&, const auto& entityHash) {
				return entityHash;
			})
	{}

	BlockExtensions::BlockExtensions(const model::TransactionRegistry& transactionRegistry)
			: m_calculateTransactionEntityHash([&transactionRegistry](const auto& transaction) {
				const auto& plugin = *transactionRegistry.findPlugin(transaction.Type);
				return model::CalculateHash(transaction, plugin.dataBuffer(transaction));
			})
			, m_calculateTransactionMerkleComponentHash([&transactionRegistry](const auto& transaction, const auto& entityHash) {
				return model::CalculateMerkleComponentHash(transaction, entityHash, transactionRegistry);
			})
	{}

	void BlockExtensions::updateBlockTransactionsHash(model::Block& block) const {
		calculateBlockTransactionsHash(block, block.BlockTransactionsHash);
	}

	void BlockExtensions::calculateBlockTransactionsHash(const model::Block& block, Hash256& blockTransactionsHash) const {
		crypto::MerkleHashBuilder builder;
		for (const auto& transaction : block.Transactions()) {
			auto transactionHash = m_calculateTransactionEntityHash(transaction);
			auto merkleComponentHash = m_calculateTransactionMerkleComponentHash(transaction, transactionHash);
			builder.update(merkleComponentHash);
		}

		builder.final(blockTransactionsHash);
	}

	void BlockExtensions::signFullBlock(const crypto::KeyPair& signer, model::Block& block) const {
		// calculate the block transactions hash
		updateBlockTransactionsHash(block);

		// sign the block
		model::SignBlockHeader(signer, block);
	}

	VerifyFullBlockResult BlockExtensions::verifyFullBlock(const model::Block& block) const {
		// check block header signature
		if (!model::VerifyBlockHeaderSignature(block))
			return VerifyFullBlockResult::Invalid_Block_Signature;

		// check block transactions hash
		Hash256 expectedBlockTransactionsHash;
		calculateBlockTransactionsHash(block, expectedBlockTransactionsHash);
		if (expectedBlockTransactionsHash != block.BlockTransactionsHash)
			return VerifyFullBlockResult::Invalid_Block_Transactions_Hash;

		// check transaction signatures
		for (const auto& transaction : block.Transactions()) {
			if (!VerifyTransactionSignature(transaction))
				return VerifyFullBlockResult::Invalid_Transaction_Signature;
		}

		return VerifyFullBlockResult::Success;
	}

	model::BlockElement BlockExtensions::convertBlockToBlockElement(const model::Block& block, const Hash256& generationHash) const {
		model::BlockElement blockElement(block);
		blockElement.EntityHash = model::CalculateHash(block);
		blockElement.GenerationHash = generationHash;

		for (const auto& transaction : block.Transactions()) {
			blockElement.Transactions.push_back(model::TransactionElement(transaction));

			auto& transactionElement = blockElement.Transactions.back();
			transactionElement.EntityHash = m_calculateTransactionEntityHash(transaction);
			transactionElement.MerkleComponentHash = m_calculateTransactionMerkleComponentHash(transaction, transactionElement.EntityHash);
		}

		return blockElement;
	}
}}
