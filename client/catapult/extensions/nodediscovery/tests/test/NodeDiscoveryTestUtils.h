/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#pragma once
#include "catapult/ionet/NetworkNode.h"
#include <vector>

namespace catapult { namespace ionet { struct Packet; } }

namespace catapult { namespace test {

	/// Creates network fingerprint for node discovery tests.
	model::UniqueNetworkFingerprint CreateNodeDiscoveryNetworkFingerprint();

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
