#pragma once
#include "catapult/ionet/NetworkNode.h"
#include <vector>

namespace catapult { namespace ionet { struct Packet; } }

namespace catapult { namespace test {

	/// Creates a random network node with \a host and \a name.
	std::unique_ptr<ionet::NetworkNode> CreateNetworkNode(const std::string& host, const std::string& name);

	/// Packs all node models (\a nodes) into a vector of network nodes.
	template<typename TNodeContainer>
	std::vector<std::unique_ptr<ionet::NetworkNode>> PackAllNodes(const TNodeContainer& nodes) {
		std::vector<std::unique_ptr<ionet::NetworkNode>> networkNodes;
		for (const auto& node : nodes)
			networkNodes.push_back(ionet::PackNode(node));

		return networkNodes;
	}

	/// Creates a node push ping packet with \a identityKey, \a version, \a host and \a name.
	std::shared_ptr<ionet::Packet> CreateNodePushPingPacket(
			const Key& identityKey,
			ionet::NodeVersion version,
			const std::string& host,
			const std::string& name);

	/// Creates a node push peers packet with \a nodes.
	std::shared_ptr<ionet::Packet> CreateNodePushPeersPacket(const std::vector<ionet::Node>& nodes);
}}
