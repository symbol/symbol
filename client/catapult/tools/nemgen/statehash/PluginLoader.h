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

#pragma once
#include "catapult/plugins/PluginManager.h"
#include "catapult/plugins/PluginModule.h"

namespace catapult { namespace config { class LocalNodeConfiguration; } }

namespace catapult { namespace tools { namespace nemgen {

	/// Loads plugins into a plugin manager.
	class PluginLoader {
	public:
		/// Creates a loader around \a config.
		explicit PluginLoader(const config::LocalNodeConfiguration& config);

	public:
		/// Gets the plugin manager.
		plugins::PluginManager& manager();

	public:
		/// Loads all configured plugins.
		void loadAll();

	private:
		void loadPlugin(const std::string& pluginName);

	private:
		const config::LocalNodeConfiguration& m_config;
		std::vector<plugins::PluginModule> m_pluginModules;
		plugins::PluginManager m_pluginManager;
	};
}}}
