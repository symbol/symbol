#pragma once
#include "PluginModule.h"
#include <vector>

namespace catapult { namespace plugins { class PluginManager; } }

namespace catapult { namespace plugins {

	/// Container of plugin modules.
	using PluginModules = std::vector<PluginModule>;

	/// Loads a plugin named \a name with \a manager from \a directory into \a modules.
	void LoadPluginByName(PluginManager& manager, PluginModules& modules, const std::string& directory, const std::string& name);
}}
