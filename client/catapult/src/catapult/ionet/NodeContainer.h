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
#include "Node.h"
#include "NodeInfo.h"
#include "catapult/utils/ArraySet.h"
#include "catapult/utils/SpinReaderWriterLock.h"
#include <unordered_map>

namespace catapult { namespace ionet { struct NodeData; } }

namespace catapult { namespace ionet {

	/// Internal container wrapped by NodeContainer.
	using NodeDataContainer = std::unordered_map<Key, NodeData, utils::ArrayHasher<Key>>;

	/// A read only view on top of node container.
	class NodeContainerView : utils::MoveOnly {
	public:
		/// Creates a view around \a nodeDataContainer with lock context \a readLock.
		NodeContainerView(const NodeDataContainer& nodeDataContainer, utils::SpinReaderWriterLock::ReaderLockGuard&& readLock);

	public:
		/// Returns the number of nodes.
		size_t size() const;

		/// Returns \c true if the node with \a identityKey is in the container, \c false otherwise.
		bool contains(const Key& identityKey) const;

		/// Gets node info for the node with \a identityKey.
		const NodeInfo& getNodeInfo(const Key& identityKey) const;

		/// Iterates over all nodes and passes them to \a consumer.
		void forEach(const consumer<const Node&, const NodeInfo&>& consumer) const;

	private:
		const NodeDataContainer& m_nodeDataContainer;
		utils::SpinReaderWriterLock::ReaderLockGuard m_readLock;
	};

	/// A write only view on top of node container.
	class NodeContainerModifier : utils::MoveOnly {
	public:
		/// A map of service identifiers to node roles.
		using ServiceRolesMap = std::vector<std::pair<ServiceIdentifier, ionet::NodeRoles>>;

	public:
		/// Creates a view around \a nodeDataContainer and \a serviceRolesMap with lock context \a readLock.
		NodeContainerModifier(
				NodeDataContainer& nodeDataContainer,
				ServiceRolesMap& serviceRolesMap,
				utils::SpinReaderWriterLock::ReaderLockGuard&& readLock);

	public:
		/// Adds a \a node to the collection with \a source.
		/// \note Node sources can be promoted but never demoted.
		void add(const Node& node, NodeSource source);

		/// Adds connection states for the service identified by \a serviceId to all nodes with \a role.
		void addConnectionStates(ServiceIdentifier serviceId, ionet::NodeRoles role);

		/// Gets connection state for the service identified by \a serviceId and the node with \a identityKey.
		ConnectionState& provisionConnectionState(ServiceIdentifier serviceId, const Key& identityKey);

		/// Ages all connections for the service identified by \a serviceId for nodes with \a identities.
		void ageConnections(ServiceIdentifier serviceId, const utils::KeySet& identities);

	private:
		void autoProvisionConnectionStates(NodeData& data);

	private:
		NodeDataContainer& m_nodeDataContainer;
		ServiceRolesMap& m_serviceRolesMap;
		utils::SpinReaderWriterLock::ReaderLockGuard m_readLock;
		utils::SpinReaderWriterLock::WriterLockGuard m_writeLock;
	};

	/// A collection of nodes.
	class NodeContainer {
	public:
		/// Creates a node container.
		NodeContainer();

		/// Destroys a node container.
		~NodeContainer();

	public:
		/// Gets a read only view of the nodes.
		NodeContainerView view() const;

		/// Gets a write only view of the nodes.
		NodeContainerModifier modifier();

	private:
		struct Impl;

	private:
		std::unique_ptr<Impl> m_pImpl;
		mutable utils::SpinReaderWriterLock m_lock;
	};

	/// Finds all active nodes in \a view.
	NodeSet FindAllActiveNodes(const NodeContainerView& view);
}}
