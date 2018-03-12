#pragma once
#include "catapult/ionet/Node.h"

namespace catapult {
	namespace ionet {
		class NodeContainerView;
		struct Packet;
	}
}

namespace catapult { namespace nodediscovery {

	/// Tries to parse \a packet into \a node.
	bool TryParseNodePacket(const ionet::Packet& packet, ionet::Node& node);

	/// Tries to parse \a packet into \a nodes.
	bool TryParseNodesPacket(const ionet::Packet& packet, ionet::NodeSet& nodes);

	/// Determines if \a node is compatible with expected network (\a networkIdentifier) and identity (\a identityKey).
	bool IsNodeCompatible(const ionet::Node& node, model::NetworkIdentifier networkIdentifier, const Key& identityKey);

	/// Filters \a nodes by returning all nodes not contained in \a view.
	ionet::NodeSet SelectUnknownNodes(const ionet::NodeContainerView& view, const ionet::NodeSet& nodes);
}}
