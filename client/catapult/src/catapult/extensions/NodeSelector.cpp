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

#include "NodeSelector.h"
#include "catapult/ionet/NodeContainer.h"

namespace catapult { namespace extensions {

	namespace {
		constexpr uint32_t GetWeightMultipler(ionet::NodeSource source) {
			switch (source) {
			case ionet::NodeSource::Dynamic:
				return 1;

			case ionet::NodeSource::Static:
				return 2;

			default:
				return 0;
			}
		}

		using NodeScorePairs = std::vector<std::pair<ionet::Node, uint32_t>>;

		struct ServiceNodesInfo {
			NodeScorePairs Actives; // node to age pairs
			WeightedCandidates Candidates; // candidate nodes with weight
			uint64_t TotalCandidateWeight = 0;
		};

		struct FindServiceNodesFilter {
			ionet::ServiceIdentifier ServiceId;
			ionet::IpProtocol SupportedProtocols;
			ionet::NodeRoles RequiredRole;
		};

		ServiceNodesInfo FindServiceNodes(
				const ionet::NodeContainerView& nodes,
				const FindServiceNodesFilter& filter,
				const ImportanceRetriever& importanceRetriever) {
			ServiceNodesInfo nodesInfo;
			auto timestamp = nodes.time();
			WeightPolicyGenerator generator;
			nodes.forEach([filter, importanceRetriever, &generator, timestamp, &nodesInfo](const auto& node, const auto& nodeInfo) {
				auto weightMultiplier = GetWeightMultipler(nodeInfo.source());
				const auto* pConnectionState = nodeInfo.getConnectionState(filter.ServiceId);
				if (!pConnectionState)
					return;

				// decrease weight of banned nodes (this blocks banned dynamic nodes while allowing reconnects to banned static nodes)
				weightMultiplier -= 0 == pConnectionState->BanAge ? 0 : 1;
				if (0 == weightMultiplier)
					return;

				auto nodeRoles = node.metadata().Roles;
				if (!HasFlag(filter.RequiredRole, nodeRoles) || !HasAnyProtocol(filter.SupportedProtocols, nodeRoles))
					return;

				// if the node is associated with the current service, mark it as either active or candidate
				if (pConnectionState->Age > 0) {
					nodesInfo.Actives.emplace_back(node, pConnectionState->Age);
				} else {
					auto interactions = nodeInfo.interactions(timestamp);
					auto weight = CalculateWeight(interactions, generator(), [importanceRetriever, &node]() {
						return importanceRetriever(node.identity().PublicKey);
					});
					nodesInfo.Candidates.emplace_back(node, weight * weightMultiplier);
					nodesInfo.TotalCandidateWeight += nodesInfo.Candidates.back().Weight;
				}
			});

			return nodesInfo;
		}

		model::NodeIdentitySet FindRemoveCandidates(const NodeScorePairs& nodePairs, uint32_t maxConnections, uint32_t maxConnectionAge) {
			// never remove the last connection
			auto removeCandidates = model::CreateNodeIdentitySet(model::NodeIdentityEqualityStrategy::Key_And_Host);
			if (nodePairs.size() <= 1)
				return removeCandidates;

			// 1. only remove nodes with sufficient age
			// 2. always remove all connections above `maxConnections`
			// 3. always remove at least one connection to force reconnection of zombies
			auto maxNodesToRemove = (nodePairs.size() >= maxConnections ? nodePairs.size() - maxConnections : 0) + 1;
			for (const auto& pair : nodePairs) {
				if (removeCandidates.size() == maxNodesToRemove)
					break;

				if (pair.second >= maxConnectionAge)
					removeCandidates.emplace(pair.first.identity());
			}

			return removeCandidates;
		}

		size_t FindCandidateIndex(const WeightedCandidates& candidates, const std::vector<bool>& usedNodeFlags, uint64_t selectedWeight) {
			uint64_t cumulativeWeight = 0;
			auto lastUnusedNodeIndex = 0u;
			for (auto i = 0u; i < candidates.size(); ++i) {
				if (usedNodeFlags[i])
					continue;

				lastUnusedNodeIndex = i;
				cumulativeWeight += candidates[i].Weight;
				if (cumulativeWeight >= selectedWeight)
					return i;
			}

			return lastUnusedNodeIndex;
		}
	}

