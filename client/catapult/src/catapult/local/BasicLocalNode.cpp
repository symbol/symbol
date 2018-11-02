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

#include "BasicLocalNode.h"
#include "ConfigurationUtils.h"
#include "MemoryCounters.h"
#include "NodeUtils.h"
#include "catapult/extensions/LocalNodeChainScore.h"
#include "catapult/extensions/LocalNodeStateRef.h"
#include "catapult/extensions/ServiceLocator.h"
#include "catapult/extensions/ServiceState.h"
#include "catapult/io/BlockStorageCache.h"
#include "catapult/ionet/NodeContainer.h"
#include "catapult/plugins/PluginLoader.h"
#include "catapult/utils/ExceptionLogging.h"
#include "catapult/utils/StackLogger.h"

namespace catapult { namespace local {

	namespace {
		std::unique_ptr<subscribers::NodeSubscriber> CreateNodeSubscriber(
				subscribers::SubscriptionManager& subscriptionManager,
				ionet::NodeContainer& nodes) {
			subscriptionManager.addNodeSubscriber(CreateNodeContainerSubscriberAdapter(nodes));
			return subscriptionManager.createNodeSubscriber();
		}

		class BasicLocalNode final : public BootedLocalNode {
		public:
			BasicLocalNode(std::unique_ptr<extensions::LocalNodeBootstrapper>&& pBootstrapper, const crypto::KeyPair& keyPair)
					: m_pBootstrapper(std::move(pBootstrapper))
					, m_serviceLocator(keyPair)
					, m_pBlockChainStorage(m_pBootstrapper->extensionManager().createBlockChainStorage())
					, m_config(m_pBootstrapper->config())
					, m_nodes(m_config.Node.MaxTrackedNodes)
					, m_catapultCache({}) // note that subcaches are added in boot
					, m_storage(m_pBootstrapper->subscriptionManager().createBlockStorage())
					, m_pUtCache(m_pBootstrapper->subscriptionManager().createUtCache(GetUtCacheOptions(m_config.Node)))
					, m_pTransactionStatusSubscriber(m_pBootstrapper->subscriptionManager().createTransactionStatusSubscriber())
					, m_pStateChangeSubscriber(m_pBootstrapper->subscriptionManager().createStateChangeSubscriber())
					, m_pNodeSubscriber(CreateNodeSubscriber(m_pBootstrapper->subscriptionManager(), m_nodes))
					, m_pluginManager(m_pBootstrapper->pluginManager())
					, m_isBooted(false) {
				SeedNodeContainer(m_nodes, *m_pBootstrapper);
			}

			~BasicLocalNode() override {
				shutdown();
			}

		public:
			void boot() {
				auto& extensionManager = m_pBootstrapper->extensionManager();

				CATAPULT_LOG(info) << "registering system plugins";
				loadPlugins();

				CATAPULT_LOG(debug) << "initializing cache";
				m_catapultCache = m_pluginManager.createCache();

				CATAPULT_LOG(debug) << "registering counters";
				registerCounters();

				utils::StackLogger stackLogger("booting local node", utils::LogLevel::Info);
				extensionManager.preLoadHandler()(m_catapultCache);
				m_pBlockChainStorage->loadFromStorage(stateRef(), m_pluginManager);

				CATAPULT_LOG(debug) << "booting extension services";
				auto serviceState = extensions::ServiceState(
						m_config,
						m_nodes,
						m_catapultCache,
						m_catapultState,
						m_storage,
						m_score,
						*m_pUtCache,
						extensionManager.networkTimeSupplier(),
						*m_pTransactionStatusSubscriber,
						*m_pStateChangeSubscriber,
						*m_pNodeSubscriber,
						m_counters,
						m_pluginManager,
						m_pBootstrapper->pool());
				extensionManager.registerServices(m_serviceLocator, serviceState);
				for (const auto& counter : m_serviceLocator.counters())
					m_counters.push_back(counter);

				m_isBooted = true;
			}

		private:
			void loadPlugins() {
				for (const auto& pluginName : m_pBootstrapper->extensionManager().systemPluginNames())
					loadPlugin(pluginName);

				for (const auto& pair : m_config.BlockChain.Plugins)
					loadPlugin(pair.first);
			}

			void loadPlugin(const std::string& pluginName) {
				LoadPluginByName(m_pluginManager, m_pluginModules, m_config.User.PluginsDirectory, pluginName);
			}

			void registerCounters() {
				AddMemoryCounters(m_counters);
				m_pluginManager.addDiagnosticCounters(m_counters, m_catapultCache); // add cache counters
				m_counters.emplace_back(utils::DiagnosticCounterId("UT CACHE"), [&source = *m_pUtCache]() {
					return source.view().size();
				});
			}

		public:
			void shutdown() override {
				utils::StackLogger stackLogger("shutting down local node", utils::LogLevel::Info);

				m_pBootstrapper->pool().shutdown();

				// only save to storage if boot succeeded
				if (m_isBooted)
					m_pBlockChainStorage->saveToStorage(stateCref());
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
			extensions::LocalNodeStateConstRef stateCref() const {
				return extensions::LocalNodeStateConstRef(m_config, m_catapultState, m_catapultCache, m_storage, m_score);
			}

			extensions::LocalNodeStateRef stateRef() {
				return extensions::LocalNodeStateRef(m_config, m_catapultState, m_catapultCache, m_storage, m_score);
			}

		private:
			// make sure rooted services and modules are unloaded last
			std::vector<plugins::PluginModule> m_pluginModules;
			std::unique_ptr<extensions::LocalNodeBootstrapper> m_pBootstrapper;
			extensions::ServiceLocator m_serviceLocator;

			std::unique_ptr<extensions::BlockChainStorage> m_pBlockChainStorage;
			const config::LocalNodeConfiguration& m_config;
			ionet::NodeContainer m_nodes;

			cache::CatapultCache m_catapultCache;
			state::CatapultState m_catapultState;
			io::BlockStorageCache m_storage;
			extensions::LocalNodeChainScore m_score;
			std::unique_ptr<cache::MemoryUtCacheProxy> m_pUtCache;

			std::unique_ptr<subscribers::TransactionStatusSubscriber> m_pTransactionStatusSubscriber;
			std::unique_ptr<subscribers::StateChangeSubscriber> m_pStateChangeSubscriber;
			std::unique_ptr<subscribers::NodeSubscriber> m_pNodeSubscriber;

			plugins::PluginManager& m_pluginManager;
			std::vector<utils::DiagnosticCounter> m_counters;
			bool m_isBooted;
		};
	}

	std::unique_ptr<BootedLocalNode> CreateBasicLocalNode(
			const crypto::KeyPair& keyPair,
			std::unique_ptr<extensions::LocalNodeBootstrapper>&& pBootstrapper) {
		// create and boot the node
		auto pLocalNode = std::make_unique<BasicLocalNode>(std::move(pBootstrapper), keyPair);
		try {
			pLocalNode->boot();
		} catch (const std::exception& ex) {
			// log the exception and rethrow as a new exception in case the exception source is a dynamic plugin
			// (this avoids a crash in the calling code, which would occur because the local node destructor unloads the source plugin)
			CATAPULT_LOG(fatal) << UNHANDLED_EXCEPTION_MESSAGE("boot");
			CATAPULT_THROW_RUNTIME_ERROR(ex.what());
		}

		return std::move(pLocalNode);
	}
}}
