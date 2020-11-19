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

#include "HostUtils.h"
#include "catapult/extensions/ProcessBootstrapper.h"
#include "catapult/plugins/PluginLoader.h"

namespace catapult { namespace local {

	namespace {
		class BootstrapperPluginLoader {
		public:
			explicit BootstrapperPluginLoader(extensions::ProcessBootstrapper& bootstrapper)
					: m_config(bootstrapper.config())
					, m_extensionManager(bootstrapper.extensionManager())
					, m_pluginManager(bootstrapper.pluginManager())
			{}

		public:
			const std::vector<plugins::PluginModule>& modules() {
				return m_pluginModules;
			}

		public:
			void loadAll() {
				for (const auto& pluginName : m_extensionManager.systemPluginNames())
					loadOne(pluginName);

				for (const auto& pair : m_config.BlockChain.Plugins)
					loadOne(pair.first);
			}

		private:
			void loadOne(const std::string& pluginName) {
				LoadPluginByName(m_pluginManager, m_pluginModules, m_config.User.PluginsDirectory, pluginName);
			}

		private:
			const config::CatapultConfiguration& m_config;
			const extensions::ExtensionManager& m_extensionManager;
			plugins::PluginManager& m_pluginManager;

			std::vector<plugins::PluginModule> m_pluginModules;
		};
	}

	std::vector<plugins::PluginModule> LoadAllPlugins(extensions::ProcessBootstrapper& bootstrapper) {
		BootstrapperPluginLoader loader(bootstrapper);
		loader.loadAll();
		return loader.modules();
	}
}}
