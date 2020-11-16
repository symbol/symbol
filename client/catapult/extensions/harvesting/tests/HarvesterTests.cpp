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

#include "harvesting/src/Harvester.h"
#include "catapult/chain/BlockDifficultyScorer.h"
#include "catapult/chain/BlockScorer.h"
#include "catapult/model/Address.h"
#include "catapult/model/Block.h"
#include "catapult/model/BlockUtils.h"
#include "catapult/model/EntityHasher.h"
#include "catapult/model/TransactionPlugin.h"
#include "tests/test/cache/AccountStateCacheTestUtils.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/EntityTestUtils.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/test/nodeps/TestConstants.h"
#include "tests/test/nodeps/Waits.h"
#include "tests/TestHarness.h"

using catapult::crypto::KeyPair;

namespace catapult { namespace harvesting {

#define TEST_CLASS HarvesterTests

	namespace {
		// region constants / factory functions

		constexpr auto Network_Identifier = model::NetworkIdentifier::Private_Test;
		constexpr auto Harvesting_Mosaic_Id = MosaicId(1234);
		constexpr auto Min_Voter_Balance = Amount(1000);

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

		std::unique_ptr<model::Block> CreateBlock(Height height) {
			auto pBlock = test::GenerateEmptyRandomBlock();
			pBlock->Height = height;
			pBlock->Timestamp = Timestamp();
			pBlock->Difficulty = Difficulty::Min();
			return pBlock;
		}

		model::BlockChainConfiguration CreateConfiguration() {
			auto config = model::BlockChainConfiguration::Uninitialized();
			config.Network.Identifier = Network_Identifier;
			config.HarvestingMosaicId = Harvesting_Mosaic_Id;
			config.BlockGenerationTargetTime = utils::TimeSpan::FromSeconds(60);
			config.BlockTimeSmoothingFactor = 0;
			config.ImportanceGrouping = 123;
			config.MaxDifficultyBlocks = 60;
			config.TotalChainImportance = test::Default_Total_Chain_Importance;
			config.MinVoterBalance = Min_Voter_Balance;
			return config;
		}

		// endregion

		// region HarvesterContext

		struct HarvesterDescriptor {
			Key SigningPublicKey;
			Key VrfPublicKey;
			crypto::VrfProof VrfProof;
		};

		struct HarvesterContext {
		public:
			HarvesterContext() : HarvesterContext(Height(1))
			{}

