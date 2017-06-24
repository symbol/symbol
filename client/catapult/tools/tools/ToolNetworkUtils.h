#pragma once
#include "catapult/types.h"
#include <string>

namespace catapult {
	namespace config { class LocalNodeConfiguration; }
	namespace tools { class NetworkConnections; }
}

namespace catapult { namespace tools {

	/// Gets the current network height using \a connections.
	Height GetHeight(const NetworkConnections& connections);

	/// Waits for the network to produce \a numBlocks blocks using \a connections.
	bool WaitForBlocks(const NetworkConnections& connections, size_t numBlocks);

	/// Loads the configuration from \a resourcesPath.
	config::LocalNodeConfiguration LoadConfiguration(const std::string& resourcesPath);
}}
