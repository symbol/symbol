#include "catapult/chain/BlockScorer.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/model/Block.h"
#include "catapult/utils/Logging.h"
#include "tests/TestHarness.h"
#include <boost/thread.hpp>

namespace catapult { namespace chain {

#define TEST_CLASS BlockScorerTests

	namespace {
#ifdef STRESS
		constexpr size_t Num_Iterations = 100'000;
		constexpr size_t Min_Hits_For_Percentage_Deviation = 1;
		constexpr size_t Max_Percentage_Deviation = 10;
		constexpr size_t Max_Average_Deviation = 5;
#else
		// due to the small number of samples, the number of hits can vary significantly between successive levels
		// (especially for low importances), so set a min hits threshold
		constexpr size_t Num_Iterations = 25'000;
		constexpr size_t Min_Hits_For_Percentage_Deviation = 1000;
		constexpr size_t Max_Percentage_Deviation = 20;
		constexpr size_t Max_Average_Deviation = 8;
#endif

		constexpr Timestamp Max_Time(1000 * 1000);

		model::BlockChainConfiguration CreateConfiguration() {
			auto config = model::BlockChainConfiguration::Uninitialized();
			config.BlockGenerationTargetTime = utils::TimeSpan::FromSeconds(60);
			config.BlockTimeSmoothingFactor = 0;
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
					, Signer(test::GenerateRandomData<Key_Size>())
			{}
		};

		using ImportanceGroups = std::vector<std::unique_ptr<ImportanceGroup>>;

		ImportanceGroups CreateDoublingImportances() {
			uint64_t importance = 10'000'000;
			ImportanceGroups groups;
			for (auto i = 0u; i < 7u; ++i) {
				groups.push_back(std::make_unique<ImportanceGroup>(importance));
				importance *= 2;
			}

			return groups;
		}

		// calculate the block time by updating the Timestamp in current and checking if it hits using the predicate
		Timestamp GetBlockTime(
				const model::Block& parent,
				model::Block& current,
				const Hash256& generationHash,
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
		Hash256 RunHitCountIteration(
				const BlockHitPredicate& predicate,
				ImportanceGroups& importances,
				const model::Block& parent,
				const Hash256& parentGenerationHash,
				model::Block& current) {
			Timestamp bestTime = Max_Time;
			Hash256 bestGenerationHash{};
			ImportanceGroup* pBestGroup = nullptr;
			for (const auto& pGroup : importances) {
				// - set the signer and generation hash
				current.Signer = pGroup->Signer;
				Hash256 nextGenerationHash;

				crypto::Sha3_256_Builder sha3;
				sha3.update(parentGenerationHash);
				sha3.update(current.Signer);
				sha3.final(nextGenerationHash);

				auto time = GetBlockTime(parent, current, nextGenerationHash, predicate);
				if (time >= bestTime)
					continue;

				bestTime = time;
				bestGenerationHash = nextGenerationHash;
				pBestGroup = pGroup.get();
			}

			// - if no blocks hit, use a random generation hash for the next iteration
			//   (in a real scenario, this would result in a lowered difficulty)
			if (nullptr == pBestGroup)
				return test::GenerateRandomData<Hash256_Size>();

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
					return Max_Average_Deviation + 1;
				}

				// - expected hit count for current is 2x previous because importance is 2x
				auto expectedHitCount = 2 * previousHitCount;
				auto deviation = static_cast<int64_t>(expectedHitCount - currentHitCount);
				auto absoluteDeviation = static_cast<uint64_t>(std::abs(deviation));
				auto percentageDeviation = absoluteDeviation * 100u / expectedHitCount;

				// - allow a max percentage deviation between consecutive groups
				auto hasSufficientHits = currentHitCount >= Min_Hits_For_Percentage_Deviation;
				CATAPULT_LOG(debug)
						<< previousGroup.Importance << " -> " << currentGroup.Importance
						<< " deviation: " << percentageDeviation << "%"
						<< (hasSufficientHits ? "" : " (insufficient hits)");

				if (!hasSufficientHits)
					continue;

				// - if the deviation is too large, fail the iteration
				if (Max_Percentage_Deviation <= percentageDeviation) {
					CATAPULT_LOG(debug) << "Max_Percentage_Deviation <= percentageDeviation (" << percentageDeviation << ")";
					return Max_Average_Deviation + 1;
				}

				EXPECT_GT(Max_Percentage_Deviation, percentageDeviation);
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
			const auto numIterationsPerThread = Num_Iterations / test::GetNumDefaultPoolThreads();
			boost::thread_group threads;
			for (auto i = 0u; i < test::GetNumDefaultPoolThreads(); ++i) {
				threads.create_thread([&predicate, &importances, i, numIterationsPerThread] {
					// Arrange: seed srand per thread
					std::srand(static_cast<unsigned int>(std::time(nullptr)) + (2u << i));

					// - set up blocks
					model::Block parent;
					parent.Timestamp = Timestamp((900 + i) * 1000);
					auto parentGenerationHash = test::GenerateRandomData<Hash256_Size>();

					model::Block current;
					current.Difficulty = Difficulty((50 + i) * 1'000'000'000'000);

					CATAPULT_LOG(debug) << "generation hash " << i << ": " << utils::HexFormat(parentGenerationHash);

					// Act: calculate hit counts for lots of blocks
					for (auto j = 0u; j < numIterationsPerThread; ++j)
						parentGenerationHash = RunHitCountIteration(predicate, importances, parent, parentGenerationHash, current);
				});
			}

			// - wait for all threads
			threads.join_all();

			// Assert: the distribution is linearly correlated with importance
			return CalculateLinearlyCorrelatedHitCountAndImportanceAverageDeviation(importances);
		}

		void AssertHitProbabilityIsLinearlyCorrelatedWithImportance(const model::BlockChainConfiguration& config) {
			// Arrange: non-deterministic because operation is probabilistic
			test::RunNonDeterministicTest("hit probability and importance correlation", [&config]() {
				// Assert:
				auto averageDeviation = CalculateLinearlyCorrelatedHitCountAndImportanceAverageDeviation(config);
				return Max_Average_Deviation > averageDeviation;
			});
		}
	}

	NO_STRESS_TEST(TEST_CLASS, HitProbabilityIsLinearlyCorrelatedWithImportance) {
		// Assert:
		auto config = CreateConfiguration();
		AssertHitProbabilityIsLinearlyCorrelatedWithImportance(config);
	}

	NO_STRESS_TEST(TEST_CLASS, HitProbabilityIsLinearlyCorrelatedWithImportanceWhenSmoothingIsEnabled) {
		// Assert:
		auto config = CreateConfiguration();
		config.BlockTimeSmoothingFactor = 10000;
		AssertHitProbabilityIsLinearlyCorrelatedWithImportance(config);
	}
}}
