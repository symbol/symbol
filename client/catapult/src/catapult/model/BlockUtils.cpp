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

#include "BlockUtils.h"
#include "FeeUtils.h"
#include "TransactionPlugin.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/crypto/MerkleHashBuilder.h"
#include "catapult/crypto/Signer.h"
#include "catapult/utils/IntegerMath.h"
#include "catapult/utils/MemoryUtils.h"
#include <cstring>

namespace catapult { namespace model {

	// region hashes

	void CalculateBlockTransactionsHash(const std::vector<const TransactionInfo*>& transactionInfos, Hash256& blockTransactionsHash) {
		crypto::MerkleHashBuilder builder;
		for (const auto* pTransactionInfo : transactionInfos)
			builder.update(pTransactionInfo->MerkleComponentHash);

		builder.final(blockTransactionsHash);
	}

	GenerationHash CalculateGenerationHash(const crypto::ProofGamma& gamma) {
		auto proofHash = GenerateVrfProofHash(gamma);
		return proofHash.copyTo<GenerationHash>();
	}

	// endregion

	// region block type

	model::EntityType CalculateBlockTypeFromHeight(Height height, uint64_t importanceGrouping) {
		if (Height(1) == height)
			return model::Entity_Type_Block_Nemesis;

		return 0 == height.unwrap() % importanceGrouping ? model::Entity_Type_Block_Importance : model::Entity_Type_Block_Normal;
	}

	// endregion

	// region block transactions info

	namespace {
		ExtendedBlockTransactionsInfo CalculateBlockTransactionsInfo(const Block& block, const TransactionRegistry* pTransactionRegistry) {
			ExtendedBlockTransactionsInfo blockTransactionsInfo;
			for (const auto& transaction : block.Transactions()) {
				auto transactionFee = CalculateTransactionFee(block.FeeMultiplier, transaction);
				blockTransactionsInfo.TotalFee = blockTransactionsInfo.TotalFee + transactionFee;
				++blockTransactionsInfo.Count;

				if (!pTransactionRegistry)
					continue;

				const auto* pPlugin = pTransactionRegistry->findPlugin(transaction.Type);
				if (pPlugin)
					blockTransactionsInfo.DeepCount += 1 + pPlugin->embeddedCount(transaction);
				else
					CATAPULT_LOG(warning) << "skipping transaction with unknown type " << transaction.Type;
			}

			return blockTransactionsInfo;
		}
	}

	BlockTransactionsInfo CalculateBlockTransactionsInfo(const Block& block) {
		return CalculateBlockTransactionsInfo(block, nullptr);
	}

	ExtendedBlockTransactionsInfo CalculateBlockTransactionsInfo(const Block& block, const TransactionRegistry& transactionRegistry) {
		return CalculateBlockTransactionsInfo(block, &transactionRegistry);
	}

	// endregion

	// region sign / verify

	void SignBlockHeader(const crypto::KeyPair& signer, Block& block) {
		crypto::Sign(signer, GetBlockHeaderDataBuffer(block), block.Signature);
	}

	bool VerifyBlockHeaderSignature(const Block& block) {
		return crypto::Verify(block.SignerPublicKey, GetBlockHeaderDataBuffer(block), block.Signature);
	}

	// endregion

	// region create block

	namespace {
		template<typename TContainer>
		void CopyTransactions(uint8_t* pDestination, const TContainer& transactions) {
			for (auto i = 0u; i < transactions.size(); ++i) {
				const auto& pTransaction = transactions[i];
				std::memcpy(pDestination, pTransaction.get(), pTransaction->Size);
				pDestination += pTransaction->Size;

				if (i < transactions.size() - 1) {
					auto paddingSize = utils::GetPaddingSize(pTransaction->Size, 8);
					std::memset(static_cast<void*>(pDestination), 0, paddingSize);
					pDestination += paddingSize;
				}
			}
		}

		template<typename TContainer>
		uint32_t CalculateTotalSize(const TContainer& transactions) {
			uint32_t totalTransactionsSize = 0;
			uint32_t lastPaddingSize = 0;
			for (const auto& pTransaction : transactions) {
				lastPaddingSize = utils::GetPaddingSize(pTransaction->Size, 8);
				totalTransactionsSize += pTransaction->Size + lastPaddingSize;
			}

			return totalTransactionsSize - lastPaddingSize;
		}

		template<typename TContainer>
		std::unique_ptr<Block> CreateBlockT(
				EntityType blockType,
				const PreviousBlockContext& context,
				NetworkIdentifier networkIdentifier,
				const Key& signerPublicKey,
				const TContainer& transactions) {
			auto headerSize = GetBlockHeaderSize(blockType, Block::Current_Version);
			auto size = headerSize + CalculateTotalSize(transactions);
			auto pBlock = utils::MakeUniqueWithSize<Block>(size);
			std::memset(static_cast<void*>(pBlock.get()), 0, headerSize);
			pBlock->Size = size;

			pBlock->SignerPublicKey = signerPublicKey;

			pBlock->Version = Block::Current_Version;
			pBlock->Network = networkIdentifier;
			pBlock->Type = blockType;

			pBlock->Height = context.BlockHeight + Height(1);
			pBlock->Difficulty = Difficulty();
			pBlock->PreviousBlockHash = context.BlockHash;

			pBlock->BeneficiaryAddress = GetSignerAddress(*pBlock);

			// append all the transactions
			auto* pDestination = reinterpret_cast<uint8_t*>(pBlock->TransactionsPtr());
			CopyTransactions(pDestination, transactions);
			return pBlock;
		}
	}

	std::unique_ptr<Block> CreateBlock(
			EntityType blockType,
			const PreviousBlockContext& context,
			NetworkIdentifier networkIdentifier,
			const Key& signerPublicKey,
			const Transactions& transactions) {
		return CreateBlockT(blockType, context, networkIdentifier, signerPublicKey, transactions);
	}

	std::unique_ptr<Block> StitchBlock(const BlockHeader& blockHeader, const Transactions& transactions) {
		auto headerSize = GetBlockHeaderSize(blockHeader.Type, blockHeader.Version);
		auto size = headerSize + CalculateTotalSize(transactions);
		auto pBlock = utils::MakeUniqueWithSize<Block>(size);
		auto* pBlockData = reinterpret_cast<uint8_t*>(pBlock.get());

		// only copy BlockHeader and zero header footer
		std::memcpy(pBlockData, &blockHeader, sizeof(BlockHeader));
		std::memset(pBlockData + sizeof(BlockHeader), 0, headerSize - sizeof(BlockHeader));
		pBlock->Size = static_cast<uint32_t>(size);

		// append all the transactions
		auto* pDestination = reinterpret_cast<uint8_t*>(pBlock->TransactionsPtr());
		CopyTransactions(pDestination, transactions);
		return pBlock;
	}

	// endregion
}}
