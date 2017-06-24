#pragma once
#include "catapult/ionet/Node.h"
#include <vector>

namespace catapult { namespace config {

	/// Loads peers from the specified stream (\a input) for the network identified by \a networkIdentifier.
	std::vector<ionet::Node> LoadPeersFromStream(std::istream& input, model::NetworkIdentifier networkIdentifier);

	/// Loads peers from the specified \a path for the network identified by \a networkIdentifier.
	std::vector<ionet::Node> LoadPeersFromPath(const std::string& path, model::NetworkIdentifier networkIdentifier);
}}
