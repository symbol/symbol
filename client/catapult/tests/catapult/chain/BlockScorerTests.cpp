#include "catapult/chain/BlockScorer.h"
#include "catapult/model/Block.h"
#include "catapult/utils/Logging.h"
#include "tests/TestHarness.h"

namespace catapult { namespace chain {

	namespace {
		model::BlockChainConfiguration CreateConfiguration() {
			auto config = model::BlockChainConfiguration::Uninitialized();
			config.BlockGenerationTargetTime = utils::TimeSpan::FromSeconds(60);
			config.BlockTimeSmoothingFactor = 0;
			return config;
		}

		void SetTimestampSeconds(model::Block& block, uint64_t time) {
			block.Timestamp = Timestamp(time * 1'000);
		}

		void SetDifficultyBillions(model::Block& block, uint64_t difficulty) {
			block.Difficulty = Difficulty(difficulty * 1'000'000'000);
		}

		void SetDifficultyTrillions(model::Block& block, uint64_t difficulty) {
			SetDifficultyBillions(block, difficulty * 1'000);
		}
	}

	// region CalculateHit

	TEST(BlockScorerTests, CanCalculateHit) {
		// Arrange:
		Hash256 generationHash{ {
			0xF7, 0xF6, 0xF5, 0xF4, 0xF3, 0xF2, 0xF1, 0xF0,
			0xE7, 0xE6, 0xE5, 0xE4, 0xE3, 0xE2, 0xE1, 0xE0,
			0xD7, 0xD6, 0xD5, 0xD4, 0xD3, 0xD2, 0xD1, 0xD0,
			0xC7, 0xC6, 0xC5, 0xC4, 0xC3, 0xC2, 0xC1, 0xC0
		} };

		// Act:
		auto hit = CalculateHit(generationHash);

		// Assert:
		EXPECT_EQ(0x20A80E8435E74u, hit);
	}

	TEST(BlockScorerTests, CanCalculateHitWhenGenerationHashIsZero) {
		// Arrange:
		Hash256 generationHash{ {} };

		// Act:
		auto hit = CalculateHit(generationHash);

		// Assert:
		EXPECT_EQ(std::numeric_limits<uint64_t>::max(), hit);
	}

	// endregion

	// region CalculateScore

	TEST(BlockScorerTests, CanCalculateScore) {
		// Arrange:
		model::Block parent;
		SetTimestampSeconds(parent, 5567320ull);

		model::Block current;
		SetTimestampSeconds(current, 5568532ull);
		SetDifficultyBillions(current, 44'888);

		// Act:
		auto score = CalculateScore(parent, current);

		// Assert:
		EXPECT_EQ(44'888'000'000'000ull - (5568532 - 5567320), score);
	}

	namespace {
		uint64_t CalculateBlockScore(uint64_t parentTime, uint64_t currentTime) {
			// Arrange:
			model::Block parent;
			SetTimestampSeconds(parent, parentTime);

			model::Block current;
			SetTimestampSeconds(current, currentTime);
			SetDifficultyBillions(current, 44'888);

			// Act:
			return CalculateScore(parent, current);
		}
	}

	TEST(BlockScorerTests, BlockScoreIsZeroWhenElapsedTimeIsZero) {
		// Act:
		auto score = CalculateBlockScore(1000, 1000);

		// Assert:
		EXPECT_EQ(0u, score);
	}

	TEST(BlockScorerTests, BlockScoreIsZeroWhenElapsedTimeIsNegative) {
		// Act:
		auto score = CalculateBlockScore(1000, 900);

		// Assert:
		EXPECT_EQ(0u, score);
	}

	// endregion

	// region CalculateTarget

	namespace {
		BlockTarget CalculateTargetFromRawValues(
				uint64_t parentTime,
				uint64_t currentTime,
				uint64_t importance,
				uint64_t difficulty = 60,
				const model::BlockChainConfiguration& config = CreateConfiguration()) {
			// Arrange:
			auto timeDiff = utils::TimeSpan::FromSeconds(currentTime - parentTime);
			auto realDifficulty = Difficulty(difficulty * 1'000'000'000'000);

			// Act:
			return CalculateTarget(timeDiff, realDifficulty, Importance(importance), config);
		}

		BlockTarget CalculateBlockTarget(
				uint64_t parentTime,
				uint64_t currentTime,
				uint64_t importance,
				uint64_t difficulty = 60,
				const model::BlockChainConfiguration& config = CreateConfiguration()) {
			// Arrange:
			model::Block parent;
			SetTimestampSeconds(parent, parentTime);

			model::Block current;
			SetTimestampSeconds(current, currentTime);
			SetDifficultyTrillions(current, difficulty);

			// Act:
			return CalculateTarget(parent, current, Importance(importance), config);
		}
	}

	TEST(BlockScorerTests, BlockTargetIsZeroWhenEffectiveImportanceIsZero) {
		// Act:
		auto target = CalculateBlockTarget(1000, 1100, 0);

		// Assert:
		EXPECT_EQ(BlockTarget(0), target);
	}

	TEST(BlockScorerTests, BlockTargetIsZeroWhenElapsedTimeIsZero) {
		// Act:
		auto target = CalculateBlockTarget(1000, 1000, 100);

		// Assert:
		EXPECT_EQ(BlockTarget(0), target);
	}

	TEST(BlockScorerTests, BlockTargetIsZeroWhenElapsedTimeIsNegative) {
		// Act:
		auto target = CalculateBlockTarget(1000, 900, 100);

		// Assert:
		EXPECT_EQ(BlockTarget(0), target);
	}

	TEST(BlockScorerTests, BlockTargetIncreasesAsTimeElapses) {
		// Act:
		auto target1 = CalculateBlockTarget(900, 1000, 72000);
		auto target2 = CalculateBlockTarget(900, 1100, 72000);

		// Assert:
		EXPECT_GT(target2, target1);
	}

	TEST(BlockScorerTests, BlockTargetIncreasesAsImportanceIncreases) {
		// Act:
		auto target1 = CalculateBlockTarget(900, 1000, 72000);
		auto target2 = CalculateBlockTarget(900, 1000, 74000);

		// Assert:
		EXPECT_GT(target2, target1);
	}

	TEST(BlockScorerTests, BlockTargetIncreasesAsDifficultyDecreases) {
		// Act:
		auto target1 = CalculateBlockTarget(900, 1000, 72000, 50);
		auto target2 = CalculateBlockTarget(900, 1000, 72000, 40);

		// Assert:
		EXPECT_GT(target2, target1);
	}

	namespace {
		const BlockTarget Two_To_64 = []() {
			BlockTarget result = 1;
			result <<= 64;
			return result;
		}();
	}

	TEST(BlockScorerTests, BlockTargetIsCorrectlyCalculated) {
		// Act:
		auto target = CalculateBlockTarget(900, 1000, 72000, 50);

		// Assert:
		BlockTarget expectedTarget = 100; // time difference (in seconds)
		expectedTarget *= 72000; // importance
		expectedTarget *= Two_To_64; // magic number
		expectedTarget /= 50'000'000'000'000; // difficulty

		// Assert:
		EXPECT_EQ(expectedTarget, target);
	}

	TEST(BlockScorerTests, TargetIsCorrectlyCalculatedFromRawValues) {
		// Act:
		auto target = CalculateTargetFromRawValues(900, 1000, 72000, 50);

		// Assert:
		BlockTarget expectedTarget = 100; // time difference (in seconds)
		expectedTarget *= 72000; // importance
		expectedTarget *= Two_To_64; // magic number
		expectedTarget /= 50'000'000'000'000; // difficulty

		// Assert:
		EXPECT_EQ(expectedTarget, target);
	}

	TEST(BlockScorerTests, BlockTargetIsConsistentWithLegacyBlockTarget) {
		// Act:
		auto target = CalculateBlockTarget(1, 101, 72000 * 8'000'000'000L);

		// Assert:
		// - expected target is 17708874310761169551360, but it is too large to fit in any built-in integral type
		// - so split it up as 1770887431076116955 * 10000 + 1360
		BlockTarget expectedTarget = 1770887431076116955ull;
		expectedTarget *= 10000;
		expectedTarget += 1360;
		EXPECT_EQ(expectedTarget, target);
	}

	TEST(BlockScorerTests, GenerationTimeHasNoImpactOnTargetWhenSmoothingIsDisabled) {
		// Act:
		auto config = CreateConfiguration();
		config.BlockTimeSmoothingFactor = 0;
		config.BlockGenerationTargetTime = utils::TimeSpan::FromSeconds(15);
		auto target1 = CalculateBlockTarget(900, 916, 72000, 50, config);

		config.BlockGenerationTargetTime = utils::TimeSpan::FromSeconds(10);
		auto target2 = CalculateBlockTarget(900, 916, 72000, 50, config);

		// Assert:
		EXPECT_EQ(target2, target1);
	}

	TEST(BlockScorerTests, BlockTargetIncreasesAsGenerationTimeDecreasesWhenSmoothingIsEnabled) {
		// Act:
		auto config = CreateConfiguration();
		config.BlockTimeSmoothingFactor = 6000;
		config.BlockGenerationTargetTime = utils::TimeSpan::FromSeconds(15);
		auto target1 = CalculateBlockTarget(900, 916, 72000, 50, config);

		config.BlockGenerationTargetTime = utils::TimeSpan::FromSeconds(10);
		auto target2 = CalculateBlockTarget(900, 916, 72000, 50, config);

		// Assert:
		EXPECT_GT(target2, target1);
	}

	namespace {
		void AssertLargerSmoothingFactorBiasesTowardsLargerTargetWhenLastBlockTimeIsLarger(uint32_t factor1, uint32_t factor2) {
			// Sanity:
			EXPECT_GT(factor2, factor1);

			// Act:
			auto config = CreateConfiguration();
			config.BlockTimeSmoothingFactor = factor1;
			config.BlockGenerationTargetTime = utils::TimeSpan::FromSeconds(15);
			auto target1 = CalculateBlockTarget(900, 916, 72000, 50, config);

			config.BlockTimeSmoothingFactor = factor2;
			auto target2 = CalculateBlockTarget(900, 916, 72000, 50, config);

			// Assert:
			EXPECT_GT(target2, target1);
		}

		void AssertLargerSmoothingFactorBiasesTowardsSmallerTargetWhenLastBlockTimeIsSmaller(uint32_t factor1, uint32_t factor2) {
			// Sanity:
			EXPECT_GT(factor2, factor1);

			// Act:
			auto config = CreateConfiguration();
			config.BlockTimeSmoothingFactor = factor1;
			config.BlockGenerationTargetTime = utils::TimeSpan::FromSeconds(15);
			auto target1 = CalculateBlockTarget(900, 914, 72000, 50, config);

			config.BlockTimeSmoothingFactor = factor2;
			auto target2 = CalculateBlockTarget(900, 914, 72000, 50, config);

			// Assert:
			EXPECT_LT(target2, target1);
		}
	}

	TEST(BlockScorerTests, BlockTargetWithSmoothingIsGreaterThanTargetWithoutSmoothingWhenLastBlockTimeIsLarger) {
		// Assert:
		AssertLargerSmoothingFactorBiasesTowardsLargerTargetWhenLastBlockTimeIsLarger(0, 6000);
	}

	TEST(BlockScorerTests, BlockTargetWithSmoothingIsLessThanTargetWithoutSmoothingWhenLastBlockTimeIsSmaller) {
		// Assert:
		AssertLargerSmoothingFactorBiasesTowardsSmallerTargetWhenLastBlockTimeIsSmaller(0, 6000);
	}

	TEST(BlockScorerTests, LargerSmoothingFactorBiasesTowardsLargerTargetWhenLastBlockTimeIsLarger) {
		// Assert:
		AssertLargerSmoothingFactorBiasesTowardsLargerTargetWhenLastBlockTimeIsLarger(3000, 6000);
	}

	TEST(BlockScorerTests, LargerSmoothingFactorBiasesTowardsSmallerTargetWhenLastBlockTimeIsSmaller) {
		// Assert:
		AssertLargerSmoothingFactorBiasesTowardsSmallerTargetWhenLastBlockTimeIsSmaller(3000, 6000);
	}

	TEST(BlockScorerTests, BlockTargetIsCorrectlyCalculatedWhenSmoothingIsEnabled) {
		// Act:
		auto config = CreateConfiguration();
		config.BlockGenerationTargetTime = utils::TimeSpan::FromSeconds(10);
		config.BlockTimeSmoothingFactor = 6000;
		auto target = CalculateBlockTarget(900, 911, 72000, 50, config);

		// Assert:
		BlockTarget multiplier = static_cast<uint64_t>((1ull << 54) * std::exp(6.0 * (11 - 10) / 10));
		multiplier <<= 10;

		BlockTarget expectedTarget = 11; // time difference (in seconds)
		expectedTarget *= 72000; // importance
		expectedTarget *= multiplier; // magic number with smoothing multiplier
		expectedTarget /= 50'000'000'000'000; // difficulty

		// Assert:
		EXPECT_EQ(expectedTarget, target);
	}

	TEST(BlockScorerTests, BlockTargetSmoothingIsCapped) {
		// Act:
		auto config = CreateConfiguration();
		config.BlockGenerationTargetTime = utils::TimeSpan::FromSeconds(10);
		config.BlockTimeSmoothingFactor = 6000;
		auto target = CalculateBlockTarget(900, 1000, 72000, 50, config);

		// Assert:
		BlockTarget expectedTarget = 100; // time difference (in seconds)
		expectedTarget *= 72000; // importance
		expectedTarget *= Two_To_64; // magic number
		expectedTarget *= 100; // max smoothing
		expectedTarget /= 50'000'000'000'000; // difficulty

		// Assert:
		EXPECT_EQ(expectedTarget, target);
	}

	// endregion

	// region BlockHitPredicate

	namespace {
		struct BlockHitPredicateContext {
			explicit BlockHitPredicateContext(Importance importance)
					: Config(CreateConfiguration())
					, Predicate(
							Config,
							[this, importance](const auto& key, auto height) {
								ImportanceLookupParams.push_back(std::make_pair(key, height));
								return importance;
							})
			{}

			model::BlockChainConfiguration Config;
			BlockHitPredicate Predicate;
			std::vector<std::pair<Key, Height>> ImportanceLookupParams;
		};

		std::unique_ptr<model::Block> CreateBlock(
				Height height,
				uint32_t timestampSeconds,
				uint32_t difficultyTrillions) {
			auto pBlock = std::make_unique<model::Block>();
			pBlock->Signer = test::GenerateRandomData<Hash256_Size>();
			pBlock->Height = height;
			SetTimestampSeconds(*pBlock, timestampSeconds);
			SetDifficultyTrillions(*pBlock, difficultyTrillions);
			return pBlock;
		}
	}

	TEST(BlockScorerTests, BlockHitPredicateReturnsTrueWhenHitIsLessThanTarget) {
		// Arrange:
		auto pParent = CreateBlock(Height(10), 900, 0);
		auto pCurrent = CreateBlock(Height(11), 1000, 50);
		const Hash256 generationHash{ { 0xF7, 0xF6, 0xF5, 0xF4 } };

		Importance signerImportance = Importance(20000000);
		BlockHitPredicateContext context(signerImportance);

		// Act:
		auto isHit = context.Predicate(*pParent, *pCurrent, generationHash);

		// Assert:
		EXPECT_LT(CalculateHit(generationHash), CalculateTarget(*pParent, *pCurrent, signerImportance, context.Config));
		EXPECT_TRUE(isHit);

		ASSERT_EQ(1u, context.ImportanceLookupParams.size());
		EXPECT_EQ(pCurrent->Signer, context.ImportanceLookupParams[0].first);
		EXPECT_EQ(pCurrent->Height, context.ImportanceLookupParams[0].second);
	}

	TEST(BlockScorerTests, BlockHitPredicateReturnsTrueWhenHitIsLessThanTarget_Context) {
		// Arrange:
		BlockHitContext hitContext;
		hitContext.GenerationHash = { { 0xF7, 0xF6, 0xF5, 0xF4 } };
		hitContext.ElapsedTime = utils::TimeSpan::FromSeconds(100);
		hitContext.Signer = test::GenerateRandomData<Hash256_Size>();
		hitContext.Difficulty = Difficulty(50 * 1'000'000'000'000);
		hitContext.Height = Height(11);

		Importance signerImportance = Importance(20000000);
		BlockHitPredicateContext context(signerImportance);

		// Act:
		auto isHit = context.Predicate(hitContext);

		// Assert:
		auto hit = CalculateHit(hitContext.GenerationHash);
		auto target = CalculateTarget(hitContext.ElapsedTime, hitContext.Difficulty, signerImportance, context.Config);
		EXPECT_LT(hit, target);
		EXPECT_TRUE(isHit);

		ASSERT_EQ(1u, context.ImportanceLookupParams.size());
		EXPECT_EQ(hitContext.Signer, context.ImportanceLookupParams[0].first);
		EXPECT_EQ(hitContext.Height, context.ImportanceLookupParams[0].second);
	}

	TEST(BlockScorerTests, BlockHitPredicateReturnsFalseWhenHitIsEqualToTarget) {
		// Arrange:
		auto pParent = CreateBlock(Height(10), 900, 0);
		auto pCurrent = CreateBlock(Height(11), 900, 50);
		const Hash256 generationHash{ {
			0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
			0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
			0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
			0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
		} };

		Importance signerImportance = Importance(100000);
		BlockHitPredicateContext context(signerImportance);

		// Act:
		auto isHit = context.Predicate(*pParent, *pCurrent, generationHash);

		// Assert:
		EXPECT_EQ(CalculateHit(generationHash), CalculateTarget(*pParent, *pCurrent, signerImportance, context.Config));
		EXPECT_FALSE(isHit);

		ASSERT_EQ(1u, context.ImportanceLookupParams.size());
		EXPECT_EQ(pCurrent->Signer, context.ImportanceLookupParams[0].first);
		EXPECT_EQ(pCurrent->Height, context.ImportanceLookupParams[0].second);
	}

	TEST(BlockScorerTests, BlockHitPredicateReturnsFalseWhenHitIsEqualToTarget_Context) {
		// Arrange:
		BlockHitContext hitContext;
		hitContext.GenerationHash = { {
			0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
			0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
			0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
			0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
		} };
		hitContext.ElapsedTime = utils::TimeSpan::FromSeconds(0);
		hitContext.Signer = test::GenerateRandomData<Hash256_Size>();
		hitContext.Difficulty = Difficulty(50 * 1'000'000'000'000);
		hitContext.Height = Height(11);

		Importance signerImportance = Importance(100000);
		BlockHitPredicateContext context(signerImportance);

		// Act:
		auto isHit = context.Predicate(hitContext);

		// Assert:
		auto hit = CalculateHit(hitContext.GenerationHash);
		auto target = CalculateTarget(hitContext.ElapsedTime, hitContext.Difficulty, signerImportance, context.Config);
		EXPECT_EQ(hit, target);
		EXPECT_FALSE(isHit);

		ASSERT_EQ(1u, context.ImportanceLookupParams.size());
		EXPECT_EQ(hitContext.Signer, context.ImportanceLookupParams[0].first);
		EXPECT_EQ(hitContext.Height, context.ImportanceLookupParams[0].second);
	}

	TEST(BlockScorerTests, BlockHitPredicateReturnsFalseWhenHitIsGreaterThanTarget) {
		// Arrange:
		auto pParent = CreateBlock(Height(10), 900, 0);
		auto pCurrent = CreateBlock(Height(11), 1000, 50);
		const Hash256 generationHash{ { 0xF7, 0xF6, 0xF5, 0xF4 } };

		Importance signerImportance = Importance(1000);
		BlockHitPredicateContext context(signerImportance);

		// Act:
		auto isHit = context.Predicate(*pParent, *pCurrent, generationHash);

		// Assert:
		EXPECT_GT(CalculateHit(generationHash), CalculateTarget(*pParent, *pCurrent, signerImportance, context.Config));
		EXPECT_FALSE(isHit);

		ASSERT_EQ(1u, context.ImportanceLookupParams.size());
		EXPECT_EQ(pCurrent->Signer, context.ImportanceLookupParams[0].first);
		EXPECT_EQ(pCurrent->Height, context.ImportanceLookupParams[0].second);
	}

	TEST(BlockScorerTests, BlockHitPredicateReturnsFalseWhenHitIsGreaterThanTarget_Context) {
		// Arrange:
		BlockHitContext hitContext;
		hitContext.GenerationHash = { { 0xF7, 0xF6, 0xF5, 0xF4 } };
		hitContext.ElapsedTime = utils::TimeSpan::FromSeconds(100);
		hitContext.Signer = test::GenerateRandomData<Hash256_Size>();
		hitContext.Difficulty = Difficulty(50 * 1'000'000'000'000);
		hitContext.Height = Height(11);

		Importance signerImportance = Importance(1000);
		BlockHitPredicateContext context(signerImportance);

		// Act:
		auto isHit = context.Predicate(hitContext);

		// Assert:
		auto hit = CalculateHit(hitContext.GenerationHash);
		auto target = CalculateTarget(hitContext.ElapsedTime, hitContext.Difficulty, signerImportance, context.Config);
		EXPECT_GT(hit, target);
		EXPECT_FALSE(isHit);

		ASSERT_EQ(1u, context.ImportanceLookupParams.size());
		EXPECT_EQ(hitContext.Signer, context.ImportanceLookupParams[0].first);
		EXPECT_EQ(hitContext.Height, context.ImportanceLookupParams[0].second);
	}

	// endregion
}}
