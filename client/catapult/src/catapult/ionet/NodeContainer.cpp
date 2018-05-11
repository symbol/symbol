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
#include "catapult/utils/HexFormatter.h"

namespace catapult { namespace ionet { struct NodeData; } }

namespace catapult { namespace ionet {

	// region NodeData

	struct NodeData {
	public:
		explicit NodeData(const Node& node, NodeSource source)
				: Node(node)
				, Info(source)
		{}

	public:
		ionet::Node Node;
		NodeInfo Info;
	};

	// endregion

	// region NodeContainerView

	NodeContainerView::NodeContainerView(
			const NodeDataContainer& nodeDataContainer,
			utils::SpinReaderWriterLock::ReaderLockGuard&& readLock)
			: m_nodeDataContainer(nodeDataContainer)
			, m_readLock(std::move(readLock))
	{}

	size_t NodeContainerView::size() const {
		return m_nodeDataContainer.size();
	}

	bool NodeContainerView::contains(const Key& identityKey) const {
		return m_nodeDataContainer.cend() != m_nodeDataContainer.find(identityKey);
	}

	const NodeInfo& NodeContainerView::getNodeInfo(const Key& identityKey) const {
		auto iter = m_nodeDataContainer.find(identityKey);
		if (m_nodeDataContainer.cend() == iter)
			CATAPULT_THROW_INVALID_ARGUMENT_1("cannot get node info for unknown node", utils::HexFormat(identityKey));

		return iter->second.Info;
	}

	void NodeContainerView::forEach(const consumer<const Node&, const NodeInfo&>& consumer) const {
		for (const auto& pair : m_nodeDataContainer)
			consumer(pair.second.Node, pair.second.Info);
	}

	// endregion

	// region NodeContainerModifier

	NodeContainerModifier::NodeContainerModifier(
			NodeDataContainer& nodeDataContainer,
			ServiceRolesMap& serviceRolesMap,
			utils::SpinReaderWriterLock::ReaderLockGuard&& readLock)
			: m_nodeDataContainer(nodeDataContainer)
			, m_serviceRolesMap(serviceRolesMap)
			, m_readLock(std::move(readLock))
			, m_writeLock(m_readLock.promoteToWriter())
	{}

	void NodeContainerModifier::add(const Node& node, NodeSource source) {
		auto iter = m_nodeDataContainer.find(node.identityKey());
		if (m_nodeDataContainer.end() == iter) {
			auto emplaceResult = m_nodeDataContainer.emplace(node.identityKey(), NodeData(node, source));
			autoProvisionConnectionStates(emplaceResult.first->second);
			return;
		}

		// if the source is no worse, update the node information
		auto& data = iter->second;
		if (data.Info.source() > source)
			return;

		data.Node = node;
		data.Info.source(source);
		autoProvisionConnectionStates(data);
	}

	namespace {
		void ProvisionIfMatch(NodeData& data, ServiceIdentifier serviceId, ionet::NodeRoles role) {
			if (HasFlag(role, data.Node.metadata().Roles))
				data.Info.provisionConnectionState(serviceId);
		}
	}

	void NodeContainerModifier::addConnectionStates(ServiceIdentifier serviceId, ionet::NodeRoles role) {
		for (auto& pair : m_nodeDataContainer)
			ProvisionIfMatch(pair.second, serviceId, role);

		// save mapping to automatically provision connection states for added nodes
		m_serviceRolesMap.emplace_back(serviceId, role);
	}

	ConnectionState& NodeContainerModifier::provisionConnectionState(ServiceIdentifier serviceId, const Key& identityKey) {
		auto iter = m_nodeDataContainer.find(identityKey);
		if (m_nodeDataContainer.end() == iter)
			CATAPULT_THROW_INVALID_ARGUMENT_1("cannot provision connection state for unknown node", utils::HexFormat(identityKey));

		return iter->second.Info.provisionConnectionState(serviceId);
	}

	void NodeContainerModifier::ageConnections(ServiceIdentifier serviceId, const utils::KeySet& identities) {
		for (auto& pair : m_nodeDataContainer) {
			auto& nodeInfo = pair.second.Info;
			if (identities.cend() != identities.find(pair.first))
				++nodeInfo.provisionConnectionState(serviceId).Age;
			else
				nodeInfo.clearAge(serviceId);
		}
	}

	void NodeContainerModifier::autoProvisionConnectionStates(NodeData& data) {
		for (const auto& pair : m_serviceRolesMap)
			ProvisionIfMatch(data, pair.first, pair.second);
	}

	// endregion

	// region NodeContainer

	struct NodeContainer::Impl {
		ionet::NodeDataContainer NodeDataContainer;
		NodeContainerModifier::ServiceRolesMap ServiceRolesMap;
	};

	NodeContainer::NodeContainer() : m_pImpl(std::make_unique<Impl>())
	{}

	NodeContainer::~NodeContainer() = default;

	NodeContainerView NodeContainer::view() const {
		return NodeContainerView(m_pImpl->NodeDataContainer, m_lock.acquireReader());
	}

	NodeContainerModifier NodeContainer::modifier() {
		return NodeContainerModifier(m_pImpl->NodeDataContainer, m_pImpl->ServiceRolesMap, m_lock.acquireReader());
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
