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

#include "NetworkNode.h"
#include "catapult/utils/MemoryUtils.h"

namespace catapult { namespace ionet {

	namespace {
		uint8_t GetPackedSize(const std::string& str) {
			return static_cast<uint8_t>(std::min<size_t>(str.size(), std::numeric_limits<uint8_t>::max()));
		}
	}

	std::unique_ptr<NetworkNode> PackNode(const Node& node) {
		const auto& endpoint = node.endpoint();
		const auto& metadata = node.metadata();

		auto hostSize = GetPackedSize(endpoint.Host);
		auto friendlyNameSize = GetPackedSize(metadata.Name);
		uint32_t packedNodeSize = SizeOf32<NetworkNode>() + hostSize + friendlyNameSize;
		auto pNetworkNode = utils::MakeUniqueWithSize<NetworkNode>(packedNodeSize);

		pNetworkNode->Size = packedNodeSize;
		pNetworkNode->Port = endpoint.Port;
		pNetworkNode->IdentityKey = node.identity().PublicKey;
		pNetworkNode->NetworkIdentifier = metadata.NetworkFingerprint.Identifier;
		pNetworkNode->NetworkGenerationHashSeed = metadata.NetworkFingerprint.GenerationHashSeed;
		pNetworkNode->Version = metadata.Version;
		pNetworkNode->Roles = metadata.Roles;

		pNetworkNode->HostSize = hostSize;
		pNetworkNode->FriendlyNameSize = friendlyNameSize;

		auto* pNetworkNodeData = reinterpret_cast<uint8_t*>(pNetworkNode.get() + 1);
		std::memcpy(pNetworkNodeData, endpoint.Host.c_str(), hostSize);
		pNetworkNodeData += hostSize;

		std::memcpy(pNetworkNodeData, metadata.Name.c_str(), friendlyNameSize);
		return pNetworkNode;
	}

	Node UnpackNode(const NetworkNode& networkNode) {
		const auto* pNetworkNodeData = reinterpret_cast<const char*>(&networkNode + 1);

		auto identity = model::NodeIdentity();
		identity.PublicKey = networkNode.IdentityKey;

		auto endpoint = NodeEndpoint();
		endpoint.Port = networkNode.Port;
		endpoint.Host = std::string(pNetworkNodeData, networkNode.HostSize);
		pNetworkNodeData += networkNode.HostSize;

		auto metadata = NodeMetadata();
		metadata.NetworkFingerprint.Identifier = networkNode.NetworkIdentifier;
		metadata.NetworkFingerprint.GenerationHashSeed = networkNode.NetworkGenerationHashSeed;
		metadata.Name = std::string(pNetworkNodeData, networkNode.FriendlyNameSize);
		metadata.Version = networkNode.Version;
		metadata.Roles = networkNode.Roles;

		return Node(identity, endpoint, metadata);
	}
}}
