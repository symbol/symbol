#include "catapult/chain/ChainUtils.h"
#include "catapult/cache_core/BlockDifficultyCache.h"
#include "catapult/chain/BlockDifficultyScorer.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/model/EntityHasher.h"
#include "catapult/utils/TimeSpan.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace chain {

#define TEST_CLASS ChainUtilsTests

	// region IsChainLink

	namespace {
		std::unique_ptr<model::Block> GenerateBlockAtHeight(Height height, Timestamp timestamp) {
			auto pBlock = test::GenerateBlockWithTransactionsAtHeight(0, height);
			pBlock->Timestamp = timestamp;
			return pBlock;
		}

		void LinkHashes(model::Block& parentBlock, model::Block& childBlock) {
			childBlock.PreviousBlockHash = model::CalculateHash(parentBlock);
		}

		void AssertNotLinkedForHeights(Height parentHeight, Height childHeight) {
			// Arrange:
			auto pParent = GenerateBlockAtHeight(parentHeight, Timestamp(100));
			auto pChild = GenerateBlockAtHeight(childHeight, Timestamp(101));
			LinkHashes(*pParent, *pChild);

			// Act:
			bool isLink = IsChainLink(*pParent, pChild->PreviousBlockHash, *pChild);

			// Assert:
			EXPECT_FALSE(isLink) << "parent " << parentHeight << ", child " << childHeight;
		}
	}

	TEST(TEST_CLASS, IsChainLinkReturnsFalseIfHeightIsMismatched) {
		// Assert:
		AssertNotLinkedForHeights(Height(70), Height(60));
		AssertNotLinkedForHeights(Height(70), Height(69));
		AssertNotLinkedForHeights(Height(70), Height(70));
		AssertNotLinkedForHeights(Height(70), Height(72));
		AssertNotLinkedForHeights(Height(70), Height(80));
	}

	TEST(TEST_CLASS, IsChainLinkReturnsFalseIfPreviousBlockHashIsIncorrect) {
		// Arrange:
		auto pParent = GenerateBlockAtHeight(Height(70), Timestamp(100));
		auto pChild = GenerateBlockAtHeight(Height(71), Timestamp(101));
		LinkHashes(*pParent, *pChild);

		// Act:
		bool isLink = IsChainLink(*pParent, test::GenerateRandomData<Hash256_Size>(), *pChild);

		// Assert:
		EXPECT_FALSE(isLink);
	}

	namespace {
		void AssertNotLinkedForTimestamps(Timestamp parentTimestamp, Timestamp childTimestamp) {
			// Arrange:
			auto pParent = GenerateBlockAtHeight(Height(90), parentTimestamp);
			auto pChild = GenerateBlockAtHeight(Height(91), childTimestamp);
			LinkHashes(*pParent, *pChild);

			// Act:
			bool isLink = IsChainLink(*pParent, pChild->PreviousBlockHash, *pChild);

			// Assert:
			EXPECT_FALSE(isLink) << "parent " << parentTimestamp << ", child " << childTimestamp;
		}
	}

	TEST(TEST_CLASS, IsChainLinkReturnsFalseIfTimestampsAreNotIncreasing) {
		// Assert:
		AssertNotLinkedForTimestamps(Timestamp(70), Timestamp(60));
		AssertNotLinkedForTimestamps(Timestamp(70), Timestamp(69));
		AssertNotLinkedForTimestamps(Timestamp(70), Timestamp(70));
	}

	namespace {
		void AssertLinkedForTimestamps(Timestamp parentTimestamp, Timestamp childTimestamp) {
			// Arrange:
			auto pParent = GenerateBlockAtHeight(Height(90), parentTimestamp);
			auto pChild = GenerateBlockAtHeight(Height(91), childTimestamp);
			LinkHashes(*pParent, *pChild);

			// Act:
			bool isLink = IsChainLink(*pParent, pChild->PreviousBlockHash, *pChild);

			// Assert:
			EXPECT_TRUE(isLink) << "parent " << parentTimestamp << ", child " << childTimestamp;
		}
	}

	TEST(TEST_CLASS, IsChainLinkReturnsTrueIfBothHeightAndHashesAreCorrectAndTimestampsAreIncreasing) {
		// Assert:
		AssertLinkedForTimestamps(Timestamp(70), Timestamp(71));
		AssertLinkedForTimestamps(Timestamp(70), Timestamp(700));
		AssertLinkedForTimestamps(Timestamp(70), Timestamp(12345));
	}

	// endregion

	// region CheckDifficulties

	namespace {
		constexpr auto Base_Difficulty = Difficulty().unwrap();

		Timestamp CalculateTimestamp(Height height, const utils::TimeSpan& timeBetweenBlocks) {
			return Timestamp((height - Height(1)).unwrap() * timeBetweenBlocks.millis());
		}

		std::unique_ptr<cache::BlockDifficultyCache> SeedBlockDifficultyCache(Height maxHeight, const utils::TimeSpan& timeBetweenBlocks) {
			auto pCache = std::make_unique<cache::BlockDifficultyCache>(0);
			auto delta = pCache->createDelta();

			for (auto height = Height(1); height <= maxHeight; height = height + Height(1)) {
				state::BlockDifficultyInfo info(height, CalculateTimestamp(height, timeBetweenBlocks), Difficulty());
				delta->insert(info);
			}

			pCache->commit();
			return pCache;
		}

		std::vector<std::unique_ptr<model::Block>> GenerateBlocks(
				Height startHeight,
				const utils::TimeSpan& timeBetweenBlocks,
				uint32_t numBlocks) {
			std::vector<std::unique_ptr<model::Block>> blocks;
			for (auto i = 0u; i < numBlocks; ++i) {
				auto pBlock = test::GenerateEmptyRandomBlock();
				pBlock->Height = startHeight + Height(i);
				pBlock->Timestamp = CalculateTimestamp(pBlock->Height, timeBetweenBlocks);
				pBlock->Difficulty = Difficulty();
				blocks.push_back(std::move(pBlock));
			}

			return blocks;
		}

		std::vector<const model::Block*> Unwrap(const std::vector<std::unique_ptr<model::Block>>& blocks) {
			std::vector<const model::Block*> blockPointers;
			for (const auto& pBlock : blocks)
				blockPointers.push_back(pBlock.get());

			return blockPointers;
		}

		model::BlockChainConfiguration CreateConfiguration() {
			auto config = model::BlockChainConfiguration::Uninitialized();
			config.BlockGenerationTargetTime = utils::TimeSpan::FromSeconds(60);
			config.MaxDifficultyBlocks = 60;
			return config;
		}
	}

	TEST(TEST_CLASS, DifficultiesAreValidIfPeerChainIsEmpty) {
		// Arrange: set up the config
		auto config = CreateConfiguration();
		config.BlockGenerationTargetTime = utils::TimeSpan::FromMilliseconds(10000);
		config.MaxDifficultyBlocks = 15;

		// - seed the difficulty cache with 20 infos
		auto pCache = SeedBlockDifficultyCache(Height(20), config.BlockGenerationTargetTime);

		// Act: check the difficulties of an empty chain
		auto result = CheckDifficulties(*pCache, {}, config);

		// Assert:
		EXPECT_EQ(0u, result);
	}

	namespace {
		void AssertDifficultiesAreValidForBlocksWithEqualDifficulties(uint32_t maxDifficultyBlocks, Height chainHeight) {
			// Arrange: set up the config
			auto config = CreateConfiguration();
			config.BlockGenerationTargetTime = utils::TimeSpan::FromMilliseconds(10000);
			config.MaxDifficultyBlocks = maxDifficultyBlocks;

			// - seed the difficulty cache with chainHeight infos
			auto pCache = SeedBlockDifficultyCache(chainHeight, config.BlockGenerationTargetTime);

			// - generate a peer chain with five blocks
			auto blocks = GenerateBlocks(chainHeight, config.BlockGenerationTargetTime, 5);

			// Act:
			auto result = CheckDifficulties(*pCache, Unwrap(blocks), config);

			// Assert:
			EXPECT_EQ(5u, result);
		}
	}

	TEST(TEST_CLASS, DifficultiesAreValidIfAllDifficultiesAreCorrectAndFullHistoryIsPresent_Equal) {
		// Assert:
		AssertDifficultiesAreValidForBlocksWithEqualDifficulties(15, Height(20));
	}

	TEST(TEST_CLASS, DifficultiesAreValidIfAllDifficultiesAreCorrectAndPartialHistoryIsPresent_Equal) {
		// Assert:
		AssertDifficultiesAreValidForBlocksWithEqualDifficulties(15, Height(5));
	}

	namespace {
		void AssertDifficultiesAreValidForBlocksWithIncreasingDifficulties(uint32_t maxDifficultyBlocks, Height chainHeight) {
			// Arrange: set up the config
			auto config = CreateConfiguration();
			config.BlockGenerationTargetTime = utils::TimeSpan::FromMilliseconds(10000);
			config.MaxDifficultyBlocks = maxDifficultyBlocks;

			// - seed the difficulty cache with chainHeight infos and copy all the infos
			using DifficultySet = cache::BlockDifficultyCacheTypes::BaseSetType::SetType;
			auto pCache = SeedBlockDifficultyCache(chainHeight, utils::TimeSpan::FromMilliseconds(9000));
			DifficultySet set;
			{
				auto view = pCache->createView();
				auto range = view->difficultyInfos(chainHeight - Height(1), view->size());
				set.insert(range.begin(), range.end());
			}

			// - generate a peer chain with seven blocks
			auto blocks = GenerateBlocks(chainHeight, utils::TimeSpan::FromMilliseconds(8000), 7);
			auto lastDifficulty = Difficulty();
			for (const auto& pBlock : blocks) {
				// - ensure only the maxDifficultyBlocks recent infos are used
				while (set.size() > maxDifficultyBlocks)
					set.erase(set.cbegin());

				// - calculate the expected difficulty
				pBlock->Difficulty = CalculateDifficulty(cache::DifficultyInfoRange(set.cbegin(), set.cend()), config);
				set.insert(state::BlockDifficultyInfo(pBlock->Height, pBlock->Timestamp, pBlock->Difficulty));

				// Sanity:
				EXPECT_NE(Difficulty(), pBlock->Difficulty);
				EXPECT_LE(lastDifficulty, pBlock->Difficulty);
				lastDifficulty = pBlock->Difficulty;
			}

			// Act:
			auto result = CheckDifficulties(*pCache, Unwrap(blocks), config);

			// Assert:
			EXPECT_EQ(7u, result);
		}
	}

	TEST(TEST_CLASS, DifficultiesAreValidIfAllDifficultiesAreCorrectAndFullHistoryIsPresent_Increasing) {
		// Assert:
		AssertDifficultiesAreValidForBlocksWithIncreasingDifficulties(15, Height(20));
	}

	TEST(TEST_CLASS, DifficultiesAreValidIfAllDifficultiesAreCorrectAndPartialHistoryIsPresent_Increasing) {
		// Assert:
		AssertDifficultiesAreValidForBlocksWithIncreasingDifficulties(15, Height(5));
	}

	namespace {
		void AssertDifficultiesAreInvalidForDifferenceAt(
				uint32_t maxDifficultyBlocks,
				Height chainHeight,
				uint32_t peerChainSize,
				size_t differenceIndex) {
			// Arrange: set up the config
			auto config = CreateConfiguration();
			config.BlockGenerationTargetTime = utils::TimeSpan::FromMilliseconds(10000);
			config.MaxDifficultyBlocks = maxDifficultyBlocks;

			// - seed the difficulty cache with chainHeight infos
			auto pCache = SeedBlockDifficultyCache(chainHeight, config.BlockGenerationTargetTime);

			// - generate a peer chain with peerChainSize blocks and change the difficulty of the block
			//   at differenceIndex
			auto blocks = GenerateBlocks(chainHeight, config.BlockGenerationTargetTime, peerChainSize);
			blocks[differenceIndex]->Difficulty = Difficulty(Base_Difficulty + 1);

			// Act:
			auto result = CheckDifficulties(*pCache, Unwrap(blocks), config);

			// Assert:
			EXPECT_EQ(differenceIndex, result);
		}
	}

	TEST(TEST_CLASS, DifficultiesAreInvalidIfFirstBlockHasIncorrectDifficulty) {
		// Assert:
		AssertDifficultiesAreInvalidForDifferenceAt(15, Height(20), 5, 0);
	}

	TEST(TEST_CLASS, DifficultiesAreInvalidIfMiddleBlockHasIncorrectDifficulty) {
		// Assert:
		AssertDifficultiesAreInvalidForDifferenceAt(15, Height(20), 5, 2);
	}

	TEST(TEST_CLASS, DifficultiesAreInvalidIfLastBlockHasIncorrectDifficulty) {
		// Assert:
		AssertDifficultiesAreInvalidForDifferenceAt(15, Height(20), 5, 4);
	}

	// endregion

	// region CalculatePartialChainScore

	namespace {
		std::unique_ptr<model::Block> CreateBlock(Timestamp timestamp, Difficulty difficulty) {
			auto pBlock = std::make_unique<model::Block>();
			pBlock->Timestamp = timestamp;
			pBlock->Difficulty = difficulty;
			return pBlock;
		}
	}

	TEST(TEST_CLASS, CanCalculatePartialChainScoreForEmptyChain) {
		// Arrange:
		auto pParentBlock = CreateBlock(Timestamp(100'000), Difficulty());

		// Act:
		auto score = CalculatePartialChainScore(*pParentBlock, {});

		// Assert:
		EXPECT_EQ(model::ChainScore(0), score);
	}

	TEST(TEST_CLASS, CanCalculatePartialChainScoreForSingleBlockChain) {
		// Arrange:
		auto pParentBlock = CreateBlock(Timestamp(100'000), Difficulty());
		std::vector<std::unique_ptr<model::Block>> blocks;
		blocks.push_back(CreateBlock(Timestamp(150'000), Difficulty() + Difficulty::Unclamped(111)));

		// Act:
		auto score = CalculatePartialChainScore(*pParentBlock, { blocks[0].get() });

		// Assert:
		EXPECT_EQ(model::ChainScore(Base_Difficulty + 111 - (150 - 100)), score);
	}

	TEST(TEST_CLASS, CanCalculatePartialChainScoreForMultiBlockChain) {
		// Arrange:
		auto pParentBlock = CreateBlock(Timestamp(100'000), Difficulty());
		std::vector<std::unique_ptr<model::Block>> blocks;
		blocks.push_back(CreateBlock(Timestamp(150'000), Difficulty() + Difficulty::Unclamped(111)));
		blocks.push_back(CreateBlock(Timestamp(175'000), Difficulty() + Difficulty::Unclamped(200)));
		blocks.push_back(CreateBlock(Timestamp(190'000), Difficulty() + Difficulty::Unclamped(300)));

		// Act:
		auto score = CalculatePartialChainScore(*pParentBlock, { blocks[0].get(), blocks[1].get(), blocks[2].get() });

		// Assert:
		EXPECT_EQ(model::ChainScore(3 * Base_Difficulty + 111 - (150 - 100) + 200 - (175 - 150) + 300 - (190 - 175)), score);
	}

	// endregion
}}
