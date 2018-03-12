#pragma once
#include "catapult/ionet/Node.h"
#include "catapult/ionet/NodeInfo.h"
#include "catapult/utils/ArraySet.h"

namespace catapult { namespace ionet { class NodeContainer; } }

namespace catapult { namespace extensions {

	/// The result of a node selection.
	struct NodeSelectionResult {
		/// The nodes that should be activatated.
		ionet::NodeSet AddCandidates;

		/// The identities of the nodes that should be deactivated.
		utils::KeySet RemoveCandidates;
	};

	/// Node aging configuration.
	struct NodeAgingConfiguration {
		/// The identity of the service for which nodes should be selected.
		ionet::ServiceIdentifier ServiceId;

		/// The maximum number of connections (per service).
		uint32_t MaxConnections;

		/// The maximum connection age.
		uint32_t MaxConnectionAge;
	};

	/// Node selection configuration.
	struct NodeSelectionConfiguration {
		/// The identity of the service for which nodes should be selected.
		ionet::ServiceIdentifier ServiceId;

		/// The required node role.
		ionet::NodeRoles RequiredRole;

		/// The maximum number of connections (per service).
		uint32_t MaxConnections;

		/// The maximum connection age.
		uint32_t MaxConnectionAge;
	};

	/// Calculates the weight for \a connectionState.
	uint32_t CalculateWeight(const ionet::ConnectionState& connectionState);

	/// Selects the subset of \a nodes to activate and deactivate according to \a config.
	/// \note This function is intended for management of outgoing connections.
	NodeSelectionResult SelectNodes(const ionet::NodeContainer& nodes, const NodeSelectionConfiguration& config);

	/// Selects the subset of \a nodes to deactivate according to \a config.
	/// \note This function is intended for management of incoming connections.
	utils::KeySet SelectNodesForRemoval(const ionet::NodeContainer& nodes, const NodeAgingConfiguration& config);
}}
