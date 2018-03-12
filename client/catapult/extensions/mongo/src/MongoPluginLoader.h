#pragma once
#include "catapult/plugins/PluginModule.h"
#include <vector>

namespace catapult { namespace mongo { class MongoPluginManager; } }

namespace catapult { namespace mongo {

	/// Container of plugin modules.
	using PluginModules = std::vector<plugins::PluginModule>;

	/// Loads a plugin named \a name with \a manager from \a directory into \a modules.
	void LoadPluginByName(MongoPluginManager& manager, PluginModules& modules, const std::string& directory, const std::string& name);
}}
