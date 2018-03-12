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
		using DifficultySet = cache::BlockDifficultyCacheTypes::BaseSetType::SetType;

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