			explicit HarvesterContext(Height height)
					: Cache(test::CreateEmptyCatapultCache(CreateConfiguration()))
					, SigningKeyPairs(CreateKeyPairs(Num_Accounts))
					, VotingKeyPairs(CreateKeyPairs(Num_Accounts))
					, VrfKeyPairs(CreateKeyPairs(Num_Accounts))
					, Beneficiary(test::GenerateRandomByteArray<Address>())
					, Importances(CreateImportances(Num_Accounts))
					, pUnlockedAccounts(std::make_unique<UnlockedAccounts>(Num_Accounts, [](const auto&) { return 0; }))
					, pLastBlock(CreateBlock(height))
					, LastBlockElement(test::BlockToBlockElement(*pLastBlock)) {
				auto delta = Cache.createDelta();
				CreateAccounts(delta.sub<cache::AccountStateCache>(), SigningKeyPairs, VotingKeyPairs, VrfKeyPairs, Importances);

				auto& statisticCache = delta.sub<cache::BlockStatisticCache>();
				for (auto i = 1u; i <= height.unwrap(); ++i)
					statisticCache.insert(state::BlockStatistic(Height(i), Timestamp(i * 1'000), Difficulty(), BlockFeeMultiplier()));

				Cache.commit(height);

				UnlockAllAccounts(*pUnlockedAccounts, SigningKeyPairs, VrfKeyPairs);

				LastBlockElement.GenerationHash = test::GenerateRandomByteArray<GenerationHash>();
			}

		public:
			std::unique_ptr<Harvester> CreateHarvester() {
				return CreateHarvester(CreateConfiguration());
			}

			std::unique_ptr<Harvester> CreateHarvester(const model::BlockChainConfiguration& config) {
				return CreateHarvester(config, [](const auto& blockHeader, auto) {
					auto size = model::GetBlockHeaderSize(blockHeader.Type, blockHeader.Version);
					auto pBlock = utils::MakeUniqueWithSize<model::Block>(size);
					std::memcpy(static_cast<void*>(pBlock.get()), &blockHeader, size);
					return pBlock;
				});
			}

			std::unique_ptr<Harvester> CreateHarvester(
					const model::BlockChainConfiguration& config,
					const BlockGenerator& blockGenerator) {
				return std::make_unique<Harvester>(Cache, config, Beneficiary, *pUnlockedAccounts, blockGenerator);
			}

			HarvesterDescriptor BestHarvester() const {
				crypto::VrfProof bestVrfProof;
				uint64_t bestHit = std::numeric_limits<uint64_t>::max();
				size_t bestIndex = 0u;

				for (auto i = 0u; i < VrfKeyPairs.size(); ++i) {
					auto vrfProof = crypto::GenerateVrfProof(LastBlockElement.GenerationHash, VrfKeyPairs[i]);
					auto generationHash = model::CalculateGenerationHash(vrfProof.Gamma);
					uint64_t hit = chain::CalculateHit(generationHash);
					if (hit < bestHit) {
						bestHit = hit;
						bestVrfProof = vrfProof;
						bestIndex = i;
					}
				}

				return { SigningKeyPairs[bestIndex].publicKey(), VrfKeyPairs[bestIndex].publicKey(), bestVrfProof };
			}

			Timestamp CalculateBlockGenerationTime(const HarvesterDescriptor& harvesterDescriptor) const {
				auto config = CreateConfiguration();
				auto difficulty = chain::CalculateDifficulty(Cache.sub<cache::BlockStatisticCache>(), pLastBlock->Height, config);
				const auto& accountStateCache = Cache.sub<cache::AccountStateCache>();
				auto view = accountStateCache.createView();
				const auto& accountState = view->find(harvesterDescriptor.SigningPublicKey).get();
				uint64_t hit = chain::CalculateHit(model::CalculateGenerationHash(harvesterDescriptor.VrfProof.Gamma));
				uint64_t referenceTarget = static_cast<uint64_t>(chain::CalculateTarget(
						utils::TimeSpan::FromMilliseconds(1000),
						difficulty,
						accountState.ImportanceSnapshots.current(),
						config));
				uint64_t seconds = hit / referenceTarget;
				return Timestamp((seconds + 1) * 1000);
			}

			void AssertBlockFields(
					const HarvesterDescriptor& harvester,
					const Address& beneficiary,
					model::EntityType type,
					uint8_t version,
					Height height,
					Timestamp timestamp,
					const model::BlockChainConfiguration& config,
					const model::Block& block) const {
				const auto& statisticCache = Cache.sub<cache::BlockStatisticCache>();
				EXPECT_EQ(harvester.SigningPublicKey, block.SignerPublicKey) << height;
				EXPECT_EQ(version, block.Version) << height;
				EXPECT_EQ(Network_Identifier, block.Network) << height;
				EXPECT_EQ(type, block.Type) << height;
				EXPECT_EQ(height, block.Height) << height;
				EXPECT_EQ(timestamp, block.Timestamp) << height;
				EXPECT_EQ(chain::CalculateDifficulty(statisticCache, pLastBlock->Height, config), block.Difficulty) << height;
				EXPECT_EQ(model::CalculateHash(*pLastBlock), block.PreviousBlockHash) << height;
				EXPECT_TRUE(model::VerifyBlockHeaderSignature(block)) << height;
				EXPECT_TRUE(model::IsSizeValid(block, model::TransactionRegistry())) << height;

				const auto& vrfProof = block.GenerationHashProof;
				auto verifyResult = crypto::VerifyVrfProof(vrfProof, LastBlockElement.GenerationHash, harvester.VrfPublicKey);
				EXPECT_NE(Hash512(), verifyResult) << height;
				EXPECT_EQ(beneficiary, block.BeneficiaryAddress) << height;
			}

		private:
			static void CreateAccounts(
					cache::AccountStateCacheDelta& cache,
					const std::vector<KeyPair>& signingKeyPairs,
					const std::vector<KeyPair>& votingKeyPairs,
					const std::vector<KeyPair>& vrfKeyPairs,
					const std::vector<Importance> importances) {
				for (auto i = 0u; i < Num_Accounts; ++i) {
					cache.addAccount(signingKeyPairs[i].publicKey(), Height(123));
					auto& accountState = cache.find(signingKeyPairs[i].publicKey()).get();
					auto multiplier = static_cast<uint64_t>(i % 2 ? -2 : 2);
					accountState.Balances.credit(Harvesting_Mosaic_Id, Min_Voter_Balance + Amount(multiplier * (i + 1)));
					accountState.ImportanceSnapshots.set(importances[i], model::ImportanceHeight(1));
					accountState.SupplementalPublicKeys.vrf().set(vrfKeyPairs[i].publicKey());
					accountState.SupplementalPublicKeys.voting().add({
						votingKeyPairs[i].publicKey().copyTo<VotingKey>(),
						FinalizationEpoch(1),
						FinalizationEpoch(100)
					});
				}

				// the height does not influence the tests
				cache.updateHighValueAccounts(Height(123));
			}

			static void UnlockAllAccounts(
					UnlockedAccounts& unlockedAccounts,
					const std::vector<KeyPair>& signingKeyPairs,
					const std::vector<KeyPair>& vrfKeyPairs) {
				auto modifier = unlockedAccounts.modifier();
				for (auto i = 0u; i < Num_Accounts; ++i) {
					modifier.add(BlockGeneratorAccountDescriptor(
							test::CopyKeyPair(signingKeyPairs[i]),
							test::CopyKeyPair(vrfKeyPairs[i])));
				}
			}

		public:
			cache::CatapultCache Cache;
			std::vector<KeyPair> SigningKeyPairs;
			std::vector<KeyPair> VotingKeyPairs;
			std::vector<KeyPair> VrfKeyPairs;
			Address Beneficiary;
			std::vector<Importance> Importances;
			std::unique_ptr<UnlockedAccounts> pUnlockedAccounts;
			std::shared_ptr<model::Block> pLastBlock;
			model::BlockElement LastBlockElement;
		};

		// endregion
	}

	// region basic tests

	TEST(TEST_CLASS, HarvestReturnsNullptrWhenNoAccountIsUnlocked) {
		// Arrange:
		HarvesterContext context;
		{
			auto modifier = context.pUnlockedAccounts->modifier();
			for (const auto& keyPair : context.SigningKeyPairs)
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

	TEST(TEST_CLASS, HarvestReturnsNullptrWhenStatisticCacheDoesNotContainInfoAtLastBlockHeight) {
		// Arrange:
		HarvesterContext context;
		auto pHarvester = context.CreateHarvester();
		auto numBlocks = CreateConfiguration().MaxDifficultyBlocks + 10;

		// - seed the block statistic cache (it already has an entry for height 1)
		{
			auto delta = context.Cache.createDelta();
			auto& blockStatisticCache = delta.sub<cache::BlockStatisticCache>();
			for (auto i = 2u; i <= numBlocks; ++i)
				blockStatisticCache.insert(state::BlockStatistic(Height(i), Timestamp(i * 1'000), Difficulty(), BlockFeeMultiplier()));

			context.Cache.commit(Height(numBlocks));
		}

		// Act: set the last block to the max difficulty in the cache and harvest
		context.pLastBlock->Height = Height(numBlocks);
		auto pBlock1 = pHarvester->harvest(context.LastBlockElement, Max_Time);

		// - set the last block to one past the max difficulty in the cache and harvest
		context.pLastBlock->Height = Height(numBlocks + 1);
		auto pBlock2 = pHarvester->harvest(context.LastBlockElement, Max_Time);

		// Assert: only the first block could be harvested (there is insufficient difficulty for the second one)
		EXPECT_TRUE(!!pBlock1);
		EXPECT_FALSE(!!pBlock2);
	}

	TEST(TEST_CLASS, HarvesterWithBestKeyCreatesBlockAtEarliestMoment) {
		// Arrange:
		// - the harvester accepts the first account that has a hit. That means that subsequent accounts might have
		// - a better (lower) hit but still won't be the signer of the block.
		test::RunNonDeterministicTest("harvester with best key harvests", []() {
			HarvesterContext context;
			auto bestHarvester = context.BestHarvester();
			auto timestamp = context.CalculateBlockGenerationTime(bestHarvester);
			auto tooEarly = Timestamp(timestamp.unwrap() - 1000);
			auto pHarvester = context.CreateHarvester();

			// Sanity: harvester cannot harvest a block before it's her turn
			auto pBlock1 = pHarvester->harvest(context.LastBlockElement, tooEarly);

			// Act: harvester should succeed at earliest possible time
			auto pBlock2 = pHarvester->harvest(context.LastBlockElement, timestamp);
			if (!pBlock2 || bestHarvester.SigningPublicKey != pBlock2->SignerPublicKey)
				return false;

			// Assert:
			EXPECT_FALSE(!!pBlock1);
			EXPECT_TRUE(!!pBlock2);
			EXPECT_EQ(bestHarvester.SigningPublicKey, pBlock2->SignerPublicKey);
			return true;
		});
	}

	TEST(TEST_CLASS, HarvestReturnsNullptrWhenNoHarvesterHasHit) {
		// Arrange:
		HarvesterContext context;
		auto bestHarvester = context.BestHarvester();
		auto timestamp = context.CalculateBlockGenerationTime(bestHarvester);
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
			for (const auto& keyPair : context.SigningKeyPairs) {
				// - next block has height 2 and thus importance is expected to be set at height 1
				auto& accountState = accountStateCache.find(keyPair.publicKey()).get();
				accountState.ImportanceSnapshots.set(accountState.ImportanceSnapshots.current(), model::ImportanceHeight(360));
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
			for (const auto& keyPair : context.SigningKeyPairs)
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
		Key firstPublicKey;
		context.pUnlockedAccounts->view().forEach([&firstPublicKey](const auto& descriptor) {
			firstPublicKey = descriptor.signingKeyPair().publicKey();
			return false;
		});

		// Act:
		auto pBlock = pHarvester->harvest(context.LastBlockElement, Max_Time);

		// Assert:
		ASSERT_TRUE(!!pBlock);
		EXPECT_EQ(firstPublicKey, pBlock->SignerPublicKey);
	}

	namespace {
		void AssertHarvestedBlockHasExpectedProperties(
				const Address& beneficiary,
				const std::function<Address (const Key&)>& expectedBeneficiaryAccessor) {
			// Arrange:
			// - the harvester accepts the first account that has a hit. That means that subsequent accounts might have
			// - a better (lower) hit but still won't be the signer of the block.
			test::RunNonDeterministicTest("harvested block has expected properties", [&beneficiary, &expectedBeneficiaryAccessor]() {
				HarvesterContext context;
				context.Beneficiary = beneficiary;

				auto pLastBlock = context.pLastBlock;
				auto bestHarvester = context.BestHarvester();
				auto timestamp = context.CalculateBlockGenerationTime(bestHarvester);
				auto pHarvester = context.CreateHarvester();
				auto config = CreateConfiguration();

				// Act:
				auto pBlock = pHarvester->harvest(context.LastBlockElement, timestamp);
				if (!pBlock || bestHarvester.SigningPublicKey != pBlock->SignerPublicKey)
					return false;

				// Assert:
				EXPECT_TRUE(!!pBlock);

				auto entityType = model::Entity_Type_Block_Normal;
				auto expectedBeneficiary = expectedBeneficiaryAccessor(bestHarvester.SigningPublicKey);
				context.AssertBlockFields(bestHarvester, expectedBeneficiary, entityType, 2, Height(2), timestamp, config, *pBlock);
				return true;
			});
		}

		void AssertHarvestedBlockHasExpectedType(model::EntityType type, uint8_t version, Height height, Height forkHeight) {
			test::RunNonDeterministicTest("harvested block has expected type", [type, version, height, forkHeight]() {
				// Arrange:
				HarvesterContext context(height - Height(1));
				auto config = CreateConfiguration();
				config.ImportanceGrouping = 5;
				config.ForkHeights.ImportanceBlock = forkHeight;

				auto bestHarvester = context.BestHarvester();
				auto timestamp = context.CalculateBlockGenerationTime(bestHarvester);
				auto pHarvester = context.CreateHarvester(config);

				// Act:
				auto pBlock = pHarvester->harvest(context.LastBlockElement, timestamp);
				if (!pBlock || bestHarvester.SigningPublicKey != pBlock->SignerPublicKey)
					return false;

				// Assert:
				context.AssertBlockFields(bestHarvester, context.Beneficiary, type, version, height, timestamp, config, *pBlock);
				return true;
			});
		}
	}

	TEST(TEST_CLASS, HarvestedBlockHasExpectedProperties_WithBeneficiary) {
		auto beneficiary = test::GenerateRandomByteArray<Address>();
		AssertHarvestedBlockHasExpectedProperties(beneficiary, [&beneficiary](const auto&) { return beneficiary; });
	}

	TEST(TEST_CLASS, HarvestedBlockHasExpectedProperties_WithoutBeneficiary) {
		AssertHarvestedBlockHasExpectedProperties(Address(), [](const auto& signer) {
			return model::PublicKeyToAddress(signer, Network_Identifier);
		});
	}

	TEST(TEST_CLASS, HarvestedBlockHasExpectedTypePreFork_Normal) {
		for (auto height : { 2u, 3u, 4u, 6u, 7u, 8u, 9u })
			AssertHarvestedBlockHasExpectedType(model::Entity_Type_Block_Normal, 1, Height(height), Height(20));
	}

	TEST(TEST_CLASS, HarvestedBlockHasExpectedTypePreFork_Importance) {
		for (auto height : { 5u, 10u, 15u })
			AssertHarvestedBlockHasExpectedType(model::Entity_Type_Block_Normal, 1, Height(height), Height(20));
	}

	TEST(TEST_CLASS, HarvestedBlockHasExpectedTypePostFork_Normal) {
		for (auto height : { 2u, 3u, 4u, 6u, 7u, 8u, 9u })
			AssertHarvestedBlockHasExpectedType(model::Entity_Type_Block_Normal, 2, Height(height), Height(0));
	}

	TEST(TEST_CLASS, HarvestedBlockHasExpectedTypePostFork_Importance) {
		for (auto height : { 5u, 10u, 15u })
			AssertHarvestedBlockHasExpectedType(model::Entity_Type_Block_Importance, 2, Height(height), Height(0));
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
			capturedParams.emplace_back(blockHeader.SignerPublicKey, maxTransactionsPerBlock);
			auto pBlock = test::GenerateEmptyRandomBlock();
			pBlock->SignerPublicKey = blockHeader.SignerPublicKey;
			return pBlock;
		});

		// Act:
		auto pHarvestedBlock = pHarvester->harvest(context.LastBlockElement, Max_Time);

		// Assert: properly signed valid block was harvested
		ASSERT_TRUE(!!pHarvestedBlock);
		EXPECT_TRUE(model::VerifyBlockHeaderSignature(*pHarvestedBlock));

		// - generator was called with expected params
		ASSERT_EQ(1u, capturedParams.size());
		EXPECT_TRUE(IsAnyKeyPairMatch(context.SigningKeyPairs, capturedParams[0].first));
		EXPECT_EQ(123u, capturedParams[0].second);

		// - block signer was passed to generator
		EXPECT_EQ(capturedParams[0].first, pHarvestedBlock->SignerPublicKey);
	}

	TEST(TEST_CLASS, HarvestReturnsNullptrWhenBlockGeneratorFails) {
		// Arrange:
		HarvesterContext context;
		auto config = CreateConfiguration();
		config.MaxTransactionsPerBlock = 123;
		std::vector<std::pair<Key, uint32_t>> capturedParams;
		auto pHarvester = context.CreateHarvester(config, [&capturedParams](const auto& blockHeader, auto maxTransactionsPerBlock) {
			capturedParams.emplace_back(blockHeader.SignerPublicKey, maxTransactionsPerBlock);
			return nullptr;
		});

		// Act:
		auto pHarvestedBlock = pHarvester->harvest(context.LastBlockElement, Max_Time);

		// Assert: no block was harvested
		EXPECT_FALSE(!!pHarvestedBlock);

		// - generator was called with expected params
		ASSERT_EQ(1u, capturedParams.size());
		EXPECT_TRUE(IsAnyKeyPairMatch(context.SigningKeyPairs, capturedParams[0].first));
		EXPECT_EQ(123u, capturedParams[0].second);
	}

	// endregion
}}
