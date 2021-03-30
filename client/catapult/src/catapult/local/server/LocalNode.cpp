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

#include "LocalNode.h"
#include "FileStateChangeStorage.h"
#include "MemoryCounters.h"
#include "NemesisBlockNotifier.h"
#include "NodeContainerSubscriberAdapter.h"
#include "NodeUtils.h"
#include "StaticNodeRefreshService.h"
#include "catapult/config/CatapultDataDirectory.h"
#include "catapult/extensions/CommitStepHandler.h"
#include "catapult/extensions/ConfigurationUtils.h"
#include "catapult/extensions/LocalNodeChainScore.h"
#include "catapult/extensions/LocalNodeStateFileStorage.h"
#include "catapult/extensions/LocalNodeStateRef.h"
#include "catapult/extensions/ProcessBootstrapper.h"
#include "catapult/extensions/ServiceLocator.h"
#include "catapult/extensions/ServiceState.h"
#include "catapult/io/BlockStorageCache.h"
#include "catapult/io/FileQueue.h"
#include "catapult/io/FilesystemUtils.h"
#include "catapult/ionet/NodeContainer.h"
#include "catapult/local/HostUtils.h"
#include "catapult/utils/StackLogger.h"

namespace catapult { namespace local {

	namespace {
		// region data directory preparation

		void MakeWriteable(const std::filesystem::path& filepath) {
			std::filesystem::permissions(filepath, std::filesystem::perms::owner_write, std::filesystem::perm_options::add);
		}

		void ImportRootFilesFromSeed(const std::string& seedDirectory, const config::CatapultDataDirectory& dataDirectory) {
			auto begin = std::filesystem::directory_iterator(seedDirectory);
			auto end = std::filesystem::directory_iterator();

			for (auto iter = begin; end != iter; ++iter) {
				if (!iter->is_regular_file())
					continue;

				auto destinationFilename = dataDirectory.rootDir().file(iter->path().filename().generic_string());
				std::filesystem::copy_file(iter->path(), destinationFilename);

				if (std::string::npos != destinationFilename.find(".dat"))
					MakeWriteable(destinationFilename);
			}
		}

		void ImportVersionedFilesFromSeed(
				const std::string& seedDirectory,
				const config::CatapultDataDirectory& dataDirectory,
				uint32_t fileDatabaseBatchSize) {
			auto outputDirectory = dataDirectory.dir("00000");
			outputDirectory.create();

			auto begin = std::filesystem::directory_iterator(seedDirectory + "/00000");
			auto end = std::filesystem::directory_iterator();

			for (auto iter = begin; end != iter; ++iter) {
				auto filename = iter->path().filename().generic_string();
				if (0 != filename.find("00001") || 1 == fileDatabaseBatchSize) {
					// copy plain file
					auto destinationFilename = outputDirectory.file(filename);
					std::filesystem::copy_file(iter->path(), destinationFilename);
					MakeWriteable(destinationFilename);
					continue;
				}

				// import file database
				auto outputFilename = "00000" + iter->path().extension().generic_string();
				io::RawFile inputFile(iter->path().generic_string(), io::OpenMode::Read_Only);
				io::RawFile outputFile(outputDirectory.file(outputFilename), io::OpenMode::Read_Write);

				// write file database header
				auto headerSize = fileDatabaseBatchSize * sizeof(uint64_t);
				outputFile.write(std::vector<uint8_t>(headerSize));
				outputFile.seek(sizeof(uint64_t));
				Write64(outputFile, headerSize);
				outputFile.seek(headerSize);

				// copy input file contents
				std::vector<uint8_t> inputBuffer(inputFile.size());
				inputFile.read(inputBuffer);
				outputFile.write(inputBuffer);
			}
		}

		config::CatapultDataDirectory PrepareDataDirectory(const config::CatapultConfiguration& config) {
			auto dataDirectory = config::CatapultDataDirectoryPreparer::Prepare(config.User.DataDirectory);

			if (!std::filesystem::exists(dataDirectory.rootDir().file("index.dat"))) {
				CATAPULT_LOG(info) << "importing seed directory";
				ImportRootFilesFromSeed(config.User.SeedDirectory, dataDirectory);
				ImportVersionedFilesFromSeed(config.User.SeedDirectory, dataDirectory, config.Node.FileDatabaseBatchSize);
			}

			return dataDirectory;
		}

