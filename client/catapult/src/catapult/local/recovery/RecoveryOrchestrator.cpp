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

#include "RecoveryOrchestrator.h"
#include "CatapultSystemState.h"
#include "MultiBlockLoader.h"
#include "RecoveryStorageAdapter.h"
#include "RepairSpooling.h"
#include "RepairState.h"
#include "StateChangeRepairingSubscriber.h"
#include "StorageStart.h"
#include "catapult/cache/ReadOnlyCatapultCache.h"
#include "catapult/cache_core/BlockStatisticCache.h"
#include "catapult/chain/BlockExecutor.h"
#include "catapult/config/CatapultDataDirectory.h"
#include "catapult/extensions/LocalNodeChainScore.h"
#include "catapult/extensions/LocalNodeStateFileStorage.h"
#include "catapult/extensions/LocalNodeStateRef.h"
#include "catapult/extensions/ProcessBootstrapper.h"
#include "catapult/io/BlockStorageCache.h"
#include "catapult/io/FilesystemUtils.h"
#include "catapult/io/MoveBlockFiles.h"
#include "catapult/local/HostUtils.h"
#include "catapult/observers/NotificationObserverAdapter.h"
#include "catapult/subscribers/BlockChangeReader.h"
#include "catapult/subscribers/BrokerMessageReaders.h"
#include "catapult/subscribers/TransactionStatusReader.h"
#include "catapult/utils/StackLogger.h"

namespace catapult { namespace local {

	namespace {
		// region DualStateChangeSubscriber

		class DualStateChangeSubscriber final : public subscribers::StateChangeSubscriber {
		public:
			DualStateChangeSubscriber(subscribers::StateChangeSubscriber& subscriber1, subscribers::StateChangeSubscriber& subscriber2)
					: m_subscriber1(subscriber1)
					, m_subscriber2(subscriber2)
			{}

		public:
			void notifyScoreChange(const model::ChainScore& chainScore) override {
				m_subscriber1.notifyScoreChange(chainScore);
				m_subscriber2.notifyScoreChange(chainScore);
			}

			void notifyStateChange(const subscribers::StateChangeInfo& stateChangeInfo) override {
				m_subscriber1.notifyStateChange(stateChangeInfo);
				m_subscriber2.notifyStateChange(stateChangeInfo);
			}

		private:
			subscribers::StateChangeSubscriber& m_subscriber1;
			subscribers::StateChangeSubscriber& m_subscriber2;
		};

		// endregion

		std::unique_ptr<io::PrunableBlockStorage> CreateStagingBlockStorage(const config::CatapultDataDirectory& dataDirectory) {
			auto stagingDirectory = dataDirectory.spoolDir("block_recover").str();
			boost::filesystem::create_directory(stagingDirectory);
			return std::make_unique<io::FileBlockStorage>(stagingDirectory, io::FileBlockStorageMode::None);
		}

		void MoveSupplementalDataFiles(const config::CatapultDataDirectory& dataDirectory) {
			if (!boost::filesystem::exists(dataDirectory.dir("state.tmp").path()))
				return;

			extensions::LocalNodeStateSerializer serializer(dataDirectory.dir("state.tmp"));
			serializer.moveTo(dataDirectory.dir("state"));
		}

		Height MoveBlockFiles(const config::CatapultDirectory& stagingDirectory, io::BlockStorage& destinationStorage) {
			io::FileBlockStorage staging(stagingDirectory.str());
			if (Height(0) == staging.chainHeight())
				return Height(0);

			// mind that startHeight will be > 1, so there is no additional check
			auto startHeight = FindStartHeight(staging);
			CATAPULT_LOG(debug) << "moving blocks: " << startHeight << "-" << staging.chainHeight();
			io::MoveBlockFiles(staging, destinationStorage, startHeight);
			return startHeight;
		}

		class DefaultRecoveryOrchestrator final : public RecoveryOrchestrator {
		public:
			explicit DefaultRecoveryOrchestrator(std::unique_ptr<extensions::ProcessBootstrapper>&& pBootstrapper)
					: m_pBootstrapper(std::move(pBootstrapper))
					, m_config(m_pBootstrapper->config())
					, m_dataDirectory(config::CatapultDataDirectoryPreparer::Prepare(m_config.User.DataDirectory))
					, m_catapultCache({}) // note that sub caches are added in boot
					, m_pBlockStorage(m_pBootstrapper->subscriptionManager().createBlockStorage(m_pBlockChangeSubscriber))
					, m_storage(CreateReadOnlyStorageAdapter(*m_pBlockStorage), CreateStagingBlockStorage(m_dataDirectory))
					, m_pTransactionStatusSubscriber(m_pBootstrapper->subscriptionManager().createTransactionStatusSubscriber())
					, m_pStateChangeSubscriber(m_pBootstrapper->subscriptionManager().createStateChangeSubscriber())
					, m_pluginManager(m_pBootstrapper->pluginManager())
					, m_stateSavingRequired(true)
			{}

