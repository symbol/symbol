#pragma once
#include "catapult/plugins.h"

namespace catapult { namespace plugins { class PluginManager; } }

namespace catapult { namespace plugins {

	/// Registers the signature verification system with \a manager.
	PLUGIN_API
	void RegisterSignatureSystem(PluginManager& manager);
}}
