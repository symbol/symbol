#include "BlockDifficultyScorer.h"
#include <boost/multiprecision/cpp_int.hpp>

namespace catapult { namespace chain {

	Difficulty CalculateDifficulty(const cache::DifficultyInfoRange& difficultyInfos, const model::BlockChainConfiguration& config) {
		// note that difficultyInfos is sorted by both heights and timestamps, so the first info has the smallest
		// height and earliest timestamp and the last info has the largest height and latest timestamp
		size_t historySize = 0;
		Difficulty::ValueType averageDifficulty = 0;
		for (const auto& difficultyInfo : difficultyInfos) {
			++historySize;
			averageDifficulty += difficultyInfo.BlockDifficulty.unwrap();
		}

		if (historySize < 2)
			return Difficulty();

		auto firstTimestamp = difficultyInfos.begin()->BlockTimestamp;

		const auto& lastInfo = *(--difficultyInfos.end());
		auto lastTimestamp = lastInfo.BlockTimestamp;
		auto lastDifficulty = lastInfo.BlockDifficulty.unwrap();

		auto timeDiff = (lastTimestamp - firstTimestamp).unwrap();
		averageDifficulty /= historySize;

		boost::multiprecision::uint128_t largeDifficulty = averageDifficulty;
		largeDifficulty *= config.BlockGenerationTargetTime.millis();
		largeDifficulty *= (historySize - 1);
		largeDifficulty /= timeDiff;
		auto difficulty = static_cast<uint64_t>(largeDifficulty);

		// clamp difficulty changes to 5%
		if (19 * lastDifficulty > 20 * difficulty)
			difficulty = (19 * lastDifficulty) / 20;
		else if (21 * lastDifficulty < 20 * difficulty)
			difficulty = (21 * lastDifficulty) / 20;

		return Difficulty(difficulty);
	}

	namespace {
		Difficulty CalculateDifficulty(
				const cache::BlockDifficultyCacheView& view,
				Height height,
				const model::BlockChainConfiguration& config) {
			auto infos = view.difficultyInfos(height, config.MaxDifficultyBlocks);
			return chain::CalculateDifficulty(infos, config);
		}
	}

	Difficulty CalculateDifficulty(const cache::BlockDifficultyCache& cache, Height height, const model::BlockChainConfiguration& config) {
		auto view = cache.createView();
		return CalculateDifficulty(*view, height, config);
	}

	bool TryCalculateDifficulty(
			const cache::BlockDifficultyCache& cache,
			Height height,
			const model::BlockChainConfiguration& config,
			Difficulty& difficulty) {
		auto view = cache.createView();
		if (!view->contains(state::BlockDifficultyInfo(height)))
			return false;

		difficulty = CalculateDifficulty(*view, height, config);
		return true;
	}
}}