			~DefaultRecoveryOrchestrator() override {
				shutdown();
			}

		public:
			void boot() {
				CATAPULT_LOG(info) << "registering system plugins";
				m_pluginModules = LoadAllPlugins(*m_pBootstrapper);

				CATAPULT_LOG(debug) << "initializing cache";
				m_catapultCache = m_pluginManager.createCache();

				utils::StackLogger stackLogger("running recovery operations", utils::LogLevel::info);
				recover();
			}

		private:
			void recover() {
				auto systemState = CatapultSystemState(m_dataDirectory);

				CATAPULT_LOG(info) << "repairing spooling, commit step " << utils::to_underlying_type(systemState.commitStep());
				RepairSpooling(m_dataDirectory, systemState.commitStep());

				CATAPULT_LOG(info) << "repairing messages";
				repairSubscribers();

				CATAPULT_LOG(info) << "loading state";
				auto heights = extensions::LoadStateFromDirectory(m_dataDirectory.dir("state"), stateRef(), m_pluginManager);
				if (heights.Cache > heights.Storage)
					CATAPULT_THROW_RUNTIME_ERROR_2("cache height is larger than storage height", heights.Cache, heights.Storage);

				if (!stateRef().Config.Node.EnableCacheDatabaseStorage)
					repairStateFromStorage(heights);

				CATAPULT_LOG(info) << "loaded block chain (height = " << heights.Storage << ", score = " << m_score.get() << ")";

				CATAPULT_LOG(info) << "repairing state";
				repairState(systemState.commitStep());

				CATAPULT_LOG(info) << "finalizing";
				systemState.reset();
			}

			void repairSubscribers() {
				// due to behavior of SubscriptionManager, block change subscriber is only subscriber that can be nullptr
				if (m_pBlockChangeSubscriber)
					processMessages("block_change", *m_pBlockChangeSubscriber, subscribers::ReadNextBlockChange);

				processMessages("transaction_status", *m_pTransactionStatusSubscriber, subscribers::ReadNextTransactionStatus);
			}

			template<typename TSubscriber, typename TMessageReader>
			void processMessages(const std::string& queueName, TSubscriber& subscriber, TMessageReader readNextMessage) {
				subscribers::ReadAll(
						{ m_dataDirectory.spoolDir(queueName).str(), "index_broker_r.dat", "index.dat" },
						subscriber,
						readNextMessage);
			}

			void repairStateFromStorage(const extensions::StateHeights& heights) {
				if (heights.Cache == heights.Storage)
					return;

				// disable load optimizations (loading from the saved state is optimization enough) in order to prevent
				// discontinuities in block analysis (e.g. statistic cache expects consecutive blocks)
				CATAPULT_LOG(info) << "loading state - block loading required";
				auto observerFactory = [&pluginManager = m_pluginManager](const auto&) { return pluginManager.createObserver(); };
				auto partialScore = LoadBlockChain(observerFactory, m_pluginManager, stateRef(), heights.Cache + Height(1));
				m_score += partialScore;
			}

			void repairState(consumers::CommitOperationStep commitStep) {
				// RepairState always needs to be called in order to recover broker messages
				std::unique_ptr<subscribers::StateChangeSubscriber> pStateChangeRepairSubscriber;
				std::unique_ptr<subscribers::StateChangeSubscriber> pDualStateChangeSubscriber;
				if (stateRef().Config.Node.EnableCacheDatabaseStorage) {
					pStateChangeRepairSubscriber = CreateStateChangeRepairingSubscriber(stateRef().Cache, stateRef().Score);
					pDualStateChangeSubscriber = std::make_unique<DualStateChangeSubscriber>(
							*pStateChangeRepairSubscriber,
							*m_pStateChangeSubscriber);
				}

				auto& repairSubscriber = stateRef().Config.Node.EnableCacheDatabaseStorage
						? *pDualStateChangeSubscriber
						: *m_pStateChangeSubscriber;
				RepairState(m_dataDirectory.spoolDir("state_change"), stateRef().Cache, *m_pStateChangeSubscriber, repairSubscriber);

				if (consumers::CommitOperationStep::State_Written != commitStep) {
					CATAPULT_LOG(debug) << " - purging state.tmp";
					io::PurgeDirectory(m_dataDirectory.dir("state.tmp").str());
					return;
				}

				CATAPULT_LOG(debug) << " - moving supplemental data and block files";
				MoveSupplementalDataFiles(m_dataDirectory);
				auto startHeight = MoveBlockFiles(m_dataDirectory.spoolDir("block_sync"), *m_pBlockStorage);

				// when verifiable state is enabled, forcibly regenerate all patricia trees because cache changes are coalesced
				if (stateRef().Config.BlockChain.EnableVerifiableState && startHeight > Height(0)) {
					CATAPULT_LOG(debug) << "- reloading supplemental state";
					extensions::LoadDependentStateFromDirectory(m_dataDirectory.dir("state"), stateRef().Cache);
					reapplyBlocks(startHeight);
				}

				// when cache database storage is enabled and State_Written,
				// cache database and supplemental data are updated by repairState
				m_stateSavingRequired = !stateRef().Config.Node.EnableCacheDatabaseStorage;
			}

