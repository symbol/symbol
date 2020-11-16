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

#include "harvesting/src/ScheduledHarvesterTask.h"
#include "harvesting/src/Harvester.h"
#include "catapult/cache_core/BlockStatisticCache.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/test/nodeps/TestConstants.h"
#include "tests/TestHarness.h"

using catapult::crypto::KeyPair;

namespace catapult { namespace harvesting {

#define TEST_CLASS ScheduledHarvesterTaskTests

	namespace {
		constexpr Timestamp Max_Time(std::numeric_limits<int64_t>::max());

		model::BlockChainConfiguration CreateConfiguration() {
			auto config = model::BlockChainConfiguration::Uninitialized();
			config.BlockGenerationTargetTime = utils::TimeSpan::FromSeconds(60);
			config.BlockTimeSmoothingFactor = 0;
			config.MaxDifficultyBlocks = 60;
			config.ImportanceGrouping = 123;
			config.TotalChainImportance = test::Default_Total_Chain_Importance;
			return config;
		}

		struct TaskOptionsWithCounters : ScheduledHarvesterTaskOptions {
			TaskOptionsWithCounters()
					: NumHarvestingAllowedCalls(0)
					, NumLastBlockElementSupplierCalls(0)
					, NumTimeSupplierCalls(0)
					, NumRangeConsumerCalls(0)
					, BlockHeight(0)
					, BlockSigner()
					, pLastBlock(std::make_shared<model::Block>())
					, LastBlockHash(test::GenerateRandomByteArray<Hash256>()) {
				HarvestingAllowed = [this]() {
					++NumHarvestingAllowedCalls;
					return true;
				};
				LastBlockElementSupplier = [this]() {
					++NumLastBlockElementSupplierCalls;

					auto pLastElement = std::make_shared<model::BlockElement>(*pLastBlock);
					pLastElement->EntityHash = LastBlockHash;
					return pLastElement;
				};
				TimeSupplier = [this]() {
					++NumTimeSupplierCalls;
					return Max_Time;
				};
				RangeConsumer = [this](const auto& range, const auto& processingComplete) {
					++NumRangeConsumerCalls;
					const auto& block = *range.cbegin();
					BlockHeight = block.Height;
					BlockSigner = block.SignerPublicKey;
					CompletionFunction = processingComplete;
				};
				pLastBlock->Size = sizeof(model::BlockHeader);
				pLastBlock->Height = Height(1);
			}

			size_t NumHarvestingAllowedCalls;
			size_t NumLastBlockElementSupplierCalls;
			size_t NumTimeSupplierCalls;
			size_t NumRangeConsumerCalls;
			Height BlockHeight;
			Key BlockSigner;
			std::shared_ptr<model::Block> pLastBlock;
			Hash256 LastBlockHash;
			disruptor::ProcessingCompleteFunc CompletionFunction;
		};

		void AddStatistic(cache::CatapultCache& cache, const model::Block& block) {
			auto delta = cache.createDelta();
			auto& statisticCache = delta.sub<cache::BlockStatisticCache>();
			state::BlockStatistic statistic(block);
			statisticCache.insert(statistic);
			cache.commit(Height());
		}