	uint32_t CalculateWeight(
			const ionet::NodeInteractions& interactions,
			WeightPolicy weightPolicy,
			const supplier<ImportanceDescriptor>& importanceSupplier) {
		// return a weight in range of 1..10'000
		if (WeightPolicy::Importance == weightPolicy) {
			// the weight of a supernode should be 10'000; a supernode has ~0.0333% importance
			auto descriptor = importanceSupplier();
			auto rawWeight = static_cast<uint32_t>(descriptor.Importance.unwrap() * 30'000'000 / descriptor.TotalChainImportance.unwrap());
			return std::max<uint32_t>({ 500, std::min<uint32_t>({ 10'000, rawWeight }) });
		} else {
			auto numAttempts = interactions.NumSuccesses + interactions.NumFailures;
			if (3 >= numAttempts)
				return 5'000;

			auto weight = interactions.NumSuccesses * 10'000 / (interactions.NumSuccesses + 9 * interactions.NumFailures);
			return std::max<uint32_t>({ 500, weight });
		}
	}

	ionet::NodeSet SelectCandidatesBasedOnWeight(
			const WeightedCandidates& candidates,
			uint64_t totalCandidateWeight,
			size_t maxCandidates) {
		ionet::NodeSet addCandidates;

		// if the number of nodes does not exceed `maxCandidates`, select all
		if (candidates.size() <= maxCandidates) {
			for (const auto& candidate : candidates)
				addCandidates.emplace(candidate.Node);

			return addCandidates;
		}

		utils::LowEntropyRandomGenerator generator;
		std::uniform_int_distribution<uint64_t> distr(0, totalCandidateWeight);
		std::vector<bool> usedNodeFlags(candidates.size(), false);
		for (auto i = 0u; i < maxCandidates; ++i) {
			auto randomWeight = distr(generator);
			auto index = FindCandidateIndex(candidates, usedNodeFlags, randomWeight);
			auto& candidate = candidates[index];

			addCandidates.emplace(candidate.Node);
			totalCandidateWeight -= candidate.Weight;
			usedNodeFlags[index] = true;
		}

		return addCandidates;
	}

	NodeSelectionResult SelectNodes(
			const ionet::NodeContainer& nodes,
			const NodeSelectionConfiguration& config,
			const ImportanceRetriever& importanceRetriever) {
		// 1. find compatible (service and role) nodes
		//    need to use node container view because node candidates are held by reference
		auto nodesView = nodes.view();
		auto findFilter = FindServiceNodesFilter{ config.ServiceId, config.SupportedProtocols, config.RequiredRole };
		auto nodesInfo = FindServiceNodes(nodesView, findFilter, importanceRetriever);
		auto numActiveNodes = nodesInfo.Actives.size();

		// 2. find removal candidates
		NodeSelectionResult result;
		result.RemoveCandidates = FindRemoveCandidates(nodesInfo.Actives, config.MaxConnections, config.MaxConnectionAge);
		numActiveNodes -= result.RemoveCandidates.size();

		// 3. find add candidates
		if (numActiveNodes < config.MaxConnections) {
			auto maxAddCandidates = config.MaxConnections - numActiveNodes;
			result.AddCandidates = SelectCandidatesBasedOnWeight(nodesInfo.Candidates, nodesInfo.TotalCandidateWeight, maxAddCandidates);
			numActiveNodes += result.AddCandidates.size();
		}

		return result;
	}

	model::NodeIdentitySet SelectNodesForRemoval(
			const ionet::NodeContainer& nodes,
			const NodeAgingConfiguration& config,
			const ImportanceRetriever& importanceRetriever) {
		// 1. find compatible (service) nodes; always match all roles
		//    need to use node container view because node candidates are held by reference
		auto nodesView = nodes.view();
		auto findFilter = FindServiceNodesFilter{ config.ServiceId, ionet::IpProtocol::All, ionet::NodeRoles::None };
		auto nodesInfo = FindServiceNodes(nodesView, findFilter, importanceRetriever);

		// 2. find removal candidates
		// a. allow at most 1/4 of active nodes to be disconnected
		// b. always retain at least one connection
		auto adjustedMaxConnections = std::max<uint32_t>(1, config.MaxConnections * 3 / 4);
		return FindRemoveCandidates(nodesInfo.Actives, adjustedMaxConnections, config.MaxConnectionAge);
	}
}}
