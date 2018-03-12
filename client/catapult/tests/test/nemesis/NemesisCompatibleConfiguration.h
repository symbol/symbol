#pragma once
#include "catapult/config/LocalNodeConfiguration.h"
#include <string>

namespace catapult { namespace test {

	/// Adds configuration for all plugins required by the default nemesis block to \a config.
	void AddNemesisPluginExtensions(model::BlockChainConfiguration& config);

	/// Adds configuration for all extensions required by api nodes to \a config.
	void AddApiPluginExtensions(config::NodeConfiguration& config);

	/// Adds configuration for all extensions required by p2p nodes to \a config.
	void AddPeerPluginExtensions(config::NodeConfiguration& config);

	/// Adds configuration for all extensions required by simple partner nodes to \a config.
	void AddSimplePartnerPluginExtensions(config::NodeConfiguration& config);

	/// Creates a test configuration for a local node with a storage in the specified directory (\a dataDirectory)
	/// that includes configuration for all plugins required by the default nemesis block.
	config::LocalNodeConfiguration LoadLocalNodeConfigurationWithNemesisPluginExtensions(const std::string& dataDirectory);
}}
