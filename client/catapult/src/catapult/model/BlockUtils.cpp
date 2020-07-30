/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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
#include "catapult/crypto/Hashes.h"
#include "catapult/crypto/MerkleHashBuilder.h"
#include "catapult/crypto/Signer.h"
#include "catapult/utils/IntegerMath.h"
#include "catapult/utils/MemoryUtils.h"
#include <cstring>

namespace catapult { namespace model {

	namespace {
		RawBuffer BlockDataBuffer(const Block& block) {
			return {
				reinterpret_cast<const uint8_t*>(&block) + VerifiableEntity::Header_Size,
				sizeof(BlockHeader) - VerifiableEntity::Header_Size - Block::Footer_Size
			};
		}
	}

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

	// region sign / verify

	void SignBlockHeader(const crypto::KeyPair& signer, Block& block) {
		crypto::Sign(signer, BlockDataBuffer(block), block.Signature);
	}

	bool VerifyBlockHeaderSignature(const Block& block) {
		return crypto::Verify(block.SignerPublicKey, BlockDataBuffer(block), block.Signature);
	}

	// endregion

	// region fees

	BlockTransactionsInfo CalculateBlockTransactionsInfo(const Block& block) {
		BlockTransactionsInfo blockTransactionsInfo;
		for (const auto& transaction : block.Transactions()) {
			auto transactionFee = CalculateTransactionFee(block.FeeMultiplier, transaction);
			blockTransactionsInfo.TotalFee = blockTransactionsInfo.TotalFee + transactionFee;
			++blockTransactionsInfo.Count;
		}

		return blockTransactionsInfo;
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
				const PreviousBlockContext& context,
				NetworkIdentifier networkIdentifier,
				const Key& signerPublicKey,
				const TContainer& transactions) {
			uint32_t size = SizeOf32<BlockHeader>() + CalculateTotalSize(transactions);
			auto pBlock = utils::MakeUniqueWithSize<Block>(size);
			std::memset(static_cast<void*>(pBlock.get()), 0, sizeof(BlockHeader));
			pBlock->Size = size;

			pBlock->SignerPublicKey = signerPublicKey;

			pBlock->Version = Block::Current_Version;
			pBlock->Network = networkIdentifier;
			pBlock->Type = Entity_Type_Block;

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
			const PreviousBlockContext& context,
			NetworkIdentifier networkIdentifier,
			const Key& signerPublicKey,
			const Transactions& transactions) {
		return CreateBlockT(context, networkIdentifier, signerPublicKey, transactions);
	}

	std::unique_ptr<Block> StitchBlock(const BlockHeader& blockHeader, const Transactions& transactions) {
		auto size = sizeof(BlockHeader) + CalculateTotalSize(transactions);
		auto pBlock = utils::MakeUniqueWithSize<Block>(size);
		std::memcpy(static_cast<void*>(pBlock.get()), &blockHeader, sizeof(BlockHeader));
		pBlock->Size = static_cast<uint32_t>(size);

		// append all the transactions
		auto* pDestination = reinterpret_cast<uint8_t*>(pBlock->TransactionsPtr());
		CopyTransactions(pDestination, transactions);
		return pBlock;
	}

	// endregion
}}
