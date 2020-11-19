/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#include "BlockExtensions.h"
#include "TransactionExtensions.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/crypto/MerkleHashBuilder.h"
#include "catapult/model/Block.h"
#include "catapult/model/BlockUtils.h"
#include "catapult/model/EntityHasher.h"
#include "catapult/model/TransactionPlugin.h"

namespace catapult { namespace extensions {

	BlockExtensions::BlockExtensions(const GenerationHashSeed& generationHashSeed)
			: m_generationHashSeed(generationHashSeed)
			, m_calculateTransactionEntityHash([generationHashSeed](const auto& transaction) {
				return model::CalculateHash(transaction, generationHashSeed);
			})
			, m_calculateTransactionMerkleComponentHash([](const auto&, const auto& entityHash) {
				return entityHash;
			})
	{}

	BlockExtensions::BlockExtensions(const GenerationHashSeed& generationHashSeed, const model::TransactionRegistry& transactionRegistry)
			: m_generationHashSeed(generationHashSeed)
			, m_calculateTransactionEntityHash([generationHashSeed, &transactionRegistry](const auto& transaction) {
				const auto& plugin = *transactionRegistry.findPlugin(transaction.Type);
				return model::CalculateHash(transaction, generationHashSeed, plugin.dataBuffer(transaction));
			})
			, m_calculateTransactionMerkleComponentHash([&transactionRegistry](const auto& transaction, const auto& entityHash) {
				return model::CalculateMerkleComponentHash(transaction, entityHash, transactionRegistry);
			})
	{}

	void BlockExtensions::updateBlockTransactionsHash(model::Block& block) const {
		calculateBlockTransactionsHash(block, block.TransactionsHash);
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
		if (expectedBlockTransactionsHash != block.TransactionsHash)
			return VerifyFullBlockResult::Invalid_Block_Transactions_Hash;

		// check transaction signatures
		TransactionExtensions transactionExtensions(m_generationHashSeed);
		for (const auto& transaction : block.Transactions()) {
			if (!transactionExtensions.verify(transaction))
				return VerifyFullBlockResult::Invalid_Transaction_Signature;
		}

		return VerifyFullBlockResult::Success;
	}

	model::BlockElement BlockExtensions::convertBlockToBlockElement(
			const model::Block& block,
			const GenerationHash& generationHash) const {
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
