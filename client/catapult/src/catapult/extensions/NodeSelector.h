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

#pragma once
#include "catapult/ionet/IpProtocol.h"
#include "catapult/ionet/NodeInfo.h"
#include "catapult/ionet/NodeSet.h"
#include "catapult/utils/ArraySet.h"
#include "catapult/utils/RandomGenerator.h"

namespace catapult { namespace ionet { class NodeContainer; } }

namespace catapult { namespace extensions {

	// region ImportanceDescriptor / ImportanceRetriever

	/// Describes an importance value.
	struct ImportanceDescriptor {
		/// Associated importance.
		catapult::Importance Importance;

		/// Total chain importance.
		catapult::Importance TotalChainImportance;
	};

	/// Retrieves an importance descriptor given a specified public key.
	using ImportanceRetriever = std::function<ImportanceDescriptor (const Key&)>;

	// endregion

	// region WeightedCandidate(s)

	/// Weighted candidate.
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

	// endregion

	// region NodeSelectionResult

	/// Result of a node selection.
	struct NodeSelectionResult {
	public:
		/// Creates a result.
		NodeSelectionResult() : RemoveCandidates(model::CreateNodeIdentitySet(model::NodeIdentityEqualityStrategy::Key_And_Host))
		{}

	public:
		/// Nodes that should be activatated.
		ionet::NodeSet AddCandidates;

		/// Identities of the nodes that should be deactivated.
		model::NodeIdentitySet RemoveCandidates;
	};

	// endregion

	// region NodeAgingConfiguration / NodeSelectionConfiguration

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

		/// Supported protocols.
		ionet::IpProtocol SupportedProtocols;

		/// Required node role.
		ionet::NodeRoles RequiredRole;

		/// Maximum number of connections (per service).
		uint32_t MaxConnections;

		/// Maximum connection age.
		uint32_t MaxConnectionAge;
	};

	// endregion

	// region WeightPolicy / WeightPolicyGenerator

	/// Weight calculation policies.
	enum class WeightPolicy {
		/// Weight is calculated using the interaction statistics.
		Interactions,

		/// Weight is calculated using importance.
		Importance
	};

	/// Weight policy generator.
	class WeightPolicyGenerator {
	public:
		/// Creates a default weight policy generator.
		WeightPolicyGenerator() : m_distr(1, 4)
		{}

	public:
		/// Generates the next weight policy.
		WeightPolicy operator()() {
			return 4 == static_cast<uint32_t>(m_distr(m_generator)) ? WeightPolicy::Importance : WeightPolicy::Interactions;
		}

	private:
		utils::LowEntropyRandomGenerator m_generator;
		std::uniform_int_distribution<> m_distr;
	};

	// endregion

	/// Calculates the weight from \a interactions or \a importanceSupplier depending on \a weightPolicy.
	uint32_t CalculateWeight(
			const ionet::NodeInteractions& interactions,
			WeightPolicy weightPolicy,
			const supplier<ImportanceDescriptor>& importanceSupplier);

	/// Finds at most \a maxCandidates add candidates from container \a candidates given a
	/// total candidate weight (\a totalCandidateWeight).
	ionet::NodeSet SelectCandidatesBasedOnWeight(
			const WeightedCandidates& candidates,
			uint64_t totalCandidateWeight,
			size_t maxCandidates);

	/// Selects the subset of \a nodes to activate and deactivate according to \a config and \a importanceRetriever.
	/// \note This function is intended for management of outgoing connections.
	NodeSelectionResult SelectNodes(
			const ionet::NodeContainer& nodes,
			const NodeSelectionConfiguration& config,
			const ImportanceRetriever& importanceRetriever);

	/// Selects the subset of \a nodes to deactivate according to \a config and \a importanceRetriever.
	/// \note This function is intended for management of incoming connections.
	model::NodeIdentitySet SelectNodesForRemoval(
			const ionet::NodeContainer& nodes,
			const NodeAgingConfiguration& config,
			const ImportanceRetriever& importanceRetriever);
}}
