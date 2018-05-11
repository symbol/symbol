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
#include "catapult/ionet/Node.h"
#include "catapult/ionet/NodeInfo.h"
#include "catapult/utils/ArraySet.h"

namespace catapult { namespace ionet { class NodeContainer; } }

namespace catapult { namespace extensions {

	/// A weighted candidate.
	struct WeightedCandidate {
	public:
		/// Creates a weighted candidate around \a node and \a weight.
		WeightedCandidate(const ionet::Node& node, uint64_t weight)
				: Node(node)
				, Weight(weight)
		{}

	public:
		/// Node.
		const ionet::Node& Node;

		/// Weight of the node.
		uint64_t Weight;
	};

	using WeightedCandidates = std::vector<WeightedCandidate>;

	/// Result of a node selection.
	struct NodeSelectionResult {
		/// Nodes that should be activatated.
		ionet::NodeSet AddCandidates;

		/// Identities of the nodes that should be deactivated.
		utils::KeySet RemoveCandidates;
	};

	/// Node aging configuration.
	struct NodeAgingConfiguration {
		/// Identity of the service for which nodes should be selected.
		ionet::ServiceIdentifier ServiceId;

		/// Maximum number of connections (per service).
		uint32_t MaxConnections;

		/// Maximum connection age.
		uint32_t MaxConnectionAge;
	};

	/// Node selection configuration.
	struct NodeSelectionConfiguration {
		/// Identity of the service for which nodes should be selected.
		ionet::ServiceIdentifier ServiceId;

		/// Required node role.
		ionet::NodeRoles RequiredRole;

		/// Maximum number of connections (per service).
		uint32_t MaxConnections;

		/// Maximum connection age.
		uint32_t MaxConnectionAge;
	};

	/// Calculates the weight for \a connectionState.
	uint32_t CalculateWeight(const ionet::ConnectionState& connectionState);

	/// Finds at most \a maxCandidates add candidates from container \a candidates given a
	/// total candidate weight (\a totalCandidateWeight).
	ionet::NodeSet SelectCandidatesBasedOnWeight(
			const WeightedCandidates& candidates,
			uint64_t totalCandidateWeight,
			size_t maxCandidates);

	/// Selects the subset of \a nodes to activate and deactivate according to \a config.
	/// \note This function is intended for management of outgoing connections.
	NodeSelectionResult SelectNodes(const ionet::NodeContainer& nodes, const NodeSelectionConfiguration& config);

	/// Selects the subset of \a nodes to deactivate according to \a config.
	/// \note This function is intended for management of incoming connections.
	utils::KeySet SelectNodesForRemoval(const ionet::NodeContainer& nodes, const NodeAgingConfiguration& config);
}}
