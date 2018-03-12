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
