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
#include "NodeDataContainer.h"
#include "NodeInteractionResult.h"
#include "catapult/utils/HexFormatter.h"

namespace catapult { namespace ionet { struct NodeData; } }

namespace catapult { namespace ionet {

	// region NodeContainerData

	struct NodeContainerData {
	public:
		NodeContainerData(
				size_t maxNodes,
				model::NodeIdentityEqualityStrategy equalityStrategy,
				const supplier<Timestamp>& timeSupplier,
				const predicate<NodeVersion>& versionPredicate)
				: MaxNodes(maxNodes)
				, TimeSupplier(timeSupplier)
				, VersionPredicate(versionPredicate)
				, NextNodeId(1)
				, NodeDataContainer(equalityStrategy)
		{}

	public:
		const size_t MaxNodes;
		const supplier<Timestamp> TimeSupplier;
		const predicate<NodeVersion> VersionPredicate;

		size_t NextNodeId;
		ionet::NodeDataContainer NodeDataContainer;
		std::vector<std::pair<ServiceIdentifier, ionet::NodeRoles>> ServiceRolesMap;
	};

	// endregion

	// region NodeContainerView

	NodeContainerView::NodeContainerView(
			const NodeContainerData& nodeContainerData,
			const BannedNodes& bannedNodes,
			utils::SpinReaderWriterLock::ReaderLockGuard&& readLock)
			: m_nodeContainerData(nodeContainerData)
			, m_bannedNodes(bannedNodes)
			, m_readLock(std::move(readLock))
	{}

	size_t NodeContainerView::size() const {
		return m_nodeContainerData.NodeDataContainer.size();
	}

	size_t NodeContainerView::bannedNodesSize() const {
		return m_bannedNodes.size();
	}

	size_t NodeContainerView::bannedNodesDeepSize() const {
		return m_bannedNodes.deepSize();
	}

	Timestamp NodeContainerView::time() const {
		return m_nodeContainerData.TimeSupplier();
	}

	bool NodeContainerView::contains(const model::NodeIdentity& identity) const {
		return !!m_nodeContainerData.NodeDataContainer.tryGet(identity);
	}

	const NodeInfo& NodeContainerView::getNodeInfo(const model::NodeIdentity& identity) const {
		const auto* pNodeData = m_nodeContainerData.NodeDataContainer.tryGet(identity);
		if (!pNodeData)
			CATAPULT_THROW_INVALID_ARGUMENT_1("cannot get node info for unknown identity", identity);

		return pNodeData->NodeInfo;
	}

	bool NodeContainerView::isBanned(const model::NodeIdentity& identity) const {
		return m_bannedNodes.isBanned(identity);
	}

	void NodeContainerView::forEach(const consumer<const Node&, const NodeInfo&>& consumer) const {
		m_nodeContainerData.NodeDataContainer.forEach([&bannedNodes = m_bannedNodes, consumer](const auto& node, const auto& nodeInfo) {
			if (!bannedNodes.isBanned(node.identity()))
				consumer(node, nodeInfo);
		});
	}

	// endregion

	// region NodeContainerModifier

	NodeContainerModifier::NodeContainerModifier(
			NodeContainerData& nodeContainerData,
			BannedNodes& bannedNodes,
			utils::SpinReaderWriterLock::WriterLockGuard&& writeLock)
			: m_nodeContainerData(nodeContainerData)
			, m_bannedNodes(bannedNodes)
			, m_writeLock(std::move(writeLock))
	{}

