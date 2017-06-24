#pragma once
#include "catapult/model/Block.h"
#include "catapult/model/ChainScore.h"
#include "catapult/types.h"
#include <vector>

namespace catapult {
	namespace cache { class BlockDifficultyCache; }
	namespace model { struct BlockChainConfiguration; }
}

namespace catapult { namespace chain {

	/// Determines if \a parent with hash \a parentHash and \a child form a chain link.
	bool IsChainLink(const model::Block& parent, const Hash256& parentHash, const model::Block& child);

	/// Checks if the difficulties in \a blocks are consistent with the difficulties stored in \a cache
	/// for the block chain described by \a config. If there is an inconsistency, the index of the first
	/// difference is returned. Otherwise, the size of \a blocks is returned.
	size_t CheckDifficulties(
			const cache::BlockDifficultyCache& cache,
			const std::vector<const model::Block*>& blocks,
			const model::BlockChainConfiguration& config);

	/// Calculates the partial chain score of \a blocks starting at \a parent.
	model::ChainScore CalculatePartialChainScore(
			const model::Block& parent,
			const std::vector<const model::Block*>& blocks);
}}
