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

#include "PluginLoader.h"
#include "catapult/config/CatapultConfiguration.h"
#include "catapult/plugins/PluginLoader.h"
#include "catapult/plugins/PluginManager.h"
#include "catapult/plugins/PluginModule.h"
#include <boost/filesystem.hpp>

namespace catapult { namespace tools { namespace nemgen {

	namespace {
		// region TempDirectoryGuard

		class TempDirectoryGuard {
		public:
			explicit TempDirectoryGuard(const std::string& directoryPath) : m_directoryPath(directoryPath)
			{}

			~TempDirectoryGuard() {
				auto numRemovedFiles = boost::filesystem::remove_all(m_directoryPath);
				CATAPULT_LOG(info)
						<< "deleted directory " << m_directoryPath << " and removed " << numRemovedFiles
						<< " files (exists? " << boost::filesystem::exists(m_directoryPath) << ")";
			}

		private:
			std::string m_directoryPath;
		};

		// endregion
	}

	// region PluginLoader::Impl

	class PluginLoader::Impl {
	public:
		Impl(const config::CatapultConfiguration& config, CacheDatabaseCleanupMode databaseCleanupMode)
				: m_config(config)
				, m_pluginManager(m_config.BlockChain, CreateStorageConfiguration(m_config), m_config.User, m_config.Inflation) {
			// in purge mode, clean up the data directory after each execution
			// (cache database is hardcoded to "statedb" so entire data directory must be temporary)
			if (CacheDatabaseCleanupMode::Purge != databaseCleanupMode)
				return;

			if (boost::filesystem::exists(m_config.User.DataDirectory))
				CATAPULT_THROW_INVALID_ARGUMENT_1("temporary data directory must not exist", m_config.User.DataDirectory);

			auto temporaryDirectory = (boost::filesystem::path(m_config.User.DataDirectory)).generic_string();
			m_pCacheDatabaseGuard = std::make_unique<TempDirectoryGuard>(temporaryDirectory);
		}

	public:
		plugins::PluginManager& manager() {
			return m_pluginManager;
		}

	public:
		void loadAll() {
			// default plugins
			for (const auto& pluginName : { "catapult.plugins.coresystem", "catapult.plugins.signature" })
				loadPlugin(pluginName);

			// custom plugins
			for (const auto& pair : m_config.BlockChain.Plugins)
				loadPlugin(pair.first);
		}

	private:
		void loadPlugin(const std::string& pluginName) {
			LoadPluginByName(m_pluginManager, m_pluginModules, m_config.User.PluginsDirectory, pluginName);
		}

	private:
		static plugins::StorageConfiguration CreateStorageConfiguration(const config::CatapultConfiguration& config) {
			plugins::StorageConfiguration storageConfig;
			storageConfig.PreferCacheDatabase = config.Node.EnableCacheDatabaseStorage;
			storageConfig.CacheDatabaseDirectory = (boost::filesystem::path(config.User.DataDirectory) / "statedb").generic_string();
			return storageConfig;
		}

	private:
		const config::CatapultConfiguration& m_config;
		std::vector<plugins::PluginModule> m_pluginModules;
		plugins::PluginManager m_pluginManager;

		std::unique_ptr<TempDirectoryGuard> m_pCacheDatabaseGuard;
	};

	// endregion

	// region PluginLoader

	PluginLoader::PluginLoader(const config::CatapultConfiguration& config, CacheDatabaseCleanupMode databaseCleanupMode)
			: m_pImpl(std::make_unique<Impl>(config, databaseCleanupMode))
	{}

	PluginLoader::~PluginLoader() = default;

	plugins::PluginManager& PluginLoader::manager() {
		return m_pImpl->manager();
	}

	const model::TransactionRegistry& PluginLoader::transactionRegistry() const {
		return m_pImpl->manager().transactionRegistry();
	}

	std::unique_ptr<const model::NotificationPublisher> PluginLoader::createNotificationPublisher() const {
		return m_pImpl->manager().createNotificationPublisher();
	}

	void PluginLoader::loadAll() {
		m_pImpl->loadAll();
	}

	// endregion
}}}
