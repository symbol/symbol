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

#include "NodeDiscoveryHandlers.h"
#include "nodediscovery/src/NodePingUtils.h"
#include "catapult/handlers/BasicProducer.h"
#include "catapult/handlers/HandlerFactory.h"
#include "catapult/ionet/NetworkNode.h"
#include "catapult/ionet/PacketPayloadFactory.h"
#include "catapult/utils/HexFormatter.h"

namespace catapult { namespace handlers {

	void RegisterNodeDiscoveryPushPingHandler(
			ionet::ServerPacketHandlers& handlers,
			const model::UniqueNetworkFingerprint& networkFingerprint,
			const NodeConsumer& nodeConsumer) {
		handlers.registerHandler(ionet::PacketType::Node_Discovery_Push_Ping, [networkFingerprint, nodeConsumer](
				const auto& packet,
				const auto& context) {
			ionet::Node node;
			if (!nodediscovery::TryParseNodePacket(packet, node))
				return;

			if (!nodediscovery::IsNodeCompatible(node, networkFingerprint, context.key())) {
				CATAPULT_LOG(warning)
						<< "ignoring ping packet for incompatible node (identity = "
						<< node.identity() << ", network = " << node.metadata().NetworkFingerprint << ")";
				return;
			}

			auto identity = model::NodeIdentity{ node.identity().PublicKey, context.host() };
			auto endpoint = node.endpoint();

			if (endpoint.Host.empty()) {
				endpoint.Host = identity.Host;
				CATAPULT_LOG(debug) << "auto detected host '" << endpoint.Host << "' for " << identity;
			}

			node = ionet::Node(identity, endpoint, node.metadata());
			CATAPULT_LOG(debug) << "processing ping from " << node;
			nodeConsumer(node);
		});
	}

	void RegisterNodeDiscoveryPullPingHandler(
			ionet::ServerPacketHandlers& handlers,
			const std::shared_ptr<const ionet::NetworkNode>& pLocalNode) {
		handlers.registerHandler(ionet::PacketType::Node_Discovery_Pull_Ping, [pLocalNode](const auto& packet, auto& context) {
			if (!IsPacketValid(packet, ionet::PacketType::Node_Discovery_Pull_Ping))
				return;

			context.response(ionet::PacketPayloadFactory::FromEntity(ionet::PacketType::Node_Discovery_Pull_Ping, pLocalNode));
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

	namespace {
		struct NodeDiscoveryPullPeersTraits {
			static constexpr auto Packet_Type = ionet::PacketType::Node_Discovery_Pull_Peers;

			class Producer : BasicProducer<ionet::NodeSet> {
			public:
				explicit Producer(const ionet::NodeSet& nodes) : BasicProducer<ionet::NodeSet>(nodes)
				{}

				auto operator()() {
					return next([](const auto& node) {
						return utils::UniqueToShared(ionet::PackNode(node));
					});
				}
			};
		};
	}

	void RegisterNodeDiscoveryPullPeersHandler(ionet::ServerPacketHandlers& handlers, const NodesSupplier& nodesSupplier) {
		handlers::BatchHandlerFactory<NodeDiscoveryPullPeersTraits>::RegisterZero(handlers, [nodesSupplier]() {
			auto pNodes = std::make_unique<ionet::NodeSet>(nodesSupplier()); // used by producer by reference
			auto producer = NodeDiscoveryPullPeersTraits::Producer(*pNodes);
			return [pNodes = std::move(pNodes), producer]() mutable {
				return producer();
			};
		});
	}
}}