		// endregion

		// region storage and subscriber factories

		std::unique_ptr<io::PrunableBlockStorage> CreateStagingBlockStorage(
				const config::CatapultDataDirectory& dataDirectory,
				uint32_t fileDatabaseBatchSize) {
			auto stagingDirectory = dataDirectory.spoolDir("block_sync").str();
			config::CatapultDirectory(stagingDirectory).create();
			return std::make_unique<io::FileBlockStorage>(stagingDirectory, fileDatabaseBatchSize, io::FileBlockStorageMode::None);
		}

		class CommitImportanceFilesStateChangeSubscriber : public subscribers::StateChangeSubscriber {
		public:
			explicit CommitImportanceFilesStateChangeSubscriber(const config::CatapultDataDirectory& dataDirectory)
					: m_dataDirectory(dataDirectory)
			{}

		public:
			void notifyScoreChange(const model::ChainScore&) override
			{}

			void notifyStateChange(const subscribers::StateChangeInfo&) override {
				// when state is committed, move importance files from wip to base
				auto destDirectory = m_dataDirectory.dir("importance");
				io::MoveAllFiles(destDirectory.dir("wip").str(), destDirectory.str());
			}

		private:
			config::CatapultDataDirectory m_dataDirectory;
		};

		std::unique_ptr<subscribers::StateChangeSubscriber> CreateStateChangeSubscriber(
				subscribers::SubscriptionManager& subscriptionManager,
				const cache::CatapultCache& catapultCache,
				const config::CatapultDataDirectory& dataDirectory) {
			subscriptionManager.addStateChangeSubscriber(CreateFileStateChangeStorage(
					std::make_unique<io::FileQueueWriter>(dataDirectory.spoolDir("state_change").str(), "index_server.dat"),
					[&catapultCache]() { return catapultCache.changesStorages(); }));
			subscriptionManager.addStateChangeSubscriber(std::make_unique<CommitImportanceFilesStateChangeSubscriber>(dataDirectory));
			return subscriptionManager.createStateChangeSubscriber();
		}

		std::unique_ptr<subscribers::NodeSubscriber> CreateNodeSubscriber(
				subscribers::SubscriptionManager& subscriptionManager,
				ionet::NodeContainer& nodes,
				const std::unordered_set<std::string>& localNetworks,
				const extensions::BannedNodeIdentitySink& bannedNodeIdentitySink) {
			subscriptionManager.addNodeSubscriber(CreateNodeContainerSubscriberAdapter(nodes, localNetworks, bannedNodeIdentitySink));
			return subscriptionManager.createNodeSubscriber();
		}

		// endregion

		// region utils

		void AddNodeCounters(std::vector<utils::DiagnosticCounter>& counters, const ionet::NodeContainer& nodes) {
			counters.emplace_back(utils::DiagnosticCounterId("NODES"), [&nodes]() {
				return nodes.view().size();
			});

			counters.emplace_back(utils::DiagnosticCounterId("BAN ACT"), [&nodes]() {
				return nodes.view().bannedNodesSize();
			});
			counters.emplace_back(utils::DiagnosticCounterId("BAN ALL"), [&nodes]() {
				return nodes.view().bannedNodesDeepSize();
			});
		}

		// endregion

