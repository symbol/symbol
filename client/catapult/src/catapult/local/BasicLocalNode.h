#pragma once
#include "BootedLocalNode.h"
#include "LocalNodeChainScore.h"
#include "MemoryCounters.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/io/BlockStorageCache.h"
#include "catapult/model/TransactionPlugin.h"
#include "catapult/plugins/PluginLoader.h"
#include "catapult/plugins/PluginManager.h"
#include "catapult/state/CatapultState.h"
#include "catapult/thread/MultiServicePool.h"
#include "catapult/utils/DiagnosticCounter.h"
#include "catapult/utils/StackLogger.h"

namespace catapult { namespace local {

	/// A basic local node.
	class BasicLocalNode : public BootedLocalNode {
	protected:
		/// Creates a basic local node around \a config, \a pMultiServicePool and \a pBlockStorage.
		BasicLocalNode(
				config::LocalNodeConfiguration&& config,
				std::unique_ptr<thread::MultiServicePool>&& pMultiServicePool,
				std::unique_ptr<io::BlockStorage>&& pBlockStorage)
				: m_config(std::move(config))
				, m_pMultiServicePool(std::move(pMultiServicePool))
				, m_catapultCache({}) // note that subcaches are added in boot
				, m_storage(std::move(pBlockStorage))
				, m_pluginManager(m_config.BlockChain)
				, m_isBooted(false)
		{}

	public:
		/// Boots the node.
		void boot() {
			CATAPULT_LOG(info) << "registering system plugins";
			for (const auto& pluginName : { "catapult.plugins.coresystem", "catapult.plugins.blockdifficultycache" })
				loadPlugin(pluginName);

			for (const auto& pair : m_config.BlockChain.Plugins)
				loadPlugin(pair.first);

			registerPlugins([this](const auto& pluginName) { this->loadPlugin(pluginName); });

			// build the cache
			m_catapultCache = m_pluginManager.createCache();

			utils::StackLogger stackLogger("Booting local node", utils::LogLevel::Info);

			loadFromStorage();

			CATAPULT_LOG(debug) << "registering counters";
			m_pluginManager.addDiagnosticCounterHook([](auto& counters, const auto&) { AddMemoryCounters(counters); });
			registerCounters([this](const auto& counterName, const auto& supplier) { this->addCounter(counterName, supplier); });
			m_pluginManager.addDiagnosticCounters(m_counters, m_catapultCache);

			CATAPULT_LOG(debug) << "booting services";
			startServices(*m_pMultiServicePool);

			m_isBooted = true;
		}

	public:
		void shutdown() override {
			utils::StackLogger stackLogger("Shutting down local node", utils::LogLevel::Info);

			m_pMultiServicePool->shutdown();

			// only save to storage if boot succeeded
			if (m_isBooted)
				saveToStorage();
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

	protected:
		using RegisterPluginFunc = std::function<void (const std::string&)>;
		using RegisterCounterFunc = std::function<void (const std::string&, const std::function<uint64_t ()>&)>;

		/// Gets a reference to the node's plugin manager.
		const plugins::PluginManager& pluginManager() const {
			return m_pluginManager;
		}

		/// Gets a reference to the node's state.
		LocalNodeStateRef stateRef() {
			return LocalNodeStateRef(m_config, m_state, m_catapultCache, m_storage, m_score);
		}

		/// Gets a const reference to the node's state.
		LocalNodeStateConstRef stateCref() const {
			return LocalNodeStateConstRef(m_config, m_state, m_catapultCache, m_storage, m_score);
		}

	private:
		void loadPlugin(const std::string& pluginName) {
			LoadPluginByName(m_pluginManager, m_pluginModules, m_config.User.PluginsDirectory, pluginName);
		}

		void addCounter(const std::string& counterName, const std::function<uint64_t ()>& supplier) {
			auto counter = utils::DiagnosticCounter(utils::DiagnosticCounterId(counterName), supplier);
			m_pluginManager.addDiagnosticCounterHook([counter](auto& counters, const auto&) {
				counters.push_back(counter);
			});
		}

	private:
		/// Registers custom plugins via \a registerPlugin.
		virtual void registerPlugins(const RegisterPluginFunc& registerPlugin) = 0;

		/// Loads data from storage.
		virtual void loadFromStorage() = 0;

		/// Registers custom counters via \a registerCounter.
		virtual void registerCounters(const RegisterCounterFunc& registerCounter) = 0;

		/// Starts all services with \a pool parallelization.
		virtual void startServices(thread::MultiServicePool& pool) = 0;

		/// Saves all in memory state to storage.
		virtual void saveToStorage() = 0;

	private:
		// make sure modules are unloaded last
		std::vector<plugins::PluginModule> m_pluginModules;

		config::LocalNodeConfiguration m_config;
		std::unique_ptr<thread::MultiServicePool> m_pMultiServicePool;

		state::CatapultState m_state;
		cache::CatapultCache m_catapultCache;
		io::BlockStorageCache m_storage;
		LocalNodeChainScore m_score;

		plugins::PluginManager m_pluginManager;
		std::vector<utils::DiagnosticCounter> m_counters;
		bool m_isBooted;
	};
}}
