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

#include "extensions/harvesting/src/BlockExecutionHashesCalculator.h"
#include "extensions/harvesting/src/Harvester.h"
#include "extensions/harvesting/src/HarvestingUtFacadeFactory.h"
#include "plugins/services/hashcache/src/cache/HashCacheStorage.h"
#include "plugins/services/hashcache/src/plugins/MemoryHashCacheSystem.h"
#include "catapult/cache/MemoryUtCache.h"
#include "catapult/cache/ReadOnlyCatapultCache.h"
#include "catapult/cache_core/BlockDifficultyCache.h"
#include "catapult/extensions/ConfigurationUtils.h"
#include "catapult/extensions/ExecutionConfigurationFactory.h"
#include "catapult/model/EntityHasher.h"
#include "catapult/observers/NotificationObserverAdapter.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/local/RealTransactionFactory.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/test/nodeps/TestConstants.h"
#include "tests/TestHarness.h"
#include <boost/thread.hpp>

namespace catapult { namespace harvesting {

#define TEST_CLASS HarvesterIntegrityTests

	namespace {
		uint64_t GetNumIterations() {
			return test::GetStressIterationCount() ? 5'000 : 250;
		}

		// region test factories

		std::shared_ptr<plugins::PluginManager> CreatePluginManager() {
			// include memory hash cache system to better trigger the race condition under test
			auto config = test::CreateLocalNodeBlockChainConfiguration();
			config.Plugins.emplace("catapult.plugins.transfer", utils::ConfigurationBag({{ "", { { "maxMessageSize", "0" } } }}));
			auto pPluginManager = test::CreatePluginManager(config);
			plugins::RegisterMemoryHashCacheSystem(*pPluginManager);
			return pPluginManager;
		}

		auto CreateConfiguration() {
			auto config = test::CreateLocalNodeBlockChainConfiguration();
			config.ShouldEnableVerifiableState = true;
			return config;
		}

		cache::CatapultCache CreateCatapultCache(const std::string& databaseDirectory) {
			auto cacheId = cache::HashCache::Id;
			auto config = CreateConfiguration();
			auto cacheConfig = cache::CacheConfiguration(databaseDirectory, utils::FileSize(), cache::PatriciaTreeStorageMode::Enabled);

			std::vector<std::unique_ptr<cache::SubCachePlugin>> subCaches(cacheId + 1);
			test::CoreSystemCacheFactory::CreateSubCaches(config, subCaches);
			auto transactionCacheDuration = CalculateTransactionCacheDuration(config);
			subCaches[cacheId] = test::MakeSubCachePlugin<cache::HashCache, cache::HashCacheStorage>(transactionCacheDuration);
			return cache::CatapultCache(std::move(subCaches));
		}

		// endregion

		// region HarvesterTestContext

		class HarvesterTestContext {
		public:
			HarvesterTestContext()
					: m_pPluginManager(CreatePluginManager())
					, m_config(test::CreateLocalNodeConfiguration(CreateConfiguration(), ""))
					, m_transactionsCache(cache::MemoryCacheOptions(1024, 1000))
					, m_cache(CreateCatapultCache(m_dbDirGuard.name()))
					, m_unlockedAccounts(100) {
				// create the harvester
				auto executionConfig = extensions::CreateExecutionConfiguration(*m_pPluginManager);
				HarvestingUtFacadeFactory utFacadeFactory(m_cache, extensions::GetUtCacheOptions(m_config.Node), executionConfig);

				Harvester::Suppliers harvesterSuppliers{
					[&cache = m_cache, &blockChainConfig = m_config.BlockChain, executionConfig](
							const auto& block,
							const auto& transactionHashes) {
						return CalculateBlockExecutionHashes(block, transactionHashes, cache, blockChainConfig, executionConfig);
					},
					[utFacadeFactory, &utCache = m_transactionsCache](auto height, auto count) {
						auto strategy = model::TransactionSelectionStrategy::Oldest;
						return CreateTransactionsInfoSupplier(strategy, utFacadeFactory, utCache)(height, count);
					}
				};

				m_pHarvester = std::make_unique<Harvester>(m_cache, m_config.BlockChain, m_unlockedAccounts, harvesterSuppliers);
			}

		public:
			cache::MemoryUtCache& transactionsCache() {
				return m_transactionsCache;
			}

			cache::CatapultCache& cache() {
				return m_cache;
			}

			Harvester& harvester() {
				return *m_pHarvester;
			}

		public:
			std::unique_ptr<model::Block> createLastBlock() {
				// create fake nemesis block
				auto pLastBlock = test::GenerateEmptyRandomBlock();
				pLastBlock->Height = Height(1);

				auto cacheDelta = m_cache.createDelta();
				auto& difficultyCache = cacheDelta.sub<cache::BlockDifficultyCache>();
				state::BlockDifficultyInfo difficultyInfo(pLastBlock->Height, pLastBlock->Timestamp, pLastBlock->Difficulty);
				difficultyCache.insert(difficultyInfo);
				m_cache.commit(Height(1));
				return pLastBlock;
			}

