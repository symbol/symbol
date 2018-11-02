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
#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/plugins/PluginLoader.h"

namespace catapult { namespace tools { namespace nemgen {

	namespace {
		plugins::StorageConfiguration CreateStorageConfiguration(const config::LocalNodeConfiguration& config) {
			plugins::StorageConfiguration storageConfig;
			storageConfig.PreferCacheDatabase = config.Node.ShouldUseCacheDatabaseStorage;
			storageConfig.CacheDatabaseDirectory = (boost::filesystem::path(config.User.DataDirectory) / "statedb").generic_string();
			return storageConfig;
		}
	}

	PluginLoader::PluginLoader(const config::LocalNodeConfiguration& config)
			: m_config(config)
			, m_pluginManager(m_config.BlockChain, CreateStorageConfiguration(config))
	{}

	plugins::PluginManager& PluginLoader::manager() {
		return m_pluginManager;
	}

	void PluginLoader::loadAll() {
		// default plugins
		for (const auto& pluginName : { "catapult.coresystem", "catapult.plugins.signature" })
			loadPlugin(pluginName);

		// custom plugins
		for (const auto& pair : m_config.BlockChain.Plugins)
			loadPlugin(pair.first);
	}

	void PluginLoader::loadPlugin(const std::string& pluginName) {
		LoadPluginByName(m_pluginManager, m_pluginModules, m_config.User.PluginsDirectory, pluginName);
	}
}}}
