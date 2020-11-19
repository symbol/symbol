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

#include "catapult/chain/BlockDifficultyScorer.h"
#include "catapult/utils/Logging.h"
#include "tests/TestHarness.h"
#include <cmath>

namespace catapult { namespace chain {

#define TEST_CLASS BlockDifficultyScorerTests

	namespace {
		using StatisticSet = cache::BlockStatisticCacheTypes::PrimaryTypes::BaseSetType::SetType::MemorySetType;

		constexpr Difficulty Base_Difficulty = Difficulty(100'000'000'000'000);

		state::BlockStatistic CreateStatistic(Height height, Timestamp timestamp, Difficulty difficulty) {
			return state::BlockStatistic(height, timestamp, difficulty, BlockFeeMultiplier());
		}

		cache::BlockStatisticRange ToRange(const StatisticSet& set) {
			return cache::BlockStatisticRange(set.cbegin(), set.cend());
		}

		model::BlockChainConfiguration CreateConfiguration() {
			auto config = model::BlockChainConfiguration::Uninitialized();
			config.BlockGenerationTargetTime = utils::TimeSpan::FromSeconds(60);
			config.MaxDifficultyBlocks = 60;
			return config;
		}
	}

	namespace {
		void AssertCalculatedDifficultyIsBaseDifficulty(const StatisticSet& set) {
			// Act:
			auto difficulty = CalculateDifficulty(ToRange(set), CreateConfiguration());

			// Assert:
			EXPECT_EQ(Base_Difficulty, difficulty);
		}
	}

	TEST(TEST_CLASS, CalculatingDifficultyOnEmptyRangeYieldsBaseDifficulty) {
		// Arrange:
		StatisticSet set;

		// Assert:
		AssertCalculatedDifficultyIsBaseDifficulty(set);
	}

	TEST(TEST_CLASS, CalculatingDifficultyOnSingleSampleYieldsBaseDifficulty) {
		// Arrange:
		StatisticSet set;
		set.insert(CreateStatistic(Height(100), Timestamp(10), Difficulty(75'000'000'000'000)));

		// Assert:
		AssertCalculatedDifficultyIsBaseDifficulty(set);
	}

	namespace {
		Difficulty GetBlockDifficultyWithConstantTimeSpacing(uint32_t targetSpacing, uint32_t actualSpacing) {
			// Arrange:
			StatisticSet set;
			for (auto i = 0u; i < 10; ++i)
				set.insert(CreateStatistic(Height(100 + i), Timestamp(12345 + i * actualSpacing), Base_Difficulty));

			auto config = CreateConfiguration();
			config.BlockGenerationTargetTime = utils::TimeSpan::FromMilliseconds(targetSpacing);

			// Act:
			return CalculateDifficulty(ToRange(set), config);
		}
	}

	TEST(TEST_CLASS, BaseDifficultyIsNotChangedWhenBlocksHaveDesiredTargetTime) {
		// Arrange:
		auto difficulty = GetBlockDifficultyWithConstantTimeSpacing(75'000, 75'000);

		// Assert:
		EXPECT_EQ(Base_Difficulty, difficulty);
	}

	TEST(TEST_CLASS, BaseDifficultyIsIncreasedWhenBlocksHaveTimeLessThanTarget) {
		// Arrange:
		auto difficulty = GetBlockDifficultyWithConstantTimeSpacing(75'000, 74'000);

		// Assert:
		EXPECT_LT(Base_Difficulty, difficulty);
	}

	TEST(TEST_CLASS, BaseDifficultyIsDecreasedWhenBlocksHaveTimeHigherThanTarget) {
		// Arrange:
		auto difficulty = GetBlockDifficultyWithConstantTimeSpacing(75'000, 76'000);

		// Assert:
		EXPECT_GT(Base_Difficulty, difficulty);
	}

	namespace {
		void AssertDifficultyChangesOverTime(uint32_t targetSpacing, uint32_t generationTime, int8_t compareResult) {
			// Arrange:
			// - initial block difficulties: BASE_DIFF, BASE_DIFF
			// - initial timestamps: t, t + TIME_DIFF
			StatisticSet set;
			set.insert(CreateStatistic(Height(100), Timestamp(100), Base_Difficulty));
			set.insert(CreateStatistic(Height(101), Timestamp(100 + generationTime), Base_Difficulty));

			auto config = CreateConfiguration();
			config.BlockGenerationTargetTime = utils::TimeSpan::FromMilliseconds(targetSpacing);

			auto previousDifficulty = Base_Difficulty;
			for (auto i = 2u; i < 62; ++i) {
				// Act: calculate the difficulty using current information
				auto difficulty = CalculateDifficulty(ToRange(set), config);

				// Assert: the difficulty changed in the expected direction
				EXPECT_COMPARE(compareResult, previousDifficulty, difficulty);

				// Arrange: add new entry to difficulty set and update previous
				set.insert(CreateStatistic(Height(100 + i), Timestamp(100 + generationTime * i), difficulty));
				previousDifficulty = difficulty;
			}
		}
	}

