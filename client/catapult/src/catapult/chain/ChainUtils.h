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
#include "catapult/model/Block.h"
#include "catapult/model/ChainScore.h"
#include <vector>

namespace catapult {
	namespace cache { class BlockStatisticCache; }
	namespace model { struct BlockChainConfiguration; }
}

namespace catapult { namespace chain {

	/// Determines if \a parent with hash \a parentHash and \a child form a chain link.
	bool IsChainLink(const model::Block& parent, const Hash256& parentHash, const model::Block& child);

	/// Checks if the difficulties in \a blocks are consistent with the difficulties stored in \a cache
	/// for the block chain described by \a config. If there is an inconsistency, the index of the first
	/// difference is returned. Otherwise, the size of \a blocks is returned.
	size_t CheckDifficulties(
			const cache::BlockStatisticCache& cache,
			const std::vector<const model::Block*>& blocks,
			const model::BlockChainConfiguration& config);

	/// Calculates the partial chain score of \a blocks starting at \a parent.
	model::ChainScore CalculatePartialChainScore(const model::Block& parent, const std::vector<const model::Block*>& blocks);
}}
