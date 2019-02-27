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

#include "harvesting/src/Harvester.h"
#include "catapult/chain/BlockDifficultyScorer.h"
#include "catapult/chain/BlockScorer.h"
#include "catapult/model/Block.h"
#include "catapult/model/BlockUtils.h"
#include "catapult/model/EntityHasher.h"
#include "catapult/model/TransactionPlugin.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/EntityTestUtils.h"
#include "tests/test/core/KeyPairTestUtils.h"
#include "tests/test/nodeps/TestConstants.h"
#include "tests/test/nodeps/Waits.h"
#include "tests/TestHarness.h"

using catapult::crypto::KeyPair;

namespace catapult { namespace harvesting {

#define TEST_CLASS HarvesterTests

	namespace {
		// region constants / factory functions

		constexpr auto Network_Identifier = model::NetworkIdentifier::Mijin_Test;

		constexpr Timestamp Max_Time(std::numeric_limits<int64_t>::max());
		constexpr Importance Default_Importance(1'000'000);
		constexpr size_t Num_Accounts = 5;

		std::vector<KeyPair> CreateKeyPairs(size_t count) {
			std::vector<KeyPair> keyPairs;
			for (auto i = 0u; i < count; ++i)
				keyPairs.push_back(KeyPair::FromPrivate(test::GenerateRandomPrivateKey()));

			return keyPairs;
		}

		std::vector<Importance> CreateImportances(size_t count) {
			return std::vector<Importance>(count, Default_Importance);
		}

		void CreateAccounts(
				cache::AccountStateCacheDelta& cache,
				const std::vector<KeyPair>& keyPairs,
				const std::vector<Importance> importances) {
			for (auto i = 0u; i < keyPairs.size(); ++i) {
				cache.addAccount(keyPairs[i].publicKey(), Height(123));
				auto& accountState = cache.find(keyPairs[i].publicKey()).get();
				accountState.ImportanceInfo.set(importances[i], model::ImportanceHeight(1));
			}
		}

		std::unique_ptr<model::Block> CreateBlock() {
			// the created block needs to have height 1 to be able to add it to the block difficulty cache
			auto pBlock = test::GenerateEmptyRandomBlock();
			pBlock->Height = Height(1);
			pBlock->Timestamp = Timestamp();
			pBlock->Difficulty = Difficulty::Min();
			return pBlock;
		}

		model::BlockChainConfiguration CreateConfiguration() {
			auto config = model::BlockChainConfiguration::Uninitialized();
			config.Network.Identifier = Network_Identifier;
			config.BlockGenerationTargetTime = utils::TimeSpan::FromSeconds(60);
			config.BlockTimeSmoothingFactor = 0;
			config.MaxDifficultyBlocks = 60;
			config.ImportanceGrouping = 123;
			config.TotalChainImportance = test::Default_Total_Chain_Importance;
			return config;
		}

		// endregion

		// region HarvesterContext

		struct HarvesterContext {
		public:
			HarvesterContext()
					: Cache(test::CreateEmptyCatapultCache(CreateConfiguration()))
					, KeyPairs(CreateKeyPairs(Num_Accounts))
					, Importances(CreateImportances(Num_Accounts))
					, pUnlockedAccounts(std::make_unique<UnlockedAccounts>(Num_Accounts))
					, pLastBlock(CreateBlock())
					, LastBlockElement(test::BlockToBlockElement(*pLastBlock)) {
				auto delta = Cache.createDelta();
				CreateAccounts(delta.sub<cache::AccountStateCache>(), KeyPairs, Importances);

				auto& difficultyCache = delta.sub<cache::BlockDifficultyCache>();
				state::BlockDifficultyInfo info(pLastBlock->Height, pLastBlock->Timestamp, pLastBlock->Difficulty);
				difficultyCache.insert(info);
				Cache.commit(Height(1));
				UnlockAllAccounts(*pUnlockedAccounts, KeyPairs);

				LastBlockElement.GenerationHash = test::GenerateRandomData<Hash256_Size>();
			}