	bool NodeContainerModifier::add(const Node& node, NodeSource source) {
		if (!m_nodeContainerData.VersionPredicate(node.metadata().Version))
			return false;

		auto& nodeDataContainer = m_nodeContainerData.NodeDataContainer;
		if (m_bannedNodes.isBanned(node.identity()))
			return false;

		auto prepareInsertResultPair = nodeDataContainer.prepareInsert(node.identity(), source);
		auto* pNodeData = prepareInsertResultPair.first;
		if (NodeDataContainer::PrepareInsertCode::Allowed != prepareInsertResultPair.second)
			return NodeDataContainer::PrepareInsertCode::Conflict != prepareInsertResultPair.second;

		if (!pNodeData) {
			if (!ensureAtLeastOneEmptySlot()) {
				CATAPULT_LOG(warning)
						<< "node container is full and no nodes are eligible for pruning"
						<< " (size = " << nodeDataContainer.size()
						<< ", max = " << m_nodeContainerData.MaxNodes << ")";
				return false;
			}

			pNodeData = nodeDataContainer.insert(NodeData(node, source, m_nodeContainerData.NextNodeId++));
			autoProvisionConnectionStates(*pNodeData);
			return true;
		}

		if (!model::NodeIdentityEquality(model::NodeIdentityEqualityStrategy::Key_And_Host)(pNodeData->Node.identity(), node.identity())) {
			auto nodeDataCopy = *pNodeData;
			nodeDataContainer.erase(pNodeData->Node.identity());

			// need to set Node before inserting so that nodeDataContainer maps are correct
			nodeDataCopy.Node = node;
			pNodeData = nodeDataContainer.insert(nodeDataCopy);
		} else {
			// safe update because identity is strictly equal
			pNodeData->Node = node;
		}

		pNodeData->NodeInfo.source(source);
		autoProvisionConnectionStates(*pNodeData);
		return true;
	}

	namespace {
		void ProvisionIfMatch(NodeData& nodeData, ServiceIdentifier serviceId, ionet::NodeRoles role) {
			if (HasFlag(role, nodeData.Node.metadata().Roles))
				nodeData.NodeInfo.provisionConnectionState(serviceId);
		}
	}

	void NodeContainerModifier::addConnectionStates(ServiceIdentifier serviceId, ionet::NodeRoles role) {
		m_nodeContainerData.NodeDataContainer.forEach([serviceId, role](auto& nodeData) {
			ProvisionIfMatch(nodeData, serviceId, role);
		});

		// save mapping to automatically provision connection states for added nodes
		m_nodeContainerData.ServiceRolesMap.emplace_back(serviceId, role);
	}

	ConnectionState& NodeContainerModifier::provisionConnectionState(ServiceIdentifier serviceId, const model::NodeIdentity& identity) {
		auto* pNodeData = m_nodeContainerData.NodeDataContainer.tryGet(identity);
		if (!pNodeData)
			CATAPULT_THROW_INVALID_ARGUMENT_1("cannot provision connection state for unknown node", identity);

		return pNodeData->NodeInfo.provisionConnectionState(serviceId);
	}

	void NodeContainerModifier::ageConnections(ServiceIdentifier serviceId, const model::NodeIdentitySet& identities) {
		m_nodeContainerData.NodeDataContainer.forEach([serviceId, &identities](auto& nodeData) {
			if (identities.cend() != identities.find(nodeData.Node.identity()))
				++nodeData.NodeInfo.provisionConnectionState(serviceId).Age;
			else
				nodeData.NodeInfo.clearAge(serviceId);
		});
	}

	void NodeContainerModifier::ageConnectionBans(
			ServiceIdentifier serviceId,
			uint32_t maxConnectionBanAge,
			uint32_t numConsecutiveFailuresBeforeBanning) {
		m_nodeContainerData.NodeDataContainer.forEach([serviceId, maxConnectionBanAge, numConsecutiveFailuresBeforeBanning](
				auto& nodeData) {
			nodeData.NodeInfo.updateBan(serviceId, maxConnectionBanAge, numConsecutiveFailuresBeforeBanning);
		});
	}

	void NodeContainerModifier::incrementSuccesses(const model::NodeIdentity& identity) {
		auto timestamp = m_nodeContainerData.TimeSupplier();
		incrementInteraction(identity, [timestamp](auto& nodeInfo) { nodeInfo.incrementSuccesses(timestamp); });
	}

