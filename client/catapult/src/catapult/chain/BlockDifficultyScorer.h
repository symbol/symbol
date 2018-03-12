#pragma once
#include "catapult/cache_core/BlockDifficultyCache.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/types.h"

namespace catapult { namespace chain {

	/// Calculates the block difficulty given the past difficulties and timestamps (\a difficultyInfos) for the
	/// block chain described by \a config.
	Difficulty CalculateDifficulty(const cache::DifficultyInfoRange& difficultyInfos, const model::BlockChainConfiguration& config);

	/// Calculates the block difficulty at \a height for the block chain described by \a config
	/// given the block difficulty \a cache.
	Difficulty CalculateDifficulty(const cache::BlockDifficultyCache& cache, Height height, const model::BlockChainConfiguration& config);

	/// Calculates the block difficulty at \a height into \a difficulty for the block chain described by
	/// \a config given the block difficulty \a cache.
	bool TryCalculateDifficulty(
			const cache::BlockDifficultyCache& cache,
			Height height,
			const model::BlockChainConfiguration& config,
			Difficulty& difficulty);
}}
