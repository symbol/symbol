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

#include "NodeContainer.h"
#include "NodeInteractionResult.h"
#include "catapult/utils/HexFormatter.h"

namespace catapult { namespace ionet { struct NodeData; } }

namespace catapult { namespace ionet {

	// region NodeData / NodeContainerData

	struct NodeData {
	public:
		NodeData(const Node& node, NodeSource source, size_t nodeId)
				: Node(node)
				, Info(source)
				, NodeId(nodeId)
		{}

	public:
		ionet::Node Node;
		NodeInfo Info;
		size_t NodeId;
	};

	struct NodeContainerData {
	public:
		NodeContainerData(size_t maxNodes, const supplier<Timestamp>& timeSupplier)
				: MaxNodes(maxNodes)
				, TimeSupplier(timeSupplier)
				, NextNodeId(1)
		{}

	public:
		const size_t MaxNodes;
		const supplier<Timestamp> TimeSupplier;
		size_t NextNodeId;
		std::unordered_map<Key, NodeData, utils::ArrayHasher<Key>> NodeDataContainer;
		std::vector<std::pair<ServiceIdentifier, ionet::NodeRoles>> ServiceRolesMap;
	};

	// endregion

	// region NodeContainerView

	NodeContainerView::NodeContainerView(
			const NodeContainerData& nodeContainerData,
			utils::SpinReaderWriterLock::ReaderLockGuard&& readLock)
			: m_nodeContainerData(nodeContainerData)
			, m_readLock(std::move(readLock))
	{}

	size_t NodeContainerView::size() const {
		return m_nodeContainerData.NodeDataContainer.size();
	}

	Timestamp NodeContainerView::time() const {
		return m_nodeContainerData.TimeSupplier();
	}

	bool NodeContainerView::contains(const Key& identityKey) const {
		return m_nodeContainerData.NodeDataContainer.cend() != m_nodeContainerData.NodeDataContainer.find(identityKey);
	}

	const NodeInfo& NodeContainerView::getNodeInfo(const Key& identityKey) const {
		auto iter = m_nodeContainerData.NodeDataContainer.find(identityKey);
		if (m_nodeContainerData.NodeDataContainer.cend() == iter)
			CATAPULT_THROW_INVALID_ARGUMENT_1("cannot get node info for unknown node", identityKey);

		return iter->second.Info;
	}

	void NodeContainerView::forEach(const consumer<const Node&, const NodeInfo&>& consumer) const {
		for (const auto& pair : m_nodeContainerData.NodeDataContainer)
			consumer(pair.second.Node, pair.second.Info);
	}

	// endregion

	// region NodeContainerModifier

	NodeContainerModifier::NodeContainerModifier(
			NodeContainerData& nodeContainerData,
			utils::SpinReaderWriterLock::ReaderLockGuard&& readLock)
			: m_nodeContainerData(nodeContainerData)
			, m_readLock(std::move(readLock))
			, m_writeLock(m_readLock.promoteToWriter())
	{}

	bool NodeContainerModifier::add(const Node& node, NodeSource source) {
		auto iter = m_nodeContainerData.NodeDataContainer.find(node.identityKey());
		if (m_nodeContainerData.NodeDataContainer.end() == iter) {
			if (!ensureAtLeastOneEmptySlot()) {
				CATAPULT_LOG(warning)
						<< "node container is full and no nodes are eligible for pruning"
						<< " (size = " << m_nodeContainerData.NodeDataContainer.size()
						<< ", max = " << m_nodeContainerData.MaxNodes << ")";
				return false;
			}

			auto emplaceResult = m_nodeContainerData.NodeDataContainer.emplace(
					node.identityKey(),
					NodeData(node, source, m_nodeContainerData.NextNodeId++));
			autoProvisionConnectionStates(emplaceResult.first->second);
			return true;
		}

		// if the source is no worse, update the node information
		auto& nodeData = iter->second;
		if (nodeData.Info.source() > source)
			return true;

		nodeData.Node = node;
		nodeData.Info.source(source);
		autoProvisionConnectionStates(nodeData);
		return true;
	}

	namespace {
		void ProvisionIfMatch(NodeData& nodeData, ServiceIdentifier serviceId, ionet::NodeRoles role) {
			if (HasFlag(role, nodeData.Node.metadata().Roles))
				nodeData.Info.provisionConnectionState(serviceId);
		}
	}

	void NodeContainerModifier::addConnectionStates(ServiceIdentifier serviceId, ionet::NodeRoles role) {
		for (auto& pair : m_nodeContainerData.NodeDataContainer)
			ProvisionIfMatch(pair.second, serviceId, role);

		// save mapping to automatically provision connection states for added nodes
		m_nodeContainerData.ServiceRolesMap.emplace_back(serviceId, role);
	}

