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

#include "filechain/src/MultiBlockLoader.h"
#include "catapult/extensions/LocalNodeChainScore.h"
#include "catapult/extensions/NemesisBlockLoader.h"
#include "catapult/extensions/PluginUtils.h"
#include "catapult/io/BlockStorageCache.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "filechain/tests/test/FilechainTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/mocks/MockMemoryBlockStorage.h"
#include "tests/test/local/BlockStateHash.h"
#include "tests/test/local/LocalNodeTestState.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/test/other/mocks/MockEntityObserver.h"
#include "tests/TestHarness.h"
#include <random>

namespace catapult { namespace filechain {

#define TEST_CLASS MultiBlockLoaderTests

	// region CreateBlockDependentEntityObserverFactory

	namespace {
		enum class ObserverFactoryResult { Transient, Permanent, Unknown };

		ObserverFactoryResult RunObserverFactoryInflectionPointTest(
				const model::Block& lastBlock,
				Height currentBlockHeight,
				Timestamp currentBlockTime) {
			// Arrange:
			auto pCurrentBlock = test::GenerateBlockWithTransactions(0, currentBlockHeight, currentBlockTime);

			// - create configuration
			auto config = model::BlockChainConfiguration::Uninitialized();
			config.MaxDifficultyBlocks = 100;
			config.BlockGenerationTargetTime = utils::TimeSpan::FromSeconds(2);
			config.MaxRollbackBlocks = 22;

			// - create observers
			mocks::MockEntityObserver transientObserver;
			mocks::MockEntityObserver permanentObserver;

			// Act:
			auto observerFactory = CreateBlockDependentEntityObserverFactory(lastBlock, config, transientObserver, permanentObserver);
			const auto& observer = observerFactory(*pCurrentBlock);

			// Assert:
			return &transientObserver == &observer
					? ObserverFactoryResult::Transient
					: &permanentObserver == &observer ? ObserverFactoryResult::Permanent : ObserverFactoryResult::Unknown;
		}
	}

	TEST(TEST_CLASS, ObserverFactoryRespectsHeightInflectionPoint) {
		// Arrange:
		auto pLastBlock = test::GenerateBlockWithTransactions(0, Height(1234), Timestamp(utils::TimeSpan::FromHours(2).millis()));
		auto runTest = [&lastBlock = *pLastBlock](auto height) {
			return RunObserverFactoryInflectionPointTest(lastBlock, height, Timestamp());
		};

		// Act + Assert: inflection point is `LastBlockHeight - MaxDifficultyBlocks + 1` [1234 - 100 + 1]
		auto inflectionHeight = Height(1234 - 100 + 1);
		EXPECT_EQ(ObserverFactoryResult::Permanent, runTest(inflectionHeight - Height(1)));
		EXPECT_EQ(ObserverFactoryResult::Transient, runTest(inflectionHeight));
		EXPECT_EQ(ObserverFactoryResult::Transient, runTest(inflectionHeight + Height(1)));
	}

	TEST(TEST_CLASS, ObserverFactoryReturnsTransientObserverWhenThereIsNoHeightInflectionPoint) {
		// Arrange:
		auto pLastBlock = test::GenerateBlockWithTransactions(0, Height(50), Timestamp(utils::TimeSpan::FromHours(2).millis()));
		auto runTest = [&lastBlock = *pLastBlock](auto height) {
			return RunObserverFactoryInflectionPointTest(lastBlock, height, Timestamp());
		};

		// Act + Assert: there is no height inflection point because `LastBlockHeight < MaxDifficultyBlocks` [50 < 100]
		EXPECT_EQ(ObserverFactoryResult::Transient, runTest(Height(1)));
		EXPECT_EQ(ObserverFactoryResult::Transient, runTest(Height(2)));
		EXPECT_EQ(ObserverFactoryResult::Transient, runTest(Height(50)));
	}

