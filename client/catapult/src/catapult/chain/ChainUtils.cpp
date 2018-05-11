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

#include "ChainUtils.h"
#include "BlockDifficultyScorer.h"
#include "BlockScorer.h"
#include "catapult/cache_core/BlockDifficultyCache.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/model/BlockUtils.h"

namespace catapult { namespace chain {

	bool IsChainLink(const model::Block& parent, const Hash256& parentHash, const model::Block& child) {
		if (parent.Height + Height(1) != child.Height || parentHash != child.PreviousBlockHash)
			return false;

		return parent.Timestamp < child.Timestamp;
	}

	namespace {
		using DifficultySet = cache::BlockDifficultyCacheTypes::PrimaryTypes::BaseSetType::SetType::MemorySetType;

		DifficultySet LoadDifficulties(
				const cache::BlockDifficultyCache& cache,
				Height height,
				const model::BlockChainConfiguration& config) {
			auto view = cache.createView();
			auto range = view->difficultyInfos(height, config.MaxDifficultyBlocks);

			DifficultySet set;
			set.insert(range.begin(), range.end());
			return set;
		}

		Difficulty CalculateDifficulty(const DifficultySet& difficulties, const model::BlockChainConfiguration& config) {
			return chain::CalculateDifficulty(cache::DifficultyInfoRange(difficulties.cbegin(), difficulties.cend()), config);
		}
	}

	size_t CheckDifficulties(
			const cache::BlockDifficultyCache& cache,
			const std::vector<const model::Block*>& blocks,
			const model::BlockChainConfiguration& config) {
		if (blocks.empty())
			return 0;

		auto difficulties = LoadDifficulties(cache, blocks[0]->Height - Height(1), config);
		auto difficulty = CalculateDifficulty(difficulties, config);

		size_t i = 0;
		for (const auto* pBlock : blocks) {
			if (difficulty != pBlock->Difficulty)
				break;

			difficulties.insert(state::BlockDifficultyInfo(pBlock->Height, pBlock->Timestamp, difficulty));

			if (difficulties.size() > config.MaxDifficultyBlocks)
				difficulties.erase(difficulties.cbegin());

			difficulty = CalculateDifficulty(difficulties, config);
			++i;
		}

		if (i != blocks.size()) {
			CATAPULT_LOG(warning) << "difficulties diverge at " << i << " of " << blocks.size()
					<< " (height " << (blocks.front()->Height + Height(i)) << ")";
		}

		return i;
	}

	model::ChainScore CalculatePartialChainScore(const model::Block& parent, const std::vector<const model::Block*>& blocks) {
		model::ChainScore score;
		auto pPreviousBlock = &parent;
		for (const auto* pBlock : blocks) {
			score += model::ChainScore(CalculateScore(*pPreviousBlock, *pBlock));
			pPreviousBlock = pBlock;
		}

		return score;
	}
}}
