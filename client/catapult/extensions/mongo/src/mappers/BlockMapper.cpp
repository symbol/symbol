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

#include "BlockMapper.h"
#include "MapperUtils.h"
#include "catapult/crypto/MerkleHashBuilder.h"
#include "catapult/model/BlockUtils.h"
#include "catapult/model/Elements.h"
#include "catapult/model/EntityHasher.h"
#include "catapult/model/FinalizationRound.h"

namespace catapult { namespace mongo { namespace mappers {

	// region ToDbModel (block)

	namespace {
		void StreamHashArray(bson_stream::document& builder, const std::string& name, const std::vector<Hash256>& hashes) {
			auto hashArray = builder << name << bson_stream::open_array;
			for (const auto& hash : hashes)
				hashArray << ToBinary(hash);

			hashArray << bson_stream::close_array;
		}

		void StreamBlockMerkleTree(
				bson_stream::document& builder,
				const std::string& countLabel,
				uint32_t count,
				const std::string& merkleTreeLabel,
				const std::vector<Hash256>& merkleTree) {
			builder << countLabel << static_cast<int32_t>(count);
			StreamHashArray(builder, merkleTreeLabel, merkleTree);
		}
	}

	bsoncxx::document::value ToDbModel(const model::BlockElement& blockElement, uint32_t totalTransactionsCount) {
		const auto& block = blockElement.Block;
		auto blockTransactionsInfo = model::CalculateBlockTransactionsInfo(block);
		auto transactionMerkleTree = model::CalculateMerkleTree(blockElement.Transactions);

		// block metadata
		bson_stream::document builder;

		builder
				<< "meta" << bson_stream::open_document
					<< "hash" << ToBinary(blockElement.EntityHash)
					<< "generationHash" << ToBinary(blockElement.GenerationHash)
					<< "totalFee" << ToInt64(blockTransactionsInfo.TotalFee)
					<< "totalTransactionsCount" << static_cast<int32_t>(totalTransactionsCount);
		StreamHashArray(builder, "stateHashSubCacheMerkleRoots", blockElement.SubCacheMerkleRoots);
		StreamBlockMerkleTree(builder, "transactionsCount", blockTransactionsInfo.Count, "transactionMerkleTree", transactionMerkleTree);

		if (blockElement.OptionalStatement) {
			const auto& blockStatement = *blockElement.OptionalStatement;
			auto statementsCount = static_cast<uint32_t>(model::CountTotalStatements(blockStatement));
			auto statementMerkleTree = model::CalculateMerkleTree(blockStatement);
			StreamBlockMerkleTree(builder, "statementsCount", statementsCount, "statementMerkleTree", statementMerkleTree);
		}

		builder << bson_stream::close_document;

		// block data
		builder << "block" << bson_stream::open_document;
		auto blockDocument = StreamVerifiableEntity(builder, block)
				<< "height" << ToInt64(block.Height)
				<< "timestamp" << ToInt64(block.Timestamp)
				<< "difficulty" << ToInt64(block.Difficulty)
				<< "proofGamma" << ToBinary(block.GenerationHashProof.Gamma)
				<< "proofVerificationHash" << ToBinary(block.GenerationHashProof.VerificationHash)
				<< "proofScalar" << ToBinary(block.GenerationHashProof.Scalar)
				<< "previousBlockHash" << ToBinary(block.PreviousBlockHash)
				<< "transactionsHash" << ToBinary(block.TransactionsHash)
				<< "receiptsHash" << ToBinary(block.ReceiptsHash)
				<< "stateHash" << ToBinary(block.StateHash)
				<< "beneficiaryAddress" << ToBinary(block.BeneficiaryAddress)
				<< "feeMultiplier" << ToInt32(block.FeeMultiplier);

		if (model::IsImportanceBlock(block.Type, block.Version)) {
			const auto& blockFooter = model::GetBlockFooter<model::ImportanceBlockFooter>(block);
			blockDocument
					<< "votingEligibleAccountsCount" << static_cast<int32_t>(blockFooter.VotingEligibleAccountsCount)
					<< "harvestingEligibleAccountsCount" << static_cast<int64_t>(blockFooter.HarvestingEligibleAccountsCount)
					<< "totalVotingBalance" << ToInt64(blockFooter.TotalVotingBalance)
					<< "previousImportanceBlockHash" << ToBinary(blockFooter.PreviousImportanceBlockHash);
		}

		builder << bson_stream::close_document;
		return builder << bson_stream::finalize;
	}

	// endregion

	// region ToDbModel (finalized block)

	bsoncxx::document::value ToDbModel(const model::FinalizationRound& round, Height height, const Hash256& hash) {
		bson_stream::document builder;
		return builder
				<< "block" << bson_stream::open_document
					<< "finalizationEpoch" << ToInt32(round.Epoch)
					<< "finalizationPoint" << ToInt32(round.Point)
					<< "height" << ToInt64(height)
					<< "hash" << ToBinary(hash)
				<< bson_stream::close_document
				<< bson_stream::finalize;
	}

	// endregion
}}}