	TEST(TEST_CLASS, ObserverFactoryRespectsTimeInflectionPoint) {
		// Arrange:
		auto pLastBlock = test::GenerateBlockWithTransactions(0, Height(1234), Timestamp(utils::TimeSpan::FromHours(2).millis()));
		auto runTest = [&lastBlock = *pLastBlock](auto time) {
			return RunObserverFactoryInflectionPointTest(lastBlock, Height(1), time);
		};

		// Act + Assert: inflection point is `LastBlockTime - TransactionCacheDuration` [2H - (1H + 22 * 2s)]
		auto inflectionTime = Timestamp(utils::TimeSpan::FromHours(1).millis() - 22 * utils::TimeSpan::FromSeconds(2).millis());
		EXPECT_EQ(ObserverFactoryResult::Permanent, runTest(inflectionTime - Timestamp(1)));
		EXPECT_EQ(ObserverFactoryResult::Transient, runTest(inflectionTime));
		EXPECT_EQ(ObserverFactoryResult::Transient, runTest(inflectionTime + Timestamp(1)));
	}

	TEST(TEST_CLASS, ObserverFactoryReturnsTransientObserverWhenThereIsNoTimeInflectionPoint) {
		// Arrange:
		auto pLastBlock = test::GenerateBlockWithTransactions(0, Height(1234), Timestamp(utils::TimeSpan::FromHours(1).millis()));
		auto runTest = [&lastBlock = *pLastBlock](auto time) {
			return RunObserverFactoryInflectionPointTest(lastBlock, Height(1), time);
		};

		// Act + Assert: there is no time inflection point because `LastBlockTime < TransactionCacheDuration` [1H < (1H + 22 * 2s)]
		EXPECT_EQ(ObserverFactoryResult::Transient, runTest(Timestamp(0)));
		EXPECT_EQ(ObserverFactoryResult::Transient, runTest(Timestamp(1)));
		EXPECT_EQ(ObserverFactoryResult::Transient, runTest(Timestamp(utils::TimeSpan::FromHours(1).millis())));
	}

	// endregion

	// region LoadBlockChain

	namespace {
		auto MakeObserverFactory(const observers::EntityObserver& observer, std::vector<Height>& heights) {
			return [&observer, &heights](const auto& block) -> const observers::EntityObserver& {
				heights.push_back(block.Height);
				return observer;
			};
		}
	}

	TEST(TEST_CLASS, LoadBlockChainLoadsZeroBlocksWhenStorageHeightIsOne) {
		// Arrange:
		mocks::MockEntityObserver observer;
		std::vector<Height> factoryHeights;
		test::LocalNodeTestState state;

		// Act:
		auto score = LoadBlockChain(MakeObserverFactory(observer, factoryHeights), state.ref(), Height(2));

		// Assert:
		EXPECT_EQ(model::ChainScore(), score);
		EXPECT_EQ(0u, observer.blockHeights().size());
		EXPECT_EQ(0u, factoryHeights.size());
	}

	namespace {
		void SetStorageChainHeight(io::BlockStorageModifier&& storage, size_t height) {
			for (auto i = 2u; i <= height; ++i) {
				auto pBlock = test::GenerateBlockWithTransactions(0, Height(i), Timestamp(i * 3000));
				pBlock->Difficulty = Difficulty(Difficulty().unwrap() + i);
				storage.saveBlock(test::BlockToBlockElement(*pBlock));
			}
		}

		constexpr uint64_t CalculateExpectedScore(size_t height) {
			// - nemesis difficulty is 0 and nemesis time is 0
			// - all other blocks have a difficulty of base + height
			// - blocks at heights 1 and 2 have time difference of 6s
			// - all other blocks have a time difference of 3s
			return
					Difficulty().unwrap() * (height - 1) // sum base difficulties
					+ height * (height + 1) / 2 // sum difficulty deltas (1..N)
					- 1 // adjust for range (2..N), first block has height 2
					- (6 + (height - 2) * 3); // subtract the time differences
		}
	}

