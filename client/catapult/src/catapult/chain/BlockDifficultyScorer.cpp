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

#include "BlockDifficultyScorer.h"
#include <boost/multiprecision/cpp_int.hpp>

namespace catapult { namespace chain {

	Difficulty CalculateDifficulty(const cache::BlockStatisticRange& statistics, const model::BlockChainConfiguration& config) {
		// note that statistics is sorted by both heights and timestamps, so the first info has the smallest
		// height and earliest timestamp and the last info has the largest height and latest timestamp
		size_t historySize = 0;
		Difficulty::ValueType averageDifficulty = 0;
		for (const auto& statistic : statistics) {
			++historySize;
			averageDifficulty += statistic.Difficulty.unwrap();
		}

		if (historySize < 2)
			return Difficulty();

		auto firstTimestamp = statistics.begin()->Timestamp;

		const auto& lastStatistic = *(--statistics.end());
		auto lastTimestamp = lastStatistic.Timestamp;
		auto lastDifficulty = lastStatistic.Difficulty.unwrap();

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
				const cache::BlockStatisticCacheView& view,
				Height height,
				const model::BlockChainConfiguration& config) {
			auto statistics = view.statistics(height, config.MaxDifficultyBlocks);
			return chain::CalculateDifficulty(statistics, config);
		}
	}

	Difficulty CalculateDifficulty(const cache::BlockStatisticCache& cache, Height height, const model::BlockChainConfiguration& config) {
		auto view = cache.createView();
		return CalculateDifficulty(*view, height, config);
	}

	bool TryCalculateDifficulty(
			const cache::BlockStatisticCache& cache,
			Height height,
			const model::BlockChainConfiguration& config,
			Difficulty& difficulty) {
		auto view = cache.createView();
		if (!view->contains(state::BlockStatistic(height)))
			return false;

		difficulty = CalculateDifficulty(*view, height, config);
		return true;
	}
}}
