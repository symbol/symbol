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

#include "catapult/chain/BlockScorer.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/model/Block.h"
#include "catapult/thread/ThreadGroup.h"
#include "catapult/utils/Logging.h"
#include "tests/test/nodeps/TestConstants.h"
#include "tests/TestHarness.h"

namespace catapult { namespace chain {

#define TEST_CLASS BlockScorerIntegrityTests

	namespace {
		constexpr Timestamp Max_Time(1000 * 1000);

		struct IntegrityTestParameters {
		public:
			IntegrityTestParameters() {
				if (test::GetStressIterationCount()) {
					NumIterations = 100'000;
					MinHitsForPercentageDeviation = 1;
					MaxPercentageDeviation = 10;
					MaxAverageDeviation = 5;
				} else {
					// due to the small number of samples, the number of hits can vary significantly between successive levels
					// (especially for low importances), so set a min hits threshold
					NumIterations = 25'000;
					MinHitsForPercentageDeviation = 1000;
					MaxPercentageDeviation = 20;
					MaxAverageDeviation = 8;
				}
			}

		public:
			size_t NumIterations;
			size_t MinHitsForPercentageDeviation;
			size_t MaxPercentageDeviation;
			size_t MaxAverageDeviation;
		};

		model::BlockChainConfiguration CreateConfiguration() {
			auto config = model::BlockChainConfiguration::Uninitialized();
			config.BlockGenerationTargetTime = utils::TimeSpan::FromSeconds(60);
			config.BlockTimeSmoothingFactor = 0;
			config.TotalChainImportance = test::Default_Total_Chain_Importance;
			return config;
		}

		struct ImportanceGroup {
		public:
			catapult::Importance Importance;
			std::atomic<size_t> HitCount;
			Key Signer;

		public:
			explicit ImportanceGroup(catapult::Importance::ValueType importance)
					: Importance(catapult::Importance(importance))
					, HitCount(0)
					, Signer(test::GenerateRandomByteArray<Key>())
			{}
		};

		using ImportanceGroups = std::vector<std::unique_ptr<ImportanceGroup>>;

		ImportanceGroups CreateDoublingImportances() {
			uint64_t importance = 10'000'000;
			ImportanceGroups groups;
			for (auto i = 0u; i < 7; ++i) {
				groups.push_back(std::make_unique<ImportanceGroup>(importance));
				importance *= 2;
			}

			return groups;
		}

		// calculate the block time by updating the Timestamp in current and checking if it hits using the predicate
		Timestamp GetBlockTime(
				const model::Block& parent,
				model::Block& current,
				const GenerationHash& generationHash,
				const BlockHitPredicate& predicate) {
			const auto MS_In_S = 1000;

			// - make sure that there is a hit possibility
			current.Timestamp = Max_Time;
			if (!predicate(parent, current, generationHash))
				return Max_Time;

			// - use a binary search to find the hit time
			uint64_t lowerBound = 0;
			uint64_t upperBound = Max_Time.unwrap();
			while (upperBound - lowerBound > MS_In_S) {
				auto middle = (upperBound + lowerBound) / 2;
				current.Timestamp = parent.Timestamp + Timestamp(middle);

				if (predicate(parent, current, generationHash))
					upperBound = middle;
				else
					lowerBound = middle;
			}

			return Timestamp(upperBound);
		}

		// runs an iteration and returns the next generation hash
		GenerationHash RunHitCountIteration(
				const BlockHitPredicate& predicate,
				ImportanceGroups& importances,
				const model::Block& parent,
				const GenerationHash& parentGenerationHash,
				model::Block& currentBlock) {
			Timestamp bestTime = Max_Time;
			GenerationHash bestGenerationHash{};
			ImportanceGroup* pBestGroup = nullptr;
			for (const auto& pGroup : importances) {
				// - set the signer and generation hash
				currentBlock.SignerPublicKey = pGroup->Signer;
				GenerationHash nextGenerationHash;

				crypto::GenerationHash_Builder hasher;
				hasher.update(parentGenerationHash);
				hasher.update(currentBlock.SignerPublicKey);
				hasher.final(nextGenerationHash);

				auto time = GetBlockTime(parent, currentBlock, nextGenerationHash, predicate);
				if (time >= bestTime)
					continue;

				bestTime = time;
				bestGenerationHash = nextGenerationHash;
				pBestGroup = pGroup.get();
			}

			// - if no blocks hit, use a random generation hash for the next iteration
			//   (in a real scenario, this would result in a lowered difficulty)
			if (!pBestGroup)
				return test::GenerateRandomByteArray<GenerationHash>();

			// - increment the hit count for the best group and use its generation hash for the next iteration
			++pBestGroup->HitCount;
			return bestGenerationHash;
		}