	TEST(TEST_CLASS, LoadBlockChainLoadsSingleBlockWhenStorageHeightIsTwo) {
		// Arrange:
		mocks::MockEntityObserver observer;
		std::vector<Height> factoryHeights;
		test::LocalNodeTestState state;
		SetStorageChainHeight(state.ref().Storage.modifier(), 2);

		// Act:
		auto score = LoadBlockChain(MakeObserverFactory(observer, factoryHeights), state.ref(), Height(2));

		// Assert:
		auto expectedHeights = std::vector<Height>{ Height(2) };
		EXPECT_EQ(model::ChainScore(CalculateExpectedScore(2)), score);
		EXPECT_EQ(1u, observer.blockHeights().size());
		EXPECT_EQ(expectedHeights, observer.blockHeights());
		EXPECT_EQ(expectedHeights, factoryHeights);
	}

	TEST(TEST_CLASS, LoadBlockChainLoadsMultipleBlocksWhenStorageHeightIsGreaterThanTwo) {
		// Arrange:
		mocks::MockEntityObserver observer;
		std::vector<Height> factoryHeights;
		test::LocalNodeTestState state;
		SetStorageChainHeight(state.ref().Storage.modifier(), 7);

		// Act:
		auto score = LoadBlockChain(MakeObserverFactory(observer, factoryHeights), state.ref(), Height(2));

		// Assert:
		auto expectedHeights = std::vector<Height>{ Height(2), Height(3), Height(4), Height(5), Height(6), Height(7) };
		EXPECT_EQ(model::ChainScore(CalculateExpectedScore(7)), score);
		EXPECT_EQ(6u, observer.blockHeights().size());
		EXPECT_EQ(expectedHeights, observer.blockHeights());
		EXPECT_EQ(expectedHeights, factoryHeights);
	}

	TEST(TEST_CLASS, LoadBlockChainLoadsMultipleBlocksStartingAtArbitraryHeight) {
		// Arrange: create a storage with 7 blocks
		mocks::MockEntityObserver observer;
		std::vector<Height> factoryHeights;
		test::LocalNodeTestState state;
		SetStorageChainHeight(state.ref().Storage.modifier(), 7);

		// Act: load blocks 4-7
		auto score = LoadBlockChain(MakeObserverFactory(observer, factoryHeights), state.ref(), Height(4));

		// Assert:
		auto expectedHeights = std::vector<Height>{ Height(4), Height(5), Height(6), Height(7) };
		EXPECT_EQ(model::ChainScore(CalculateExpectedScore(7) - CalculateExpectedScore(3)), score);
		EXPECT_EQ(4u, observer.blockHeights().size());
		EXPECT_EQ(expectedHeights, observer.blockHeights());
		EXPECT_EQ(expectedHeights, factoryHeights);
	}

	// endregion

	// region LoadBlockChain - state enabled

	namespace {
		std::vector<Address> GenerateDeterministicAddresses(size_t count) {
			std::vector<Address> addresses;
			for (auto i = 0u; i < count; ++i)
				addresses.push_back(Address{ { static_cast<uint8_t>(1 + i) } });

			return addresses;
		}

