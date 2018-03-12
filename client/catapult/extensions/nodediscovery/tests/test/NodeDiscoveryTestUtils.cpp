#include "NodeDiscoveryTestUtils.h"
#include "catapult/ionet/NetworkNode.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/test/nodeps/Random.h"

namespace catapult { namespace test {

	std::unique_ptr<ionet::NetworkNode> CreateNetworkNode(const std::string& host, const std::string& name) {
		auto hostSize = static_cast<uint8_t>(host.size());
		auto nameSize = static_cast<uint8_t>(name.size());
		uint32_t size = sizeof(ionet::NetworkNode) + hostSize + nameSize;

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
		auto hostSize = static_cast<uint8_t>(host.size());
		auto nameSize = static_cast<uint8_t>(name.size());
		uint32_t payloadSize = sizeof(ionet::NetworkNode) + hostSize + nameSize;
		auto pPacket = test::CreateRandomPacket(payloadSize, ionet::PacketType::Node_Discovery_Push_Ping);
		auto& networkNode = reinterpret_cast<ionet::NetworkNode&>(*pPacket->Data());
		networkNode.Size = payloadSize;
		networkNode.IdentityKey = identityKey;
		networkNode.NetworkIdentifier = model::NetworkIdentifier::Mijin_Test;
		networkNode.Version = version;
		networkNode.HostSize = hostSize;
		networkNode.FriendlyNameSize = nameSize;
		memcpy(pPacket->Data() + sizeof(ionet::NetworkNode), host.data(), hostSize);
		memcpy(pPacket->Data() + sizeof(ionet::NetworkNode) + hostSize, name.data(), nameSize);
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

		auto pData = pPacket->Data();
		for (const auto& pNetworkNode : networkNodes) {
			memcpy(pData, pNetworkNode.get(), pNetworkNode->Size);
			pData += pNetworkNode->Size;
		}

		return pPacket;
	}
}}