		KeyPair AddImportantAccount(cache::CatapultCache& cache) {
			auto keyPair = KeyPair::FromPrivate(test::GenerateRandomPrivateKey());
			auto delta = cache.createDelta();
			auto& accountStateCache = delta.sub<cache::AccountStateCache>();
			accountStateCache.addAccount(keyPair.publicKey(), Height(1));
			auto& accountState = accountStateCache.find(keyPair.publicKey()).get();
			accountState.ImportanceSnapshots.set(Importance(1'000'000'000), model::ImportanceHeight(1));
			cache.commit(Height(1));
			return test::CopyKeyPair(keyPair);
		}

		void UnlockAccount(UnlockedAccounts& unlockedAccounts, const KeyPair& keyPair) {
			auto modifier = unlockedAccounts.modifier();
			modifier.add(BlockGeneratorAccountDescriptor(test::CopyKeyPair(keyPair), test::GenerateKeyPair()));
		}

		struct HarvesterContext {
			HarvesterContext(const model::Block& lastBlock)
					: Config(CreateConfiguration())
					, Cache(test::CreateEmptyCatapultCache(Config))
					, Accounts(1, [](const auto&) { return 0; }) {
				AddStatistic(Cache, lastBlock);
			}

			model::BlockChainConfiguration Config;
			cache::CatapultCache Cache;
			UnlockedAccounts Accounts;
		};

		auto CreateHarvester(HarvesterContext& context) {
			return std::make_unique<Harvester>(context.Cache, context.Config, Address(), context.Accounts, [](
					const auto& blockHeader,
					auto) {
				auto size = model::GetBlockHeaderSize(blockHeader.Type, blockHeader.Version);
				auto pBlock = utils::MakeUniqueWithSize<model::Block>(size);
				std::memcpy(static_cast<void*>(pBlock.get()), &blockHeader, size);
				return pBlock;
			});
		}
	}

	TEST(TEST_CLASS, TaskIsShortCircuitedWhenHarvestingIsNotAllowed) {
		// Arrange:
		TaskOptionsWithCounters options;
		options.HarvestingAllowed = [&options]() {
			++options.NumHarvestingAllowedCalls;
			return false;
		};
		HarvesterContext context(*options.pLastBlock);
		ScheduledHarvesterTask task(options, CreateHarvester(context));

		// Act:
		task.harvest();

		// Assert:
		EXPECT_EQ(1u, options.NumHarvestingAllowedCalls);
		EXPECT_EQ(0u, options.NumLastBlockElementSupplierCalls);
		EXPECT_EQ(0u, options.NumTimeSupplierCalls);
		EXPECT_EQ(0u, options.NumRangeConsumerCalls);
		EXPECT_EQ(Height(0), options.BlockHeight);
		EXPECT_EQ(Key(), options.BlockSigner);
	}

	TEST(TEST_CLASS, BlockConsumerIsNotCalledWhenNoBlockIsHarvested) {
		// Arrange:
		TaskOptionsWithCounters options;
		HarvesterContext context(*options.pLastBlock);
		ScheduledHarvesterTask task(options, CreateHarvester(context));

		// Act: no block can be harvested since no account is unlocked
		task.harvest();

		// Assert:
		EXPECT_EQ(1u, options.NumHarvestingAllowedCalls);
		EXPECT_EQ(1u, options.NumLastBlockElementSupplierCalls);
		EXPECT_EQ(1u, options.NumTimeSupplierCalls);
		EXPECT_EQ(0u, options.NumRangeConsumerCalls);
		EXPECT_EQ(Height(0), options.BlockHeight);
		EXPECT_EQ(Key(), options.BlockSigner);
	}

	TEST(TEST_CLASS, BlockConsumerIsCalledWhenBlockIsHarvested) {
		// Arrange:
		TaskOptionsWithCounters options;
		HarvesterContext context(*options.pLastBlock);
		auto keyPair = AddImportantAccount(context.Cache);
		UnlockAccount(context.Accounts, keyPair);
		ScheduledHarvesterTask task(options, CreateHarvester(context));

		// Act:
		task.harvest();

		// Assert:
		EXPECT_EQ(1u, options.NumHarvestingAllowedCalls);
		EXPECT_EQ(1u, options.NumLastBlockElementSupplierCalls);
		EXPECT_EQ(1u, options.NumTimeSupplierCalls);
		EXPECT_EQ(1u, options.NumRangeConsumerCalls);
		EXPECT_EQ(Height(2), options.BlockHeight);
		EXPECT_EQ(keyPair.publicKey(), options.BlockSigner);
	}

	TEST(TEST_CLASS, BlockConsumerIsNotCalledWhenLastHarvestedBlockIsStillBeingProcessed) {
		// Arrange:
		TaskOptionsWithCounters options;
		HarvesterContext context(*options.pLastBlock);
		auto keyPair = AddImportantAccount(context.Cache);
		UnlockAccount(context.Accounts, keyPair);
		ScheduledHarvesterTask task(options, CreateHarvester(context));

		// Act:
		task.harvest();
		task.harvest();

		// Assert: the second harvest did not push a block to the consumer
		// - the check for the processing complete flag is done before anything else
		EXPECT_EQ(1u, options.NumHarvestingAllowedCalls);
		EXPECT_EQ(1u, options.NumLastBlockElementSupplierCalls);
		EXPECT_EQ(1u, options.NumTimeSupplierCalls);
		EXPECT_EQ(1u, options.NumRangeConsumerCalls);
		EXPECT_EQ(Height(2), options.BlockHeight);
		EXPECT_EQ(keyPair.publicKey(), options.BlockSigner);
	}

	TEST(TEST_CLASS, BlockConsumerIsCalledAgainAfterLastHarvestedBlockWasCompletelyProcessed) {
		// Arrange:
		TaskOptionsWithCounters options;
		HarvesterContext context(*options.pLastBlock);
		auto keyPair = AddImportantAccount(context.Cache);
		UnlockAccount(context.Accounts, keyPair);
		ScheduledHarvesterTask task(options, CreateHarvester(context));

		// Act:
		task.harvest();
		options.CompletionFunction(0, disruptor::ConsumerCompletionResult());
		task.harvest();

		// Assert: the second harvest did push a block to the consumer
		EXPECT_EQ(2u, options.NumHarvestingAllowedCalls);
		EXPECT_EQ(2u, options.NumLastBlockElementSupplierCalls);
		EXPECT_EQ(2u, options.NumTimeSupplierCalls);
		EXPECT_EQ(2u, options.NumRangeConsumerCalls);
		EXPECT_EQ(Height(2), options.BlockHeight);
		EXPECT_EQ(keyPair.publicKey(), options.BlockSigner);
	}

	TEST(TEST_CLASS, BlockConsumerIsCalledAgainWhenSubsequentHarvestProducesBlock) {
		// Arrange:
		TaskOptionsWithCounters options;
		HarvesterContext context(*options.pLastBlock);
		ScheduledHarvesterTask task(options, CreateHarvester(context));

		// - harvest once (no account is unlocked)
		task.harvest();

		// Sanity:
		EXPECT_EQ(0u, options.NumRangeConsumerCalls);

		// Act: unlock an account and harvest again
		auto keyPair = AddImportantAccount(context.Cache);
		UnlockAccount(context.Accounts, keyPair);
		task.harvest();

		// Assert: the second harvest triggered a call to the consumer
		EXPECT_EQ(2u, options.NumHarvestingAllowedCalls);
		EXPECT_EQ(2u, options.NumLastBlockElementSupplierCalls);
		EXPECT_EQ(2u, options.NumTimeSupplierCalls);
		EXPECT_EQ(1u, options.NumRangeConsumerCalls);
		EXPECT_EQ(Height(2), options.BlockHeight);
		EXPECT_EQ(keyPair.publicKey(), options.BlockSigner);
	}
}}