		uint64_t CalculateLinearlyCorrelatedHitCountAndImportanceAverageDeviation(const ImportanceGroups& importances) {
			// log all values
			for (const auto& pGroup : importances)
				CATAPULT_LOG(debug) << pGroup->Importance << " -> " << pGroup->HitCount;

			// Assert:
			auto numGroupsWithSufficientHits = 0u;
			uint64_t cumulativePercentageDeviation = 0;
			for (auto i = 1u; i < importances.size(); ++i) {
				const auto& previousGroup = *importances[i - 1];
				const auto& currentGroup = *importances[i];
				auto previousHitCount = previousGroup.HitCount.load();
				auto currentHitCount = currentGroup.HitCount.load();

				// - current hit count is greater than previous (and previous is nonzero to avoid divide by zero)
				EXPECT_GT(currentHitCount, previousHitCount) << "current " << i;
				if (0 == previousHitCount) {
					CATAPULT_LOG(debug) << "no hits for iteration " << (i - 1);
					return IntegrityTestParameters().MaxAverageDeviation + 1;
				}

				// - expected hit count for current is 2x previous because importance is 2x
				auto expectedHitCount = 2 * previousHitCount;
				auto deviation = static_cast<int64_t>(expectedHitCount - currentHitCount);
				auto absoluteDeviation = static_cast<uint64_t>(std::abs(deviation));
				auto percentageDeviation = absoluteDeviation * 100u / expectedHitCount;

				// - allow a max percentage deviation between consecutive groups
				auto hasSufficientHits = currentHitCount >= IntegrityTestParameters().MinHitsForPercentageDeviation;
				CATAPULT_LOG(debug)
						<< previousGroup.Importance << " -> " << currentGroup.Importance
						<< " deviation: " << percentageDeviation << "%"
						<< (hasSufficientHits ? "" : " (insufficient hits)");

				if (!hasSufficientHits)
					continue;

				// - if the deviation is too large, fail the iteration
				if (IntegrityTestParameters().MaxPercentageDeviation <= percentageDeviation) {
					CATAPULT_LOG(debug) << "MaxPercentageDeviation <= percentageDeviation (" << percentageDeviation << ")";
					return IntegrityTestParameters().MaxAverageDeviation + 1;
				}

				EXPECT_GT(IntegrityTestParameters().MaxPercentageDeviation, percentageDeviation);
				cumulativePercentageDeviation += percentageDeviation;
				++numGroupsWithSufficientHits;
			}

			// - allow max average percentage deviation among all groups with sufficient hits
			auto averageDeviation = cumulativePercentageDeviation / numGroupsWithSufficientHits;
			CATAPULT_LOG(debug) << "average deviation " << averageDeviation << " (num groups " << numGroupsWithSufficientHits << ")";
			return averageDeviation;
		}

		uint64_t CalculateLinearlyCorrelatedHitCountAndImportanceAverageDeviation(const model::BlockChainConfiguration& config) {
			// Arrange: set up test importances
			auto importances = CreateDoublingImportances();

			// - set up chain
			BlockHitPredicate predicate(config, [&importances](const auto& signerKey, auto) {
				auto iter = std::find_if(importances.cbegin(), importances.cend(), [&signerKey](const auto& pGroup) {
					return pGroup->Signer == signerKey;
				});
				return importances.cend() != iter ? (*iter)->Importance : Importance(0);
			});

			// - calculate chain scores on all threads
			const auto numIterationsPerThread = IntegrityTestParameters().NumIterations / test::GetNumDefaultPoolThreads();
			thread::ThreadGroup threads;
			for (auto i = 0u; i < test::GetNumDefaultPoolThreads(); ++i) {
				threads.spawn([&predicate, &importances, i, numIterationsPerThread] {
					// Arrange: seed srand per thread
					std::srand(static_cast<unsigned int>(std::time(nullptr)) + (2u << i));

					// - set up blocks
					model::Block parent;
					parent.Timestamp = Timestamp((900 + i) * 1000);
					auto parentGenerationHash = test::GenerateRandomByteArray<GenerationHash>();

					model::Block current;
					current.Difficulty = Difficulty((50 + i) * 1'000'000'000'000);

					CATAPULT_LOG(debug) << "generation hash " << i << ": " << parentGenerationHash;

					// Act: calculate hit counts for lots of blocks
					for (auto j = 0u; j < numIterationsPerThread; ++j)
						parentGenerationHash = RunHitCountIteration(predicate, importances, parent, parentGenerationHash, current);
				});
			}

			// - wait for all threads
			threads.join();

			// Assert: the distribution is linearly correlated with importance
			return CalculateLinearlyCorrelatedHitCountAndImportanceAverageDeviation(importances);
		}

		void AssertHitProbabilityIsLinearlyCorrelatedWithImportance(const model::BlockChainConfiguration& config) {
			// Arrange: non-deterministic because operation is probabilistic
			test::RunNonDeterministicTest("hit probability and importance correlation", [&config]() {
				// Assert:
				auto averageDeviation = CalculateLinearlyCorrelatedHitCountAndImportanceAverageDeviation(config);
				return IntegrityTestParameters().MaxAverageDeviation > averageDeviation;
			});
		}
	}

	NO_STRESS_TEST(TEST_CLASS, HitProbabilityIsLinearlyCorrelatedWithImportance) {
		// Arrange:
		auto config = CreateConfiguration();

		// Act + Assert:
		AssertHitProbabilityIsLinearlyCorrelatedWithImportance(config);
	}

	NO_STRESS_TEST(TEST_CLASS, HitProbabilityIsLinearlyCorrelatedWithImportanceWhenSmoothingIsEnabled) {
		// Arrange:
		auto config = CreateConfiguration();
		config.BlockTimeSmoothingFactor = 10000;

		// Act + Assert:
		AssertHitProbabilityIsLinearlyCorrelatedWithImportance(config);
	}
}}