	ConnectionState& NodeContainerModifier::provisionConnectionState(ServiceIdentifier serviceId, const Key& identityKey) {
		auto iter = m_nodeContainerData.NodeDataContainer.find(identityKey);
		if (m_nodeContainerData.NodeDataContainer.end() == iter)
			CATAPULT_THROW_INVALID_ARGUMENT_1("cannot provision connection state for unknown node", identityKey);

		return iter->second.Info.provisionConnectionState(serviceId);
	}

	void NodeContainerModifier::ageConnections(ServiceIdentifier serviceId, const utils::KeySet& identities) {
		for (auto& pair : m_nodeContainerData.NodeDataContainer) {
			auto& nodeInfo = pair.second.Info;
			if (identities.cend() != identities.find(pair.first))
				++nodeInfo.provisionConnectionState(serviceId).Age;
			else
				nodeInfo.clearAge(serviceId);
		}
	}

	void NodeContainerModifier::ageConnectionBans(
			ServiceIdentifier serviceId,
			uint32_t maxConnectionBanAge,
			uint32_t numConsecutiveFailuresBeforeBanning) {
		for (auto& pair : m_nodeContainerData.NodeDataContainer)
			pair.second.Info.updateBan(serviceId, maxConnectionBanAge, numConsecutiveFailuresBeforeBanning);
	}

	void NodeContainerModifier::incrementSuccesses(const Key& identityKey) {
		auto timestamp = m_nodeContainerData.TimeSupplier();
		incrementInteraction(identityKey, [timestamp](auto& info) { info.incrementSuccesses(timestamp); });
	}

	void NodeContainerModifier::incrementFailures(const Key& identityKey) {
		auto timestamp = m_nodeContainerData.TimeSupplier();
		incrementInteraction(identityKey, [timestamp](auto& info) { info.incrementFailures(timestamp); });
	}

	void NodeContainerModifier::autoProvisionConnectionStates(NodeData& nodeData) {
		for (const auto& pair : m_nodeContainerData.ServiceRolesMap)
			ProvisionIfMatch(nodeData, pair.first, pair.second);
	}

	bool NodeContainerModifier::ensureAtLeastOneEmptySlot() {
		auto& nodeContainer = m_nodeContainerData.NodeDataContainer;
		if (nodeContainer.size() < m_nodeContainerData.MaxNodes)
			return true;

		const NodeData* pMatchingNodeData = nullptr;
		for (const auto& pair : nodeContainer) {
			// only prune dynamic nodes that are inactive
			const auto& nodeInfo = pair.second.Info;
			if (nodeInfo.source() > NodeSource::Dynamic || nodeInfo.hasActiveConnection())
				continue;

			if (pMatchingNodeData) {
				// select the worst node; first tiebreaker is source, second is age (NodeId)
				auto isPreviouslyMatchingNodeBetter =
					pMatchingNodeData->Info.source() < nodeInfo.source()
					|| (pMatchingNodeData->Info.source() == nodeInfo.source() && pMatchingNodeData->NodeId < pair.second.NodeId);

				// if pMatchingNodeData is worse than the current node, don't update it
				if (isPreviouslyMatchingNodeBetter)
					continue;
			}

			pMatchingNodeData = &pair.second;
		}

		if (pMatchingNodeData) {
			nodeContainer.erase(pMatchingNodeData->Node.identityKey());
			return true;
		}

		return false;
	}

	void NodeContainerModifier::incrementInteraction(const Key& identityKey, const consumer<NodeInfo&>& incrementer) {
		auto iter = m_nodeContainerData.NodeDataContainer.find(identityKey);
		if (m_nodeContainerData.NodeDataContainer.cend() == iter)
			return;

		incrementer(iter->second.Info);
	}

	// endregion

	// region NodeContainer

	NodeContainer::NodeContainer() : NodeContainer(std::numeric_limits<size_t>::max(), []() { return Timestamp(0); })
	{}

	NodeContainer::NodeContainer(size_t maxNodes, const supplier<Timestamp>& timeSupplier)
			: m_pImpl(std::make_unique<NodeContainerData>(maxNodes, timeSupplier))
	{}

	NodeContainer::~NodeContainer() = default;

	NodeContainerView NodeContainer::view() const {
		auto readLock = m_lock.acquireReader();
		return NodeContainerView(*m_pImpl, std::move(readLock));
	}

	NodeContainerModifier NodeContainer::modifier() {
		auto readLock = m_lock.acquireReader();
		return NodeContainerModifier(*m_pImpl, std::move(readLock));
	}

	// endregion

	// region utils

	NodeSet FindAllActiveNodes(const NodeContainerView& view) {
		NodeSet activeNodes;
		view.forEach([&activeNodes](const auto& node, const auto& nodeInfo) {
			if (nodeInfo.hasActiveConnection())
				activeNodes.emplace(node);
		});

		return activeNodes;
	}

	// endregion
}}
