#include "BlockMapper.h"
#include "MapperUtils.h"
#include "catapult/model/Elements.h"

namespace catapult { namespace mongo { namespace mappers {

	namespace {
		auto& StreamBlockMetadata(
				bson_stream::document& builder,
				const Hash256& hash,
				const Hash256& generationHash,
				Amount totalFee,
				int32_t numTransactions) {
			builder << "meta"
					<< bson_stream::open_document
						<< "hash" << ToBinary(hash)
						<< "generationHash" << ToBinary(generationHash)
						<< "totalFee" << ToInt64(totalFee)
						<< "numTransactions" << numTransactions
					<< bson_stream::close_document;
			return builder;
		}
	}

	bsoncxx::document::value ToDbModel(const model::BlockElement& blockElement) {
		Amount totalFee;
		int32_t numTransactions = 0;
		const auto& block = blockElement.Block;
		for (const auto& transaction : block.Transactions()) {
			totalFee = totalFee + transaction.Fee;
			++numTransactions;
		}

		// block metadata
		bson_stream::document builder;
		StreamBlockMetadata(builder, blockElement.EntityHash, blockElement.GenerationHash, totalFee, numTransactions);

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