		public:
			std::unique_ptr<Harvester> CreateHarvester() {
				return CreateHarvester(CreateConfiguration());
			}

			std::unique_ptr<Harvester> CreateHarvester(const model::BlockChainConfiguration& config) {
				return CreateHarvester(config, [](const auto& blockHeader, auto) {
					auto pBlock = std::make_unique<model::Block>();
					std::memcpy(static_cast<void*>(pBlock.get()), &blockHeader, sizeof(model::BlockHeader));
					return pBlock;
				});
			}

			std::unique_ptr<Harvester> CreateHarvester(
					const model::BlockChainConfiguration& config,
					const BlockGenerator& blockGenerator) {
				return std::make_unique<Harvester>(Cache, config, *pUnlockedAccounts, blockGenerator);
			}

		private:
			static void UnlockAllAccounts(UnlockedAccounts& unlockedAccounts, const std::vector<KeyPair>& keyPairs) {
				auto modifier = unlockedAccounts.modifier();
				for (const auto& keyPair : keyPairs)
					modifier.add(test::CopyKeyPair(keyPair));
			}

		public:
			cache::CatapultCache Cache;
			std::vector<KeyPair> KeyPairs;
			std::vector<Importance> Importances;
			std::unique_ptr<UnlockedAccounts> pUnlockedAccounts;
			std::shared_ptr<model::Block> pLastBlock;
			model::BlockElement LastBlockElement;
		};

		// endregion

		// region test utils

		Key BestHarvesterKey(const model::BlockElement& lastBlockElement, const std::vector<KeyPair>& keyPairs) {
			const KeyPair* pBestKeyPair = nullptr;
			uint64_t bestHit = std::numeric_limits<uint64_t>::max();
			for (const auto& keyPair : keyPairs) {
				auto generationHash = model::CalculateGenerationHash(lastBlockElement.GenerationHash, keyPair.publicKey());
				uint64_t hit = chain::CalculateHit(generationHash);
				if (hit < bestHit) {
					bestHit = hit;
					pBestKeyPair = &keyPair;
				}
			}

			return pBestKeyPair->publicKey();
		}

		Timestamp CalculateBlockGenerationTime(const HarvesterContext& context, const Key& publicKey) {
			auto pLastBlock = context.pLastBlock;
			auto config = CreateConfiguration();
			auto difficulty = chain::CalculateDifficulty(context.Cache.sub<cache::BlockDifficultyCache>(), pLastBlock->Height, config);
			const auto& accountStateCache = context.Cache.sub<cache::AccountStateCache>();
			auto view = accountStateCache.createView();
			const auto& accountState = view->find(publicKey).get();
			uint64_t hit = chain::CalculateHit(model::CalculateGenerationHash(context.LastBlockElement.GenerationHash, publicKey));
			uint64_t referenceTarget = static_cast<uint64_t>(chain::CalculateTarget(
					utils::TimeSpan::FromMilliseconds(1000),
					difficulty,
					accountState.ImportanceInfo.current(),
					config));
			uint64_t seconds = hit / referenceTarget;
			return Timestamp((seconds + 1) * 1000);
		}

		// endregion
	}

	// region basic tests

	TEST(TEST_CLASS, HarvestReturnsNullptrWhenNoAccountIsUnlocked) {
		// Arrange:
		HarvesterContext context;
		{
			auto modifier = context.pUnlockedAccounts->modifier();
			for (const auto& keyPair : context.KeyPairs)
				modifier.remove(keyPair.publicKey());
		}

		auto pHarvester = context.CreateHarvester();

		// Sanity:
		EXPECT_EQ(0u, context.pUnlockedAccounts->view().size());

		// Act:
		auto pBlock = pHarvester->harvest(context.LastBlockElement, Max_Time);

		// Assert:
		EXPECT_FALSE(!!pBlock);
	}

	TEST(TEST_CLASS, HarvestReturnsBlockWhenEnoughTimeElapsed) {
		// Arrange:
		HarvesterContext context;
		auto pHarvester = context.CreateHarvester();

		// Act:
		auto pBlock = pHarvester->harvest(context.LastBlockElement, Max_Time);

		// Assert:
		EXPECT_TRUE(!!pBlock);
	}

