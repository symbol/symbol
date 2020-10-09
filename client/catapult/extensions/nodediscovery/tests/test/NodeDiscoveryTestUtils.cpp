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

#include "NodeDiscoveryTestUtils.h"
#include "catapult/ionet/NetworkNode.h"
#include "catapult/utils/HexParser.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/test/nodeps/Random.h"

namespace catapult { namespace test {

	model::UniqueNetworkFingerprint CreateNodeDiscoveryNetworkFingerprint() {
		constexpr auto Generation_Hash_Seed_String = "272C4ECC55B7A42A07478A9550543C62673D1599A8362CC662E019049B76B7F2";
		return model::UniqueNetworkFingerprint(
				model::NetworkIdentifier::Private_Test,
				utils::ParseByteArray<GenerationHashSeed>(Generation_Hash_Seed_String));
	}

	std::unique_ptr<ionet::NetworkNode> CreateNetworkNode(const std::string& host, const std::string& name) {
		auto hostSize = static_cast<uint8_t>(host.size());
		auto nameSize = static_cast<uint8_t>(name.size());
		uint32_t size = SizeOf32<ionet::NetworkNode>() + hostSize + nameSize;

		auto pNetworkNode = utils::MakeUniqueWithSize<ionet::NetworkNode>(size);
		FillWithRandomData({ reinterpret_cast<uint8_t*>(pNetworkNode.get()), size });
		pNetworkNode->Size = size;
		pNetworkNode->HostSize = hostSize;
		pNetworkNode->FriendlyNameSize = nameSize;
		return pNetworkNode;
	}

	std::shared_ptr<ionet::Packet> CreateNodePushPingPacket(
			const Key& identityKey,
			ionet::NodeVersion version,
			const std::string& host,
			const std::string& name) {
		auto networkFingerprint = CreateNodeDiscoveryNetworkFingerprint();

		auto hostSize = static_cast<uint8_t>(host.size());
		auto nameSize = static_cast<uint8_t>(name.size());
		uint32_t payloadSize = SizeOf32<ionet::NetworkNode>() + hostSize + nameSize;
		auto pPacket = test::CreateRandomPacket(payloadSize, ionet::PacketType::Node_Discovery_Push_Ping);
		auto& networkNode = reinterpret_cast<ionet::NetworkNode&>(*pPacket->Data());
		networkNode.Size = payloadSize;
		networkNode.IdentityKey = identityKey;
		networkNode.NetworkIdentifier = networkFingerprint.Identifier;
		networkNode.NetworkGenerationHashSeed = networkFingerprint.GenerationHashSeed;
		networkNode.Version = version;
		networkNode.HostSize = hostSize;
		networkNode.FriendlyNameSize = nameSize;
		std::memcpy(pPacket->Data() + sizeof(ionet::NetworkNode), host.data(), hostSize);
		std::memcpy(pPacket->Data() + sizeof(ionet::NetworkNode) + hostSize, name.data(), nameSize);
		return pPacket;
	}

	std::shared_ptr<ionet::Packet> CreateNodePushPeersPacket(const std::vector<ionet::Node>& nodes) {
		// pack nodes and determine payload size
		auto payloadSize = 0u;
		auto networkNodes = PackAllNodes(nodes);
		for (const auto& pNetworkNode : networkNodes)
			payloadSize += pNetworkNode->Size;

		// prepare packet
		auto pPacket = ionet::CreateSharedPacket<ionet::Packet>(payloadSize);
		pPacket->Type = ionet::PacketType::Node_Discovery_Push_Peers;

		auto* pData = pPacket->Data();
		for (const auto& pNetworkNode : networkNodes) {
			std::memcpy(pData, pNetworkNode.get(), pNetworkNode->Size);
			pData += pNetworkNode->Size;
		}

		return pPacket;
	}
}}
