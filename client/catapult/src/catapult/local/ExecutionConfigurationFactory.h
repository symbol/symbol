#pragma once
#include "catapult/chain/ExecutionConfiguration.h"

namespace catapult { namespace plugins { class PluginManager; } }

namespace catapult { namespace local {

	/// Creates an execution configuration based on \a pluginManager.
	chain::ExecutionConfiguration CreateExecutionConfiguration(const plugins::PluginManager& pluginManager);
}}
