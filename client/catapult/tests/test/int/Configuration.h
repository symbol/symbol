#pragma once
#include "catapult/config/LocalNodeConfiguration.h"
#include <string>

namespace catapult { namespace test {

	/// Adds configuration for all plugins required by the default nemesis block to \a config.
	void AddNemesisPluginExtensions(model::BlockChainConfiguration& config);

	/// Creates a test configuration for a local node with a storage in the specified directory (\a dataDirectory)
	/// that includes configuration for all plugins required by the default nemesis block.
	config::LocalNodeConfiguration LoadLocalNodeConfigurationWithNemesisPluginExtensions(const std::string& dataDirectory);
}}
