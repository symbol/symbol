#pragma once
#include "catapult/plugins.h"

namespace catapult { namespace plugins { class PluginManager; } }

namespace catapult { namespace plugins {

	/// Registers the in-memory hash cache with \a manager.
	PLUGIN_API
	void RegisterMemoryHashCacheSystem(PluginManager& manager);
}}
