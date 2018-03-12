#include "NodeDiscoveryHandlers.h"
#include "nodediscovery/src/NodePingUtils.h"
#include "catapult/ionet/NetworkNode.h"
#include "catapult/ionet/PacketEntityUtils.h"
#include "catapult/utils/HexFormatter.h"

namespace catapult { namespace handlers {

	void RegisterNodeDiscoveryPushPingHandler(
			ionet::ServerPacketHandlers& handlers,
			model::NetworkIdentifier networkIdentifier,
			const NodeConsumer& nodeConsumer) {
		handlers.registerHandler(ionet::PacketType::Node_Discovery_Push_Ping, [networkIdentifier, nodeConsumer](
				const auto& packet,
				const auto& context) {
			ionet::Node node;
			if (!nodediscovery::TryParseNodePacket(packet, node))
				return;

			if (!nodediscovery::IsNodeCompatible(node, networkIdentifier, context.key())) {
				CATAPULT_LOG(warning)
						<< "ignoring ping packet for incompatible node (identity = "
						<< utils::HexFormat(node.identityKey()) << ", network = " << node.metadata().NetworkIdentifier << ")";
				return;
			}

			if (node.endpoint().Host.empty()) {
				auto endpoint = ionet::NodeEndpoint{ context.host(), node.endpoint().Port };
				CATAPULT_LOG(debug) << "auto detected host '" << endpoint.Host << "' for " << utils::HexFormat(node.identityKey());
				node = ionet::Node(node.identityKey(), endpoint, node.metadata());
			}

			nodeConsumer(node);
		});
	}

	void RegisterNodeDiscoveryPullPingHandler(ionet::ServerPacketHandlers& handlers, const ionet::NetworkNode& localNode) {
		handlers.registerHandler(ionet::PacketType::Node_Discovery_Pull_Ping, [&localNode](const auto& packet, auto& context) {
			if (!IsPacketValid(packet, ionet::PacketType::Node_Discovery_Pull_Ping))
				return;

			auto pResponsePacket = ionet::CreateSharedPacket<ionet::Packet>(localNode.Size);
			pResponsePacket->Type = ionet::PacketType::Node_Discovery_Pull_Ping;
			memcpy(pResponsePacket->Data(), &localNode, localNode.Size);
			context.response(pResponsePacket);
		});
	}

	void RegisterNodeDiscoveryPushPeersHandler(ionet::ServerPacketHandlers& handlers, const NodesConsumer& nodesConsumer) {
		handlers.registerHandler(ionet::PacketType::Node_Discovery_Push_Peers, [nodesConsumer](const auto& packet, const auto&) {
			ionet::NodeSet nodes;
			if (!nodediscovery::TryParseNodesPacket(packet, nodes) || nodes.empty())
				return;

			// nodes are not checked for compatibility here because they are being forwarded from a potentially malicious node
			// (incompatible nodes will be filtered out later by ping push handler)
			nodesConsumer(nodes);
		});
	}

	void RegisterNodeDiscoveryPullPeersHandler(ionet::ServerPacketHandlers& handlers, const NodesSupplier& nodesSupplier) {
		handlers.registerHandler(ionet::PacketType::Node_Discovery_Pull_Peers, [nodesSupplier](const auto& packet, auto& context) {
			if (!IsPacketValid(packet, ionet::PacketType::Node_Discovery_Pull_Peers))
				return;

			// send a response packet even if there are no active nodes in order to acknowledge the request
			std::vector<std::shared_ptr<ionet::NetworkNode>> networkNodes;
			for (const auto& node : nodesSupplier())
				networkNodes.emplace_back(ionet::PackNode(node));

			context.response(ionet::PacketPayload::FromEntities(ionet::PacketType::Node_Discovery_Pull_Peers, networkNodes));
		});
	}
}}