		auto CreateBlocks(size_t maxHeight) {
			// each block has at most 20 txes
			const auto numRecipientAccounts = (maxHeight - 1) * 20;

			std::mt19937_64 rnd(0x11223344'55667788ull);
			auto nemesisKeyPairs = test::GetNemesisKeyPairs();
			auto recipients = GenerateDeterministicAddresses(numRecipientAccounts);

			size_t recipientIndex = 0;
			std::vector<std::unique_ptr<model::Block>> blocks;
			for (auto height = 2u; height <= maxHeight; ++height)
				blocks.push_back(test::CreateBlock(nemesisKeyPairs, recipients[recipientIndex++], rnd, height).pBlock);

			return blocks;
		}

		void ExecuteNemesis(const extensions::LocalNodeStateRef& stateRef, plugins::PluginManager& pluginManager) {
			auto pPublisher = pluginManager.createNotificationPublisher();
			auto pObserver = extensions::CreateEntityObserver(pluginManager);
			auto cacheDelta = stateRef.Cache.createDelta();
			extensions::NemesisBlockLoader loader(cacheDelta, pluginManager.transactionRegistry(), *pPublisher, *pObserver);

			loader.executeAndCommit(stateRef, extensions::StateHashVerification::Disabled);
		}

		template<typename TAction>
		void ExecuteWithStorage(io::BlockStorageCache& storage, TAction action) {
			// Arrange:
			test::TempDirectoryGuard tempDataDirectory("../temp.dir");
			auto config = test::CreateStateHashEnabledLocalNodeConfiguration(tempDataDirectory.name());
			auto pPluginManager = test::CreatePluginManager(config);
			auto pObserver = extensions::CreateEntityObserver(*pPluginManager);

			// blockChain config copy
			auto blockChainConfig = pPluginManager->config();
			auto cache = pPluginManager->createCache();
			state::CatapultState state;
			extensions::LocalNodeChainScore score;
			auto localNodeConfig = test::CreateLocalNodeConfiguration(std::move(blockChainConfig), tempDataDirectory.name());

			extensions::LocalNodeStateRef stateRef(localNodeConfig, state, cache, storage, score);
			ExecuteNemesis(stateRef, *pPluginManager);

			// Act:
			std::vector<Height> factoryHeights;
			LoadBlockChain(MakeObserverFactory(*pObserver, factoryHeights), stateRef, Height(2));

			action(stateRef.Cache, *pPluginManager);
		}

		void RunLoadBlockChainTest(io::BlockStorageCache& storage, size_t maxHeight) {
			// Arrange: create one additional block to simplify test, blocks[0].height = 2
			auto blocks = CreateBlocks(maxHeight + 1);

			// - calculate expected state hash after loading first two blocks (1, 2)
			Hash256 expectedHash;
			ExecuteWithStorage(storage, [&expectedHash, &block = *blocks[0] ](auto& cache, const auto& pluginManager) {
				auto cacheDetachedDelta = cache.createDetachableDelta().detach();
				auto pCacheDelta = cacheDetachedDelta.lock();

				expectedHash = test::CalculateBlockStateHash(block, *pCacheDelta, pluginManager);
			});

			// Act:
			// - add single block to the storage
			// - compare current state hash with expected hash
			// - calculate next expected hash by using current cache state and next block
			for (auto height = 2u; height <= maxHeight; ++height) {
				const auto& block = *blocks[height - 2];
				storage.modifier().saveBlock(test::BlockToBlockElement(block));

				// - load whole chain and verify hash
				const auto& nextBlock = *blocks[height - 1];
				ExecuteWithStorage(storage, [&expectedHash, &nextBlock](auto& cache, const auto& pluginManager) {
					// Assert:
					// - retrieve state hash calculated when loading chain
					auto hashInfo = cache.createView().calculateStateHash();

					EXPECT_EQ(expectedHash, hashInfo.StateHash);

					// - calculate next expected hash
					auto cacheDetachedDelta = cache.createDetachableDelta().detach();
					auto pCacheDelta = cacheDetachedDelta.lock();

					expectedHash = test::CalculateBlockStateHash(nextBlock, *pCacheDelta, pluginManager);
				});
			}
		}
	}

	TEST(TEST_CLASS, LoadBlockChainLoadsMultipleBlocks_StateHashEnabled) {
		// Arrange:
		io::BlockStorageCache storage(std::make_unique<mocks::MockMemoryBlockStorage>());

		// Act + Assert:
		RunLoadBlockChainTest(storage, 7);
	}

	// endregion
}}