			void reapplyBlocks(Height startHeight) {
				auto chainHeight = m_pBlockStorage->chainHeight();
				CATAPULT_LOG(info) << "reapplying blocks to regenerate patricia trees " << startHeight << "-" << chainHeight;

				auto cacheDelta = stateRef().Cache.createDelta();
				auto observerState = observers::ObserverState(cacheDelta);

				auto readOnlyCache = cacheDelta.toReadOnly();
				auto resolverContext = m_pluginManager.createResolverContext(readOnlyCache);

				observers::NotificationObserverAdapter observer(
						m_pluginManager.createObserver(),
						m_pluginManager.createNotificationPublisher());
				chain::BlockExecutionContext executionContext{ observer, resolverContext, observerState };

				// reapplyBlocks is only called when EnableVerifiableState is true.
				// non rocksdb backed caches (e.g. BlockStatisticCache) are loaded initially from "state" directory
				// but are not updated / reloaded after copying "state.tmp" to "state".
				// of these, only BlockStatisticCache is problematic because it has strict bounds checking.
				// as a workaround, fill it with statistics up to the new chain height.
				FillBlockStatisticCache(cacheDelta.sub<cache::BlockStatisticCache>(), chainHeight);

				CATAPULT_LOG(debug) << " - rolling back blocks";
				for (auto height = chainHeight; height >= startHeight; height = height - Height(1))
					chain::RollbackBlock(*m_pBlockStorage->loadBlockElement(height), executionContext);

				CATAPULT_LOG(debug) << " - executing blocks";
				for (auto height = startHeight; height <= chainHeight; height = height + Height(1)) {
					chain::ExecuteBlock(*m_pBlockStorage->loadBlockElement(height), executionContext);
					cacheDelta.calculateStateHash(height); // force recalculation of patricia tree
				}

				stateRef().Cache.commit(chainHeight);
			}

		private:
			static void FillBlockStatisticCache(cache::BlockStatisticCacheDelta& blockStatisticCacheDelta, Height newHeight) {
				auto originalHeight = newHeight;
				while (!blockStatisticCacheDelta.contains(state::BlockStatistic(originalHeight)))
					originalHeight = originalHeight - Height(1);

				// these statistics are placeholders and don't need to reflect the block exactly as long as the height is correct
				for (auto height = originalHeight + Height(1); height <= newHeight; height = height + Height(1))
					blockStatisticCacheDelta.insert(state::BlockStatistic(height));
			}

		public:
			void shutdown() override {
				utils::StackLogger stackLogger("shutting down recovery orchestrator", utils::LogLevel::info);

				m_pBootstrapper->pool().shutdown();
				saveStateToDisk();
			}

		private:
			void saveStateToDisk() {
				if (!m_stateSavingRequired) {
					// just write the commit step file that was deleted during recover
					io::IndexFile commitStepFile(m_dataDirectory.rootDir().file("commit_step.dat"));
					commitStepFile.set(utils::to_underlying_type(consumers::CommitOperationStep::All_Updated));
					return;
				}

				constexpr auto SaveStateToDirectoryWithCheckpointing = extensions::SaveStateToDirectoryWithCheckpointing;
				SaveStateToDirectoryWithCheckpointing(m_dataDirectory, m_config.Node, m_catapultCache, m_score.get());
			}

		public:
			model::ChainScore score() const override {
				return m_score.get();
			}

		private:
			extensions::LocalNodeStateRef stateRef() {
				return extensions::LocalNodeStateRef(m_config, m_catapultCache, m_storage, m_score);
			}

		private:
			// make sure modules are unloaded last
			std::vector<plugins::PluginModule> m_pluginModules;
			std::unique_ptr<extensions::ProcessBootstrapper> m_pBootstrapper;

			io::BlockChangeSubscriber* m_pBlockChangeSubscriber;
			const config::CatapultConfiguration& m_config;
			config::CatapultDataDirectory m_dataDirectory;

			cache::CatapultCache m_catapultCache;
			std::unique_ptr<io::BlockStorage> m_pBlockStorage;
			io::BlockStorageCache m_storage;
			extensions::LocalNodeChainScore m_score;

			std::unique_ptr<subscribers::TransactionStatusSubscriber> m_pTransactionStatusSubscriber;
			std::unique_ptr<subscribers::StateChangeSubscriber> m_pStateChangeSubscriber;

			plugins::PluginManager& m_pluginManager;
			bool m_stateSavingRequired;
		};
	}

	std::unique_ptr<RecoveryOrchestrator> CreateRecoveryOrchestrator(std::unique_ptr<extensions::ProcessBootstrapper>&& pBootstrapper) {
		return CreateAndBootHost<DefaultRecoveryOrchestrator>(std::move(pBootstrapper));
	}
}}