			void prepareSenderAccountAndTransactions(crypto::KeyPair&& keyPair, Timestamp deadline) {
				// 1. seed an account with an initial balance of N
				{
					auto mosaicId = test::Default_Currency_Mosaic_Id;
					auto cacheDelta = m_cache.createDelta();
					auto& accountStateCacheDelta = cacheDelta.sub<cache::AccountStateCache>();
					accountStateCacheDelta.addAccount(keyPair.publicKey(), Height(1));
					auto accountStateIter = accountStateCacheDelta.find(keyPair.publicKey());
					accountStateIter.get().Balances.credit(mosaicId, Amount(GetNumIterations()));
					accountStateIter.get().ImportanceInfo.set(Importance(10'000'000), model::ImportanceHeight(1));
					m_cache.commit(Height(1));
				}

				// 2. seed the UT cache with N txes
				auto recipient = test::GenerateRandomData<Key_Size>();
				for (auto i = 0u; i < GetNumIterations(); ++i) {
					auto pTransaction = test::CreateTransferTransaction(keyPair, recipient, Amount(1));
					pTransaction->MaxFee = Amount(0);
					pTransaction->Deadline = deadline;

					auto transactionHash = model::CalculateHash(*pTransaction);
					model::TransactionInfo transactionInfo(std::move(pTransaction), transactionHash);
					m_transactionsCache.modifier().add(std::move(transactionInfo));
				}

				// 3. unlock the account
				m_unlockedAccounts.modifier().add(std::move(keyPair));
			}

			void execute(const model::TransactionInfo& transactionInfo) {
				auto cacheDelta = m_cache.createDelta();
				auto catapultState = state::CatapultState();
				auto observerState = observers::ObserverState(cacheDelta, catapultState);

				// 4. prepare resolvers
				auto readOnlyCache = cacheDelta.toReadOnly();
				auto resolverContext = m_pPluginManager->createResolverContext(readOnlyCache);

				// 5. execute block
				auto notifyMode = observers::NotifyMode::Commit;
				observers::NotificationObserverAdapter entityObserver(
						m_pPluginManager->createObserver(),
						m_pPluginManager->createNotificationPublisher());
				auto observerContext = observers::ObserverContext(observerState, Height(1), notifyMode, resolverContext);
				entityObserver.notify(model::WeakEntityInfo(*transactionInfo.pEntity, transactionInfo.EntityHash), observerContext);
				m_cache.commit(Height(1));
			}

		private:
			test::TempDirectoryGuard m_dbDirGuard;
			std::shared_ptr<plugins::PluginManager> m_pPluginManager;
			config::LocalNodeConfiguration m_config;
			cache::MemoryUtCache m_transactionsCache;
			cache::CatapultCache m_cache;
			UnlockedAccounts m_unlockedAccounts;
			std::unique_ptr<Harvester> m_pHarvester;
		};

		// endregion
	}

	TEST(TEST_CLASS, HarvestIsThreadSafeWhenCacheIsChanging) {
		// Arrange:
		HarvesterTestContext context;
		auto pLastBlock = context.createLastBlock();
		auto nextBlockTimestamp = pLastBlock->Timestamp + Timestamp(10'000);

		// - seed a sender account and unconfirmed transactions
		context.prepareSenderAccountAndTransactions(test::GenerateKeyPair(), nextBlockTimestamp);

		// Act:
		// - simulate tx confirmation (block dispatcher) by confirming one tx at a time
		boost::thread_group threads;
		threads.create_thread([&context] {
			for (auto i = 0u; i < GetNumIterations(); ++i) {
				// 1. get next transaction info from UT cache
				model::TransactionInfo nextTransactionInfo;
				{
					auto utCacheView = context.transactionsCache().view();
					auto pTransaction = utCacheView.unknownTransactions(BlockFeeMultiplier(0), {})[0];
					auto transactionHash = model::CalculateHash(*pTransaction);
					nextTransactionInfo = model::TransactionInfo(std::move(pTransaction), transactionHash);
				}

				// 2. simulate application
				context.execute(nextTransactionInfo);
				test::Sleep(5);

				// 3. remove it from ut cache
				{
					auto utCacheModifier = context.transactionsCache().modifier();
					utCacheModifier.remove(nextTransactionInfo.EntityHash);
				}
			}
		});

		// - simulate harvester by harvesting blocks one tx at a time
		auto numHarvests = 0u;
		auto numHarvestAttempts = 0u;
		auto previousBlockElement = test::BlockToBlockElement(*pLastBlock);
		threads.create_thread([&context, &numHarvests, &numHarvestAttempts, &previousBlockElement] {
			auto harvestTimestamp = previousBlockElement.Block.Timestamp + Timestamp(10'000);
			for (;;) {
				auto pHarvestedBlock = context.harvester().harvest(previousBlockElement, harvestTimestamp);

				++numHarvestAttempts;
				if (pHarvestedBlock)
					++numHarvests;

				if (0 == context.transactionsCache().view().size())
					break;
			}
		});

		// - wait for all threads
		threads.join_all();

		// Assert: at least one block was harvested and all transactions were processed
		CATAPULT_LOG(debug) << numHarvests << "/" << numHarvestAttempts << " blocks harvested";

		auto cacheView = context.cache().createView();
		EXPECT_LT(numHarvests, numHarvestAttempts);
		EXPECT_EQ(GetNumIterations(), cacheView.sub<cache::HashCache>().size());
	}
}}
