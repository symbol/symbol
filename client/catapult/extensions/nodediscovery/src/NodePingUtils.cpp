/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

#include "NodePingUtils.h"
#include "catapult/ionet/NetworkNode.h"
#include "catapult/ionet/NodeContainer.h"
#include "catapult/ionet/PacketEntityUtils.h"
#include "catapult/model/SizeChecker.h"

namespace catapult { namespace nodediscovery {

	bool TryParseNodePacket(const ionet::Packet& packet, ionet::Node& node) {
		auto dataSize = ionet::CalculatePacketDataSize(packet);
		if (!ionet::ContainsSingleEntity<ionet::NetworkNode>({ packet.Data(), dataSize }, model::IsSizeValidT<ionet::NetworkNode>)) {
			CATAPULT_LOG(warning) << "node packet is malformed with size " << dataSize;
			return false;
		}

		node = UnpackNode(reinterpret_cast<const ionet::NetworkNode&>(*packet.Data()));
		return true;
	}

	bool TryParseNodesPacket(const ionet::Packet& packet, ionet::NodeSet& nodes) {
		auto range = ionet::ExtractEntitiesFromPacket<ionet::NetworkNode>(packet, model::IsSizeValidT<ionet::NetworkNode>);
		if (range.empty() && sizeof(ionet::PacketHeader) != packet.Size) {
			CATAPULT_LOG(warning) << "rejecting empty range: " << packet;
			return false;
		}

		for (const auto& networkNode : range)
			nodes.emplace(ionet::UnpackNode(networkNode));

		return true;
	}

	bool IsNodeCompatible(const ionet::Node& node, const model::UniqueNetworkFingerprint& networkFingerprint, const Key& identityKey) {
		// this function is used to check that a remote node returns expected identity key to local node
		// in order to prevent remote from announcing one key but then returning information for another node
		return node.metadata().NetworkFingerprint == networkFingerprint && node.identity().PublicKey == identityKey;
	}

	ionet::NodeSet SelectUnknownNodes(const ionet::NodeContainerView& view, const ionet::NodeSet& nodes) {
		std::unordered_map<Key, ionet::Node, utils::ArrayHasher<Key>> identityKeyToNodeMap;
		for (const auto& node : nodes)
			identityKeyToNodeMap.emplace(node.identity().PublicKey, node);

		// filter nodes based on identity key because they will not have identity host set
		view.forEach([&identityKeyToNodeMap](const auto& node, const auto&) {
			auto mapIter = identityKeyToNodeMap.find(node.identity().PublicKey);
			if (identityKeyToNodeMap.cend() != mapIter)
				identityKeyToNodeMap.erase(mapIter);
		});

		ionet::NodeSet unknownNodes;
		for (const auto& pair : identityKeyToNodeMap)
			unknownNodes.emplace(pair.second);

		CATAPULT_LOG(debug) << "reduced " << nodes.size() << " candidates to " << unknownNodes.size();
		return unknownNodes;
	}
}}
