#pragma once
#include "catapult/config/LocalNodeConfiguration.h"

namespace catapult { namespace tools {

	/// Loads the configuration from \a resourcesPath.
	config::LocalNodeConfiguration LoadConfiguration(const std::string& resourcesPath);

	/// Loads optional api peers configuration from \a resourcesPath for network \a networkIdentifier.
	std::vector<ionet::Node> LoadOptionalApiPeers(const std::string& resourcesPath, model::NetworkIdentifier networkIdentifier);

	/// Loads p2p peers configuration from \a resourcesPath for network \a networkIdentifier.
	std::vector<ionet::Node> LoadPeers(const std::string& resourcesPath, model::NetworkIdentifier networkIdentifier);
}}