	TEST(TEST_CLASS, DifficultyIsDynamicallyIncreasedWhenTimeIsBelowTarget) {
		AssertDifficultyChangesOverTime(95'000, 94'000, 1);
	}

	TEST(TEST_CLASS, DifficultyIsDynamicallyDecreasedWhenTimeIsAboveTarget) {
		AssertDifficultyChangesOverTime(95'000, 96'000, -1);
	}

	namespace {
		bool IsClamped(Difficulty difficulty) {
			return Difficulty::Min() == difficulty || Difficulty::Max() == difficulty;
		}

		void AssertPercentageChange(uint32_t targetSpacing, uint32_t generationTime, int32_t expectedChange) {
			// Arrange:
			// - initial block difficulties: BASE_DIFF, BASE_DIFF
			// - initial timestamps: t, t + TIME_DIFF
			StatisticSet set;
			set.insert(CreateStatistic(Height(100), Timestamp(100), Base_Difficulty));
			set.insert(CreateStatistic(Height(101), Timestamp(100 + generationTime), Base_Difficulty));

			auto config = CreateConfiguration();
			config.BlockGenerationTargetTime = utils::TimeSpan::FromMilliseconds(targetSpacing);

			// Act + Assert
			auto previousDifficulty = Base_Difficulty;
			for (auto i = 2u; i < 102; ++i) {
				// Act: calculate the difficulty using current information
				auto difficulty = CalculateDifficulty(ToRange(set), config);
				auto difficultyDiff = static_cast<double>(static_cast<int64_t>((difficulty - previousDifficulty).unwrap()));
				auto percentageChange = difficultyDiff * 100.0 / static_cast<double>(previousDifficulty.unwrap());
				auto percentageChangeRounded = static_cast<int32_t>(std::round(percentageChange));

				if (IsClamped(difficulty)) {
					CATAPULT_LOG(debug) << "difficulty is clamped after " << i << " samples";
					EXPECT_LE(40u, i) << "breaking after " << i << " samples";
					return;
				}

				// Assert: the percentage change matches the expected change
				CATAPULT_LOG(debug) << "sample = " << i << ", % change = " << percentageChangeRounded << ", difficulty = " << difficulty;
				EXPECT_EQ(expectedChange, percentageChangeRounded);

				// Arrange: add new entry to difficulty set and update previous
				set.insert(CreateStatistic(Height(100 + i), Timestamp(100 + generationTime * i), difficulty));
				previousDifficulty = difficulty;
			}
		}
	}

	TEST(TEST_CLASS, DifficultyIncreasesAtMostFivePercentPerBlockWhenTimeIsFarBelowTarget) {
		AssertPercentageChange(60'000, 2'000, 5);
	}

	TEST(TEST_CLASS, DifficultyDecreasesAtMostFivePercentPerBlockWhenTimeIsFarAboveTarget) {
		// Assert: it is ok that a larger difference (248 - 60) is required for a 5% decrease than a 5% increase (60 - 2)
		AssertPercentageChange(60'000, 248'000, -5);
	}

