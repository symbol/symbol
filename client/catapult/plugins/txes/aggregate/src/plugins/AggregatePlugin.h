#pragma once
#include "catapult/plugins.h"

namespace catapult { namespace plugins { class PluginManager; } }

namespace catapult { namespace plugins {

	/// Registers aggregate support with \a manager.
	PLUGIN_API
	void RegisterAggregateSubsystem(PluginManager& manager);
}}