		class DefaultLocalNode final : public LocalNode {
		public:
			DefaultLocalNode(std::unique_ptr<extensions::ProcessBootstrapper>&& pBootstrapper, const config::CatapultKeys& keys)
					: m_pBootstrapper(std::move(pBootstrapper))
					, m_serviceLocator(keys)
					, m_config(m_pBootstrapper->config())
					, m_dataDirectory(PrepareDataDirectory(m_config))
					, m_nodes(
							m_config.Node.MaxTrackedNodes,
							m_config.BlockChain.Network.NodeEqualityStrategy,
							GetBanSettings(m_config.Node.Banning),
							m_pBootstrapper->extensionManager().networkTimeSupplier(m_config.BlockChain.Network.EpochAdjustment),
							ionet::CreateRangeNodeVersionPredicate(
									m_config.Node.MinPartnerNodeVersion,
									m_config.Node.MaxPartnerNodeVersion))
					, m_catapultCache({}) // note that sub caches are added in boot
					, m_storage(
							m_pBootstrapper->subscriptionManager().createBlockStorage(m_pBlockChangeSubscriber),
							CreateStagingBlockStorage(m_dataDirectory, m_config.Node.FileDatabaseBatchSize))
					, m_pUtCache(m_pBootstrapper->subscriptionManager().createUtCache(extensions::GetUtCacheOptions(m_config.Node)))
					, m_pFinalizationSubscriber(m_pBootstrapper->subscriptionManager().createFinalizationSubscriber())
					, m_pNodeSubscriber(CreateNodeSubscriber(
							m_pBootstrapper->subscriptionManager(),
							m_nodes,
							m_config.Node.LocalNetworks,
							m_bannedNodeIdentitySink))
					, m_pStateChangeSubscriber(CreateStateChangeSubscriber(
							m_pBootstrapper->subscriptionManager(),
							m_catapultCache,
							m_dataDirectory))
					, m_pTransactionStatusSubscriber(m_pBootstrapper->subscriptionManager().createTransactionStatusSubscriber())
					, m_pluginManager(m_pBootstrapper->pluginManager())
					, m_isBooted(false) {
				ValidateNodes(m_pBootstrapper->staticNodes());
				AddLocalNode(m_nodes, m_pBootstrapper->config());
			}

			~DefaultLocalNode() override {
				shutdown();
			}

		public:
			void boot() {
				CATAPULT_LOG(info) << "registering system plugins";
				m_pluginModules = LoadAllPlugins(*m_pBootstrapper);

				CATAPULT_LOG(debug) << "initializing cache";
				m_catapultCache = m_pluginManager.createCache();

				CATAPULT_LOG(debug) << "registering counters";
				registerCounters();

				utils::StackLogger stackLogger("booting local node", utils::LogLevel::info);
				auto isFirstBoot = executeAndNotifyNemesis();
				loadStateFromDisk();

				CATAPULT_LOG(debug) << "booting extension services";
				auto& extensionManager = m_pBootstrapper->extensionManager();
				extensionManager.addServiceRegistrar(CreateStaticNodeRefreshServiceRegistrar(m_pBootstrapper->staticNodes()));
				auto serviceState = extensions::ServiceState(
						m_config,
						m_nodes,
						m_catapultCache,
						m_storage,
						m_score,
						*m_pUtCache,
						extensionManager.networkTimeSupplier(m_config.BlockChain.Network.EpochAdjustment),
						*m_pFinalizationSubscriber,
						*m_pNodeSubscriber,
						*m_pStateChangeSubscriber,
						*m_pTransactionStatusSubscriber,
						m_counters,
						m_pluginManager,
						m_pBootstrapper->pool());
				extensionManager.registerServices(m_serviceLocator, serviceState);
				for (const auto& counter : m_serviceLocator.counters())
					m_counters.push_back(counter);

				// notice that CreateNodeContainerSubscriberAdapter takes reference to m_bannedNodeIdentitySink,
				// otherwise this would not work
				m_bannedNodeIdentitySink = serviceState.hooks().bannedNodeIdentitySink();
				m_isBooted = true;

				// save nemesis state on first boot so that state directory is created and NemesisBlockNotifier
				// is always bypassed on subsequent boots
				if (isFirstBoot)
					saveStateToDisk();
			}

		private:
			void registerCounters() {
				AddMemoryCounters(m_counters);
				const auto& catapultCache = m_catapultCache;
				m_counters.emplace_back(utils::DiagnosticCounterId("TOT CONF TXES"), [&catapultCache]() {
					return catapultCache.createView().dependentState().NumTotalTransactions;
				});

				m_pluginManager.addDiagnosticCounters(m_counters, m_catapultCache); // add cache counters
				m_counters.emplace_back(utils::DiagnosticCounterId("UT CACHE"), [&source = *m_pUtCache]() {
					return source.view().size();
				});
				m_counters.emplace_back(utils::DiagnosticCounterId("UT CACHE MEM"), [&source = *m_pUtCache]() {
					return source.view().memorySize().megabytes();
				});

				AddNodeCounters(m_counters, m_nodes);
			}

