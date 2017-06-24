#pragma once
#include "catapult/plugins.h"

namespace catapult { namespace plugins { class PluginManager; } }

namespace catapult { namespace plugins {

	/// Registers namespace support with \a manager.
	PLUGIN_API
	void RegisterNamespaceSubsystem(PluginManager& manager);
}}