	TEST(TEST_CLASS, HarvestReturnsNullptrWhenDifficultyCacheDoesNotContainInfoAtLastBlockHeight) {
		// Arrange:
		HarvesterContext context;
		auto pHarvester = context.CreateHarvester();
		auto numBlocks = CreateConfiguration().MaxDifficultyBlocks + 10;

		// - seed the block difficulty cache (it already has an entry for height 1)
		{
			auto delta = context.Cache.createDelta();
			auto& blockDifficultyCache = delta.sub<cache::BlockDifficultyCache>();
			for (auto i = 2u; i <= numBlocks; ++i)
				blockDifficultyCache.insert(state::BlockDifficultyInfo(Height(i), Timestamp(i * 1'000), Difficulty()));

			context.Cache.commit(Height(numBlocks));
		}

		// Act: set the last block to the max difficulty info in the cache and harvest
		context.pLastBlock->Height = Height(numBlocks);
		auto pBlock1 = pHarvester->harvest(context.LastBlockElement, Max_Time);

		// - set the last block to one past the max difficulty info in the cache and harvest
		context.pLastBlock->Height = Height(numBlocks + 1);
		auto pBlock2 = pHarvester->harvest(context.LastBlockElement, Max_Time);

		// Assert: only the first block could be harvested (there is insufficient difficulty info for the second one)
		EXPECT_TRUE(!!pBlock1);
		EXPECT_FALSE(!!pBlock2);
	}

	TEST(TEST_CLASS, HarvesterWithBestKeyCreatesBlockAtEarliestMoment) {
		// Arrange:
		// - the harvester accepts the first account that has a hit. That means that subsequent accounts might have
		// - a better (lower) hit but still won't be the signer of the block.
		test::RunNonDeterministicTest("harvester with best key harvests", []() {
			HarvesterContext context;
			auto bestKey = BestHarvesterKey(context.LastBlockElement, context.KeyPairs);
			auto timestamp = CalculateBlockGenerationTime(context, bestKey);
			auto tooEarly = Timestamp(timestamp.unwrap() - 1000);
			auto pHarvester = context.CreateHarvester();

			// Sanity: harvester cannot harvest a block before it's her turn
			auto pBlock1 = pHarvester->harvest(context.LastBlockElement, tooEarly);

			// Act: harvester should succeed at earliest possible time
			auto pBlock2 = pHarvester->harvest(context.LastBlockElement, timestamp);
			if (!pBlock2 || bestKey != pBlock2->Signer)
				return false;

			// Assert:
			EXPECT_FALSE(!!pBlock1);
			EXPECT_TRUE(!!pBlock2);
			EXPECT_EQ(bestKey, pBlock2->Signer);
			return true;
		});
	}

	TEST(TEST_CLASS, HarvestReturnsNullptrWhenNoHarvesterHasHit) {
		// Arrange:
		HarvesterContext context;
		auto bestKey = BestHarvesterKey(context.LastBlockElement, context.KeyPairs);
		auto timestamp = CalculateBlockGenerationTime(context, bestKey);
		auto tooEarly = Timestamp(timestamp.unwrap() - 1000);
		auto pHarvester = context.CreateHarvester();

		// Act:
		auto pBlock = pHarvester->harvest(context.LastBlockElement, tooEarly);

		// Assert:
		EXPECT_FALSE(!!pBlock);
	}

	TEST(TEST_CLASS, HarvestReturnsNullptrWhenNoHarvesterHasImportanceAtBlockHeight) {
		// Arrange:
		HarvesterContext context;
		auto pHarvester = context.CreateHarvester();

		// - modifiy account importances
		{
			auto cacheDelta = context.Cache.createDelta();
			auto& accountStateCache = cacheDelta.sub<cache::AccountStateCache>();
			for (const auto& keyPair : context.KeyPairs) {
				// - next block has height 2 and thus importance is expected to be set at height 1
				auto& accountState = accountStateCache.find(keyPair.publicKey()).get();
				accountState.ImportanceInfo.set(accountState.ImportanceInfo.current(), model::ImportanceHeight(360));
			}

			context.Cache.commit(Height());
		}

		// Act:
		auto pBlock = pHarvester->harvest(context.LastBlockElement, Max_Time);

		// Assert:
		EXPECT_FALSE(!!pBlock);
	}

	TEST(TEST_CLASS, HarvestReturnsNullptrWhenAccountsAreUnlockedButNotFoundInCache) {
		// Arrange:
		HarvesterContext context;
		auto pHarvester = context.CreateHarvester();

		{
			auto cacheDelta = context.Cache.createDelta();
			auto& accountStateCache = cacheDelta.sub<cache::AccountStateCache>();
			for (const auto& keyPair : context.KeyPairs)
				accountStateCache.queueRemove(keyPair.publicKey(), Height(123));

			accountStateCache.commitRemovals();
			context.Cache.commit(Height());

			// Sanity:
			EXPECT_EQ(0u, accountStateCache.size());
		}

		EXPECT_NE(0u, context.pUnlockedAccounts->view().size());

		// Act:
		auto pBlock = pHarvester->harvest(context.LastBlockElement, Max_Time);

		// Assert:
		EXPECT_FALSE(!!pBlock);
	}

	TEST(TEST_CLASS, HarvestHasFirstHarvesterWithHitAsSigner) {
		// Arrange:
		HarvesterContext context;
		auto pHarvester = context.CreateHarvester();
		auto firstPublicKey = context.pUnlockedAccounts->view().begin()->publicKey();

		// Act:
		auto pBlock = pHarvester->harvest(context.LastBlockElement, Max_Time);

		// Assert:
		ASSERT_TRUE(!!pBlock);
		EXPECT_EQ(firstPublicKey, pBlock->Signer);
	}

	TEST(TEST_CLASS, HarvestedBlockHasExpectedProperties) {
		// Arrange:
		// - the harvester accepts the first account that has a hit. That means that subsequent accounts might have
		// - a better (lower) hit but still won't be the signer of the block.
		test::RunNonDeterministicTest("harvested block has expected properties", []() {
			HarvesterContext context;
			auto pLastBlock = context.pLastBlock;
			auto bestKey = BestHarvesterKey(context.LastBlockElement, context.KeyPairs);
			auto timestamp = CalculateBlockGenerationTime(context, bestKey);
			auto pHarvester = context.CreateHarvester();
			const auto& difficultyCache = context.Cache.sub<cache::BlockDifficultyCache>();
			auto config = CreateConfiguration();

			// Act:
			auto pBlock = pHarvester->harvest(context.LastBlockElement, timestamp);
			if (!pBlock || bestKey != pBlock->Signer)
				return false;

			// Assert:
			EXPECT_TRUE(!!pBlock);
			EXPECT_EQ(timestamp, pBlock->Timestamp);
			EXPECT_EQ(Height(2), pBlock->Height);
			EXPECT_EQ(bestKey, pBlock->Signer);
			EXPECT_EQ(model::CalculateHash(*context.pLastBlock), pBlock->PreviousBlockHash);
			EXPECT_TRUE(model::VerifyBlockHeaderSignature(*pBlock));
			EXPECT_EQ(chain::CalculateDifficulty(difficultyCache, pLastBlock->Height, config), pBlock->Difficulty);
			EXPECT_EQ(model::MakeVersion(Network_Identifier, 3), pBlock->Version);
			EXPECT_EQ(model::Entity_Type_Block, pBlock->Type);
			EXPECT_TRUE(model::IsSizeValid(*pBlock, model::TransactionRegistry()));
			return true;
		});
	}

	TEST(TEST_CLASS, HarvesterRespectsCustomBlockChainConfiguration) {
		// Arrange: the custom configuration has a much higher target time and uses smoothing. After 24 hours
		//          the harvester using the default configuration is most likely able to harvest a block
		//          while with the custom configuration it is very unlikely to have a hit.
		auto customConfig = CreateConfiguration();
		customConfig.BlockGenerationTargetTime = utils::TimeSpan::FromHours(1000);
		customConfig.BlockTimeSmoothingFactor = 10000;

		auto numHarvester1Blocks = 0u;
		auto numHarvester2Blocks = 0u;
		for (auto i = 0u; i < test::GetMaxNonDeterministicTestRetries(); ++i) {
			HarvesterContext context;
			auto pHarvester1 = context.CreateHarvester(); // using default configuration
			auto pHarvester2 = context.CreateHarvester(customConfig);
			auto harvestTime = Timestamp(utils::TimeSpan::FromHours(24).millis());

			// Act:
			auto pBlock1 = pHarvester1->harvest(context.LastBlockElement, harvestTime);
			auto pBlock2 = pHarvester2->harvest(context.LastBlockElement, harvestTime);

			if (pBlock1)
				++numHarvester1Blocks;
			if (pBlock2)
				++numHarvester2Blocks;
		}

		// Assert: the first harvester (with default config) should have been able to harvest on every iteration
		CATAPULT_LOG(debug) << "harvested blocks: H1 = " << numHarvester1Blocks << ", H2 = " << numHarvester2Blocks;
		EXPECT_GT(numHarvester1Blocks, numHarvester2Blocks);
	}

	// endregion

	// region block generator delegation

	namespace {
		bool IsAnyKeyPairMatch(const std::vector<KeyPair>& keyPairs, const Key& key) {
			return std::any_of(keyPairs.cbegin(), keyPairs.cend(), [&key](const auto& keyPair) {
				return key == keyPair.publicKey();
			});
		}
	}

	TEST(TEST_CLASS, HarvestDelegatesToBlockGenerator) {
		// Arrange:
		HarvesterContext context;
		auto config = CreateConfiguration();
		config.MaxTransactionsPerBlock = 123;
		std::vector<std::pair<Key, uint32_t>> capturedParams;
		auto pHarvester = context.CreateHarvester(config, [&capturedParams](const auto& blockHeader, auto maxTransactionsPerBlock) {
			capturedParams.emplace_back(blockHeader.Signer, maxTransactionsPerBlock);
			auto pBlock = test::GenerateEmptyRandomBlock();
			pBlock->Signer = blockHeader.Signer;
			return pBlock;
		});

		// Act:
		auto pHarvestedBlock = pHarvester->harvest(context.LastBlockElement, Max_Time);

		// Assert: properly signed valid block was harvested
		ASSERT_TRUE(!!pHarvestedBlock);
		EXPECT_TRUE(model::VerifyBlockHeaderSignature(*pHarvestedBlock));

		// - generator was called with expected params
		ASSERT_EQ(1u, capturedParams.size());
		EXPECT_TRUE(IsAnyKeyPairMatch(context.KeyPairs, capturedParams[0].first));
		EXPECT_EQ(123u, capturedParams[0].second);

		// - block signer was passed to generator
		EXPECT_EQ(capturedParams[0].first, pHarvestedBlock->Signer);
	}

	TEST(TEST_CLASS, HarvestReturnsNullptrWhenBlockGeneratorFails) {
		// Arrange:
		HarvesterContext context;
		auto config = CreateConfiguration();
		config.MaxTransactionsPerBlock = 123;
		std::vector<std::pair<Key, uint32_t>> capturedParams;
		auto pHarvester = context.CreateHarvester(config, [&capturedParams](const auto& blockHeader, auto maxTransactionsPerBlock) {
			capturedParams.emplace_back(blockHeader.Signer, maxTransactionsPerBlock);
			return nullptr;
		});

		// Act:
		auto pHarvestedBlock = pHarvester->harvest(context.LastBlockElement, Max_Time);

		// Assert: no block was harvested
		EXPECT_FALSE(!!pHarvestedBlock);

		// - generator was called with expected params
		ASSERT_EQ(1u, capturedParams.size());
		EXPECT_TRUE(IsAnyKeyPairMatch(context.KeyPairs, capturedParams[0].first));
		EXPECT_EQ(123u, capturedParams[0].second);
	}

	// endregion
}}
