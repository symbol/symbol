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

#pragma once
#include "catapult/ionet/NodeSet.h"

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

	/// Determines if \a node is compatible with expected network (\a networkFingerprint) and identity (\a identityKey).
	bool IsNodeCompatible(const ionet::Node& node, const model::UniqueNetworkFingerprint& networkFingerprint, const Key& identityKey);

	/// Filters \a nodes by returning all nodes not contained in \a view.
	/// \note \a nodes is expected to be the result of a peers request and, thus, will not have identity host set.
	ionet::NodeSet SelectUnknownNodes(const ionet::NodeContainerView& view, const ionet::NodeSet& nodes);
}}
