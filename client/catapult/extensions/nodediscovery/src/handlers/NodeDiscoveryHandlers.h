#pragma once
#include "catapult/ionet/Node.h"
#include "catapult/ionet/PacketHandlers.h"
#include "catapult/model/NetworkInfo.h"

namespace catapult { namespace ionet { struct NetworkNode; } }

namespace catapult { namespace handlers {

	/// Consumes a node.
	using NodeConsumer = consumer<const ionet::Node&>;

	/// Consumes multiple nodes.
	using NodesConsumer = consumer<const ionet::NodeSet&>;

	/// Supplies nodes.
	using NodesSupplier = supplier<ionet::NodeSet>;

	/// Registers a node discovery push ping handler in \a handlers that forwards node information to \a nodeConsumer given
	/// the current network identifier (\a networkIdentifier).
	void RegisterNodeDiscoveryPushPingHandler(
			ionet::ServerPacketHandlers& handlers,
			model::NetworkIdentifier networkIdentifier,
			const NodeConsumer& nodeConsumer);

	/// Registers a node discovery pull ping handler in \a handlers that responds with \a localNode.
	void RegisterNodeDiscoveryPullPingHandler(ionet::ServerPacketHandlers& handlers, const ionet::NetworkNode& localNode);

	/// Registers a node discovery push peers handler in \a handlers that forwards received nodes to \a nodesConsumer.
	void RegisterNodeDiscoveryPushPeersHandler(ionet::ServerPacketHandlers& handlers, const NodesConsumer& nodesConsumer);

	/// Registers a node discovery pull peers handler in \a handlers that responds with nodes from \a nodesSupplier.
	void RegisterNodeDiscoveryPullPeersHandler(ionet::ServerPacketHandlers& handlers, const NodesSupplier& nodesSupplier);
}}