	TEST(TEST_CLASS, DifficultyDoesNotChangeWhenItReachesMaximumAndTimeIsBelowTarget) {
		// Arrange:
		// - initial block difficulties: MAX_DIFF, MAX_DIFF
		// - initial timestamps: t, t + 2s
		StatisticSet set;
		set.insert(CreateStatistic(Height(100), Timestamp(100), Difficulty::Max()));
		set.insert(CreateStatistic(Height(101), Timestamp(2'100), Difficulty::Max()));

		auto config = CreateConfiguration();
		config.BlockGenerationTargetTime = utils::TimeSpan::FromSeconds(60);

		for (auto i = 2u; i < 102; ++i) {
			// Act: calculate the difficulty using current information
			auto difficulty = CalculateDifficulty(ToRange(set), config);

			// Assert: the difficulty does not change (it is clamped at max difficulty)
			EXPECT_EQ(Difficulty::Max(), difficulty);

			// Arrange: add new entry to difficulty set
			set.insert(CreateStatistic(Height(100 + i), Timestamp(100 + 2'000 * i), difficulty));
		}
	}

	TEST(TEST_CLASS, DifficultyDoesNotChangeWhenItReachesMinimumAndTimeIsAboveTarget) {
		// Arrange:
		// - initial block difficulties: MIN_DIFF, MIN_DIFF
		// - initial timestamps: t, t + 120s
		StatisticSet set;
		set.insert(CreateStatistic(Height(100), Timestamp(100), Difficulty::Min()));
		set.insert(CreateStatistic(Height(101), Timestamp(120'100), Difficulty::Min()));

		auto config = CreateConfiguration();
		config.BlockGenerationTargetTime = utils::TimeSpan::FromSeconds(60);

		for (auto i = 2u; i < 102; ++i) {
			// Act: calculate the difficulty using current information
			auto difficulty = CalculateDifficulty(ToRange(set), config);

			// Assert: the difficulty does not change (it is clamped at min difficulty)
			EXPECT_EQ(Difficulty::Min(), difficulty);

			// Arrange: add new entry to difficulty set
			set.insert(CreateStatistic(Height(100 + i), Timestamp(100 + 120'000 * i), difficulty));
		}
	}

	namespace {
		void PrepareCache(cache::BlockStatisticCache& cache, size_t numInfos) {
			auto minDifficulty = Difficulty::Min().unwrap();
			auto delta = cache.createDelta();
			for (auto i = 0u; i < numInfos; ++i)
				delta->insert(CreateStatistic(Height(i + 1), Timestamp(60'000 * i), Difficulty(minDifficulty + 1000 * i)));

			cache.commit();
		}

		struct CacheTraits {
			static Difficulty CalculateDifficulty(
					const cache::BlockStatisticCache& cache,
					Height height,
					const model::BlockChainConfiguration& config) {
				return chain::CalculateDifficulty(cache, height, config);
			}

			static void AssertDifficultyCalculationFailure(
					const cache::BlockStatisticCache& cache,
					Height height,
					const model::BlockChainConfiguration& config) {
				// Act + Assert:
				EXPECT_THROW(chain::CalculateDifficulty(cache, height, config), catapult_invalid_argument);
			}
		};

		struct TryCacheTraits {
			static Difficulty CalculateDifficulty(
					const cache::BlockStatisticCache& cache,
					Height height,
					const model::BlockChainConfiguration& config) {
				Difficulty difficulty;
				EXPECT_TRUE(TryCalculateDifficulty(cache, height, config, difficulty));
				return difficulty;
			}

			static void AssertDifficultyCalculationFailure(
					const cache::BlockStatisticCache& cache,
					Height height,
					const model::BlockChainConfiguration& config) {
				Difficulty difficulty;
				EXPECT_FALSE(TryCalculateDifficulty(cache, height, config, difficulty));
			}
		};
	}

#define CACHE_OVERLOAD_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Cache) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<CacheTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_TryCache) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TryCacheTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	CACHE_OVERLOAD_TRAITS_BASED_TEST(DifferentOverloadsYieldSameResult) {
		// Arrange:
		auto count = 10u;
		cache::BlockStatisticCache cache(count);
		PrepareCache(cache, count);
		auto config = CreateConfiguration();

		// Act:
		auto difficulty1 = TTraits::CalculateDifficulty(cache, Height(count), config);

		auto view = cache.createView();
		auto difficulty2 = CalculateDifficulty(view->statistics(Height(count), count), config);

		// Assert:
		EXPECT_EQ(difficulty1, difficulty2);
	}

	CACHE_OVERLOAD_TRAITS_BASED_TEST(MaxDifficultyBlocksInConfigIsRespected) {
		// Arrange:
		auto count = 10u;
		cache::BlockStatisticCache cache(count);
		PrepareCache(cache, count);
		auto config = CreateConfiguration();
		config.MaxDifficultyBlocks = 5;

		// Act: target time of blocks in history cache is met, so calculated difficulty should be
		//      the average historical difficulty of the last 5 blocks which is minDifficulty + 7000
		auto difficulty = TTraits::CalculateDifficulty(cache, Height(count), config);

		// Assert:
		EXPECT_EQ(Difficulty::Min() + Difficulty::Unclamped(7000), difficulty);
	}

	CACHE_OVERLOAD_TRAITS_BASED_TEST(CannotCalculateDifficultyWhenStartingHeightIsNotInCache) {
		// Arrange:
		auto count = 10u;
		cache::BlockStatisticCache cache(count);
		PrepareCache(cache, count);
		auto config = CreateConfiguration();

		// Act + Assert: try to calculate the difficulty for a height two past the last info
		TTraits::AssertDifficultyCalculationFailure(cache, Height(count + 1), config);
	}
}}
