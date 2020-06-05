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

#include "NodeDataContainer.h"

namespace catapult { namespace ionet {

	NodeDataContainer::NodeDataContainer(model::NodeIdentityEqualityStrategy equalityStrategy)
			: m_equalityStrategy(equalityStrategy)
			, m_nodeMap(model::CreateNodeIdentityMap<NodeData>(m_equalityStrategy))
	{}

	size_t NodeDataContainer::size() const {
		return m_nodeMap.size();
	}

	const NodeData* NodeDataContainer::tryGet(const model::NodeIdentity& identity) const {
		auto iter = m_nodeMap.find(identity);
		return m_nodeMap.cend() == iter ? nullptr : &iter->second;
	}

	NodeData* NodeDataContainer::tryGet(const model::NodeIdentity& identity) {
		auto iter = m_nodeMap.find(identity);
		return m_nodeMap.end() == iter ? nullptr : &iter->second;
	}

	void NodeDataContainer::forEach(const consumer<const Node&, const NodeInfo&>& consumer) const {
		for (const auto& pair : m_nodeMap)
			consumer(pair.second.Node, pair.second.NodeInfo);
	}

	void NodeDataContainer::forEach(const consumer<NodeData&>& consumer) {
		for (auto& pair : m_nodeMap)
			consumer(pair.second);
	}

	namespace {
		enum class CanUpdateIdentityResult { Equal, Allowed, Disallowed };

		bool IsSourceDowngrade(const NodeData& nodeData, NodeSource source) {
			return nodeData.NodeInfo.source() > source;
		}

		CanUpdateIdentityResult CanUpdateIdentity(NodeData& nodeData, const model::NodeIdentity& newIdentity, NodeSource newSource) {
			auto strictIdentityEquality = model::NodeIdentityEquality(model::NodeIdentityEqualityStrategy::Key_And_Host);
			if (strictIdentityEquality(newIdentity, nodeData.Node.identity()))
				return CanUpdateIdentityResult::Equal;

			auto canUpdate = NodeSource::Local != nodeData.NodeInfo.source();
			auto isSourceDowngrade = IsSourceDowngrade(nodeData, newSource);
			if (canUpdate) {
				if (!nodeData.NodeInfo.hasActiveConnection()) {
					// if there are no active connections, identity updates are always allowed
					// if there is a downgrade, set the HasIdentityUpdateInProgress flag to allow the next update
					if (isSourceDowngrade)
						nodeData.HasIdentityUpdateInProgress = true;
				} else {
					// if there are active connections and the HasIdentityUpdateInProgress flag is set,
					// allow the update if and only if it can be committed
					canUpdate = nodeData.HasIdentityUpdateInProgress && !isSourceDowngrade;
					nodeData.HasIdentityUpdateInProgress = false;
				}
			}

			if (canUpdate)
				return CanUpdateIdentityResult::Allowed;

			CATAPULT_LOG(warning)
					<< "rejecting new host (" << newIdentity
					<< ") with in use identity key (" << nodeData.Node.identity() << ")";
			return CanUpdateIdentityResult::Disallowed;
		}
	}

	std::pair<NodeData*, NodeDataContainer::PrepareInsertCode> NodeDataContainer::prepareInsert(
			const model::NodeIdentity& identity,
			NodeSource source) {
		auto* pNodeData = tryGet(identity);

		if (pNodeData && CanUpdateIdentityResult::Disallowed == CanUpdateIdentity(*pNodeData, identity, source))
			return std::make_pair(nullptr, PrepareInsertCode::Conflict);

		if (model::NodeIdentityEqualityStrategy::Host == m_equalityStrategy) {
			auto keyHostIter = m_keyHostMap.find(identity.PublicKey);
			if (m_keyHostMap.cend() != keyHostIter) {
				auto keyHostNodeIdentity = model::NodeIdentity{ keyHostIter->first, keyHostIter->second };
				auto nodeIter = m_nodeMap.find(keyHostNodeIdentity);
				auto canUpdateIdentityResult = CanUpdateIdentity(nodeIter->second, identity, source);

				if (CanUpdateIdentityResult::Equal != canUpdateIdentityResult) {
					// fail if migration is disallowed
					if (CanUpdateIdentityResult::Disallowed == canUpdateIdentityResult)
						return std::make_pair(nullptr, PrepareInsertCode::Conflict);

					// if an allowable identity change is detected, but it can't be committed, short-circuit before updating container
					if (IsSourceDowngrade(nodeIter->second, source))
						return std::make_pair(pNodeData, pNodeData ? PrepareInsertCode::Redundant : PrepareInsertCode::Allowed);

					if (!pNodeData) {
						// host is not previously seen but key is, so migrate data associated with key to new host
						pNodeData = &nodeIter->second;
					} else {
						// both host and key have been previously seen (separately), so use data associated with host
						m_keyHostMap.erase(keyHostIter);
						m_nodeMap.erase(nodeIter);
					}
				}
			}
		}

		if (pNodeData && IsSourceDowngrade(*pNodeData, source))
			return std::make_pair(pNodeData, PrepareInsertCode::Redundant);

		return std::make_pair(pNodeData, PrepareInsertCode::Allowed);
	}

	NodeData* NodeDataContainer::insert(const NodeData& nodeData) {
		const auto& identity = nodeData.Node.identity();
		if (model::NodeIdentityEqualityStrategy::Host == m_equalityStrategy)
			m_keyHostMap.emplace(identity.PublicKey, identity.Host);

		auto* pNewNodeData = &m_nodeMap.emplace(identity, nodeData).first->second;
		pNewNodeData->HasIdentityUpdateInProgress = false;
		return pNewNodeData;
	}

	void NodeDataContainer::erase(const model::NodeIdentity& identity) {
		if (model::NodeIdentityEqualityStrategy::Host == m_equalityStrategy)
			m_keyHostMap.erase(identity.PublicKey);

		m_nodeMap.erase(identity);
	}

	const NodeData* NodeDataContainer::tryFindWorst() const {
		const NodeData* pWorstNodeData = nullptr;
		for (const auto& pair : m_nodeMap) {
			// only prune dynamic nodes that are inactive
			const auto& nodeInfo = pair.second.NodeInfo;
			if (nodeInfo.source() > NodeSource::Dynamic || nodeInfo.hasActiveConnection())
				continue;

			if (pWorstNodeData) {
				// select the worst node - first tiebreaker is source, second is age (NodeId)
				if (pWorstNodeData->NodeInfo.source() < nodeInfo.source())
					continue;

				if (pWorstNodeData->NodeInfo.source() == nodeInfo.source() && pWorstNodeData->NodeId < pair.second.NodeId)
					continue;

				// pair.second is worse than pWorstNodeData
			}

			pWorstNodeData = &pair.second;
		}

		return pWorstNodeData;
	}
}}
