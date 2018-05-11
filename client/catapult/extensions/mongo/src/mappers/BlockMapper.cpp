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

#include "BlockMapper.h"
#include "MapperUtils.h"
#include "catapult/crypto/MerkleHashBuilder.h"
#include "catapult/model/Elements.h"
#include "catapult/model/EntityHasher.h"

namespace catapult { namespace mongo { namespace mappers {

	namespace {
		void StreamMerkleTree(bson_stream::document& builder, const std::vector<Hash256>& merkleTree) {
			auto merkleTreeArray = builder << "merkleTree" << bson_stream::open_array;
			for (const auto& hash : merkleTree)
				merkleTreeArray << ToBinary(hash);

			merkleTreeArray << bson_stream::close_array;
		}

		auto& StreamBlockMetadata(
				bson_stream::document& builder,
				const Hash256& hash,
				const Hash256& generationHash,
				const std::vector<Hash256>& merkleTree,
				Amount totalFee,
				int32_t numTransactions) {
			builder << "meta"
					<< bson_stream::open_document
						<< "hash" << ToBinary(hash)
						<< "generationHash" << ToBinary(generationHash)
						<< "totalFee" << ToInt64(totalFee)
						<< "numTransactions" << numTransactions;

			StreamMerkleTree(builder, merkleTree);

			builder << bson_stream::close_document;
			return builder;
		}
	}

	bsoncxx::document::value ToDbModel(const model::BlockElement& blockElement) {
		auto merkleTree = model::CalculateMerkleTree(blockElement.Transactions);

		Amount totalFee;
		int32_t numTransactions = 0;
		const auto& block = blockElement.Block;
		for (const auto& transaction : block.Transactions()) {
			totalFee = totalFee + transaction.Fee;
			++numTransactions;
		}

		// block metadata
		bson_stream::document builder;
		StreamBlockMetadata(builder, blockElement.EntityHash, blockElement.GenerationHash, merkleTree, totalFee, numTransactions);

		// block data
		builder << "block" << bson_stream::open_document;
		StreamVerifiableEntity(builder, block)
				<< "height" << ToInt64(block.Height)
				<< "timestamp" << ToInt64(block.Timestamp)
				<< "difficulty" << ToInt64(block.Difficulty)
				<< "previousBlockHash" << ToBinary(block.PreviousBlockHash)
				<< "blockTransactionsHash" << ToBinary(block.BlockTransactionsHash);
		builder << bson_stream::close_document;
		return builder << bson_stream::finalize;
	}

	state::BlockDifficultyInfo ToDifficultyInfo(const bsoncxx::document::view& document) {
		state::BlockDifficultyInfo difficultyInfo;
		difficultyInfo.BlockHeight = GetValue64<Height>(document["height"]);
		difficultyInfo.BlockDifficulty = GetValue64<Difficulty>(document["difficulty"]);
		difficultyInfo.BlockTimestamp = GetValue64<Timestamp>(document["timestamp"]);
		return difficultyInfo;
	}
}}}