	void NodeContainerModifier::incrementFailures(const model::NodeIdentity& identity) {
		auto timestamp = m_nodeContainerData.TimeSupplier();
		incrementInteraction(identity, [timestamp](auto& nodeInfo) { nodeInfo.incrementFailures(timestamp); });
	}

	void NodeContainerModifier::ban(const model::NodeIdentity& identity, uint32_t reason) {
		m_bannedNodes.add(identity, reason);
	}

	void NodeContainerModifier::pruneBannedNodes() {
		m_bannedNodes.prune();
	}

	void NodeContainerModifier::autoProvisionConnectionStates(NodeData& nodeData) {
		for (const auto& pair : m_nodeContainerData.ServiceRolesMap)
			ProvisionIfMatch(nodeData, pair.first, pair.second);
	}

	bool NodeContainerModifier::ensureAtLeastOneEmptySlot() {
		auto& nodeDataContainer = m_nodeContainerData.NodeDataContainer;
		if (nodeDataContainer.size() < m_nodeContainerData.MaxNodes)
			return true;

		const auto* pWorstNodeData = nodeDataContainer.tryFindWorst();
		if (pWorstNodeData) {
			nodeDataContainer.erase(pWorstNodeData->Node.identity());
			return true;
		}

		return false;
	}

	void NodeContainerModifier::incrementInteraction(const model::NodeIdentity& identity, const consumer<NodeInfo&>& incrementer) {
		auto* pNodeData = m_nodeContainerData.NodeDataContainer.tryGet(identity);
		if (!pNodeData)
			return;

		incrementer(pNodeData->NodeInfo);
	}

	// endregion

	// region NodeContainer

	NodeContainer::NodeContainer()
			: NodeContainer(
					std::numeric_limits<size_t>::max(),
					model::NodeIdentityEqualityStrategy::Key_And_Host,
					BanSettings(),
					[]() { return Timestamp(0); },
					[](auto) { return true; })
	{}

	NodeContainer::NodeContainer(
			size_t maxNodes,
			model::NodeIdentityEqualityStrategy equalityStrategy,
			const BanSettings& banSettings,
			const supplier<Timestamp>& timeSupplier,
			const predicate<NodeVersion>& versionPredicate)
			: m_pImpl(std::make_unique<NodeContainerData>(maxNodes, equalityStrategy, timeSupplier, versionPredicate))
			, m_bannedNodes(banSettings, timeSupplier, equalityStrategy)
	{}

	NodeContainer::~NodeContainer() = default;

	NodeContainerView NodeContainer::view() const {
		auto readLock = m_lock.acquireReader();
		return NodeContainerView(*m_pImpl, m_bannedNodes, std::move(readLock));
	}

	NodeContainerModifier NodeContainer::modifier() {
		auto writeLock = m_lock.acquireWriter();
		return NodeContainerModifier(*m_pImpl, m_bannedNodes, std::move(writeLock));
	}

	// endregion

	// region utils

	predicate<NodeVersion> CreateRangeNodeVersionPredicate(NodeVersion minVersion, NodeVersion maxVersion) {
		return [minVersion, maxVersion](auto version) {
			return minVersion <= version && version <= maxVersion;
		};
	}

	NodeSet FindAllActiveNodes(const NodeContainerView& view) {
		return FindAllActiveNodes(view, [](auto) { return true; });
	}

	NodeSet FindAllActiveNodes(const NodeContainerView& view, const predicate<NodeSource>& includePredicate) {
		NodeSet activeNodes;
		view.forEach([includePredicate, &activeNodes](const auto& node, const auto& nodeInfo) {
			if (nodeInfo.hasActiveConnection() && includePredicate(nodeInfo.source()))
				activeNodes.emplace(node);
		});

		return activeNodes;
	}

	// endregion
}}
