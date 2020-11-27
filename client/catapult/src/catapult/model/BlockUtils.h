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

#pragma once
#include "Block.h"
#include "Elements.h"
#include "EntityInfo.h"

namespace catapult { namespace model {

	// region hashes

	/// Calculates the block transactions hash of \a transactionInfos into \a blockTransactionsHash.
	void CalculateBlockTransactionsHash(const std::vector<const TransactionInfo*>& transactionInfos, Hash256& blockTransactionsHash);

	/// Calculates the generation hash from \a gamma.
	GenerationHash CalculateGenerationHash(const crypto::ProofGamma& gamma);

	// endregion

	// region block type

	/// Calculates the block type at \a height given \a importanceGrouping.
	model::EntityType CalculateBlockTypeFromHeight(Height height, uint64_t importanceGrouping);

	// endregion

	// region block transactions info

	/// Information about transactions stored in a block.
	struct BlockTransactionsInfo {
		/// Number of (top-level) transactions.
		uint32_t Count = 0;

		/// Total fee of all transactions.
		Amount TotalFee;
	};

	/// Information about transactions stored in a block, including transaction registry dependent information.
	struct ExtendedBlockTransactionsInfo : public BlockTransactionsInfo {
		/// Number of transactions (including embedded transactions).
		uint32_t DeepCount = 0;
	};

	/// Calculates information about transactions stored in \a block.
	BlockTransactionsInfo CalculateBlockTransactionsInfo(const Block& block);

	/// Calculates information about transactions stored in \a block given \a transactionRegistry.
	ExtendedBlockTransactionsInfo CalculateBlockTransactionsInfo(const Block& block, const TransactionRegistry& transactionRegistry);

	// endregion

	// region sign / verify

	/// Signs \a block header as \a signer.
	/// \note All header data is assumed to be present and valid.
	void SignBlockHeader(const crypto::KeyPair& signer, Block& block);

	/// Validates signature of \a block header.
	bool VerifyBlockHeaderSignature(const Block& block);

	// endregion

	// region create block

	/// Container of transactions.
	using Transactions = std::vector<std::shared_ptr<const Transaction>>;

	/// Context passed when creating new block.
	struct PreviousBlockContext {
		/// Creates an empty context.
		PreviousBlockContext()
				: BlockHash()
				, GenerationHash()
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
		catapult::GenerationHash GenerationHash;

		/// Height of previous block.
		Height BlockHeight;

		/// Timestamp of previous block.
		catapult::Timestamp Timestamp;
	};

	/// Creates an unsigned Block with \a blockType given \a context, signer public key (\a signerPublicKey) and \a transactions
	/// for a network with identifier \a networkIdentifier.
	std::unique_ptr<Block> CreateBlock(
			EntityType blockType,
			const PreviousBlockContext& context,
			NetworkIdentifier networkIdentifier,
			const Key& signerPublicKey,
			const Transactions& transactions);

	/// Creates a new block by stitching together \a blockHeader and \a transactions.
	std::unique_ptr<Block> StitchBlock(const BlockHeader& blockHeader, const Transactions& transactions);

	// endregion
}}
