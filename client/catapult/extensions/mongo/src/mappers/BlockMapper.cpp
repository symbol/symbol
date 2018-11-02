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
		void StreamHashArray(bson_stream::document& builder, const std::string& name, const std::vector<Hash256>& hashes) {
			auto hashArray = builder << name << bson_stream::open_array;
			for (const auto& hash : hashes)
				hashArray << ToBinary(hash);

			hashArray << bson_stream::close_array;
		}

		auto& StreamBlockMetadata(
				bson_stream::document& builder,
				const model::BlockElement& blockElement,
				const std::vector<Hash256>& merkleTree,
				Amount totalFee,
				int32_t numTransactions) {
			builder << "meta"
					<< bson_stream::open_document
						<< "hash" << ToBinary(blockElement.EntityHash)
						<< "generationHash" << ToBinary(blockElement.GenerationHash)
						<< "totalFee" << ToInt64(totalFee)
						<< "numTransactions" << numTransactions;

			StreamHashArray(builder, "subCacheMerkleRoots", blockElement.SubCacheMerkleRoots);
			StreamHashArray(builder, "merkleTree", merkleTree);

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
		StreamBlockMetadata(builder, blockElement, merkleTree, totalFee, numTransactions);

		// block data
		builder << "block" << bson_stream::open_document;
		StreamVerifiableEntity(builder, block)
				<< "height" << ToInt64(block.Height)
				<< "timestamp" << ToInt64(block.Timestamp)
				<< "difficulty" << ToInt64(block.Difficulty)
				<< "previousBlockHash" << ToBinary(block.PreviousBlockHash)
				<< "blockTransactionsHash" << ToBinary(block.BlockTransactionsHash)
				<< "stateHash" << ToBinary(block.StateHash);
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