			bool executeAndNotifyNemesis() {
				// only execute nemesis during first boot
				if (extensions::HasSerializedState(m_dataDirectory.dir("state")))
					return false;

				NemesisBlockNotifier notifier(m_config.BlockChain, m_catapultCache, m_storage, m_pluginManager);

				if (m_pBlockChangeSubscriber)
					notifier.raise(*m_pBlockChangeSubscriber);

				notifier.raise(*m_pFinalizationSubscriber);
				notifier.raise(*m_pStateChangeSubscriber);

				// indicate the nemesis block is fully updated so that it can be processed downstream immediately
				auto commitStep = extensions::CreateCommitStepHandler(m_dataDirectory);
				commitStep(consumers::CommitOperationStep::All_Updated);

				// skip next *two* messages because subscriber creates two files during raise (score change and state change)
				if (m_config.Node.EnableAutoSyncCleanup)
					io::FileQueueReader(m_dataDirectory.spoolDir("state_change").str(), "index_server_r.dat", "index_server.dat").skip(2);

				return true;
			}

			void loadStateFromDisk() {
				auto heights = extensions::LoadStateFromDirectory(m_dataDirectory.dir("state"), stateRef(), m_pluginManager);

				// if cache and storage heights are inconsistent, recovery is needed
				if (heights.Cache != heights.Storage) {
					std::ostringstream out;
					out << "cache height (" << heights.Cache << ") is inconsistent with storage height (" << heights.Storage << ")";
					CATAPULT_THROW_RUNTIME_ERROR(out.str().c_str());
				}

				CATAPULT_LOG(info) << "loaded block chain (height = " << heights.Cache << ", score = " << m_score.get() << ")";
			}

		public:
			void shutdown() override {
				utils::StackLogger stackLogger("shutting down local node", utils::LogLevel::info);

				m_pBootstrapper->pool().shutdown();
				saveStateToDisk();
			}

		private:
			void saveStateToDisk() {
				// only save to storage if boot succeeded
				if (!m_isBooted)
					return;

				constexpr auto SaveStateToDirectoryWithCheckpointing = extensions::SaveStateToDirectoryWithCheckpointing;
				SaveStateToDirectoryWithCheckpointing(m_dataDirectory, m_config.Node, m_catapultCache, m_score.get());
			}

		public:
			const cache::CatapultCache& cache() const override {
				return m_catapultCache;
			}

			model::ChainScore score() const override {
				return m_score.get();
			}

			LocalNodeCounterValues counters() const override {
				LocalNodeCounterValues values;
				values.reserve(m_counters.size());
				for (const auto& counter : m_counters)
					values.emplace_back(counter);

				return values;
			}

			ionet::NodeContainerView nodes() const override {
				return m_nodes.view();
			}

		private:
			extensions::LocalNodeStateRef stateRef() {
				return extensions::LocalNodeStateRef(m_config, m_catapultCache, m_storage, m_score);
			}

		private:
			// make sure rooted services and modules are unloaded last
			std::vector<plugins::PluginModule> m_pluginModules;
			std::unique_ptr<extensions::ProcessBootstrapper> m_pBootstrapper;
			extensions::ServiceLocator m_serviceLocator;

			io::BlockChangeSubscriber* m_pBlockChangeSubscriber;
			const config::CatapultConfiguration& m_config;
			config::CatapultDataDirectory m_dataDirectory;
			ionet::NodeContainer m_nodes;

			cache::CatapultCache m_catapultCache;
			io::BlockStorageCache m_storage;
			extensions::LocalNodeChainScore m_score;
			std::unique_ptr<cache::MemoryUtCacheProxy> m_pUtCache;

			std::unique_ptr<subscribers::FinalizationSubscriber> m_pFinalizationSubscriber;
			std::unique_ptr<subscribers::NodeSubscriber> m_pNodeSubscriber;
			std::unique_ptr<subscribers::StateChangeSubscriber> m_pStateChangeSubscriber;
			std::unique_ptr<subscribers::TransactionStatusSubscriber> m_pTransactionStatusSubscriber;

			plugins::PluginManager& m_pluginManager;
			std::vector<utils::DiagnosticCounter> m_counters;
			extensions::BannedNodeIdentitySink m_bannedNodeIdentitySink;
			bool m_isBooted;
		};
	}

	std::unique_ptr<LocalNode> CreateLocalNode(
			const config::CatapultKeys& keys,
			std::unique_ptr<extensions::ProcessBootstrapper>&& pBootstrapper) {
		return CreateAndBootHost<DefaultLocalNode>(std::move(pBootstrapper), keys);
	}
}}
