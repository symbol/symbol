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
#include "NodeSelector.h"
#include "catapult/config/NodeConfiguration.h"
#include "catapult/ionet/NodeContainer.h"
#include "catapult/thread/Task.h"
#include <vector>

namespace catapult {
	namespace net {
		class ConnectionContainer;
		class PacketWriters;
	}
}

namespace catapult { namespace extensions {

	/// A node selector.
	using NodeSelector = supplier<NodeSelectionResult>;

	/// Creates and prepares a node selector given \a serviceId, \a requiredRole, \a config and \a nodes.
	/// \note All nodes in \a nodes are provisioned for \a serviceId.
	/// \note The selector is intended to be used in conjunction with CreateConnectPeersTask for managing outgoing connections.
	NodeSelector CreateNodeSelector(
			ionet::ServiceIdentifier serviceId,
			ionet::NodeRoles requiredRole,
			const config::NodeConfiguration::ConnectionsSubConfiguration& config,
			ionet::NodeContainer& nodes);

	/// Creates a task for the service identified by \a serviceId that connects to \a nodes using \a packetWriters and \a selector.
	/// \note \a selector returns add candidates (subset of compatible nodes in \a nodes)
	///        and remove candidates (subset of active connections in \a packetWriters).
	thread::Task CreateConnectPeersTask(
			ionet::NodeContainer& nodes,
			net::PacketWriters& packetWriters,
			ionet::ServiceIdentifier serviceId,
			const NodeSelector& selector);

	/// A remove-only node selector.
	using RemoveOnlyNodeSelector = supplier<utils::KeySet>;

	/// Creates and prepares a remove-only node selector given \a serviceId, \a config and \a nodes.
	/// \note The selector is intended to be used in conjunction with CreateAgePeersTask for managing incoming connections.
	RemoveOnlyNodeSelector CreateRemoveOnlyNodeSelector(
			ionet::ServiceIdentifier serviceId,
			const config::NodeConfiguration::ConnectionsSubConfiguration& config,
			ionet::NodeContainer& nodes);

	/// Creates a task for the service identified by \a serviceId that ages \a nodes using \a connectionContainer and \a selector.
	/// \note \a selector returns remove candidates (subset of active connections in \a connectionContainer).
	thread::Task CreateAgePeersTask(
			ionet::NodeContainer& nodes,
			net::ConnectionContainer& connectionContainer,
			ionet::ServiceIdentifier serviceId,
			const RemoveOnlyNodeSelector& selector);
}}
