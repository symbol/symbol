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

namespace catapult {
	namespace ionet {
		struct NodeContainerData;
		struct NodeData;
		struct NodeInteractionResult;
	}
}

namespace catapult { namespace ionet {

	/// A read only view on top of node container.
	class NodeContainerView : utils::MoveOnly {
	public:
		/// Creates a view around \a nodeContainerData with lock context \a readLock.
		NodeContainerView(const NodeContainerData& nodeContainerData, utils::SpinReaderWriterLock::ReaderLockGuard&& readLock);

	public:
		/// Returns the number of nodes.
		size_t size() const;

		/// Gets current container time.
		Timestamp time() const;

		/// Returns \c true if the node with \a identityKey is in the container, \c false otherwise.
		bool contains(const Key& identityKey) const;

		/// Gets node info for the node with \a identityKey.
		const NodeInfo& getNodeInfo(const Key& identityKey) const;

		/// Iterates over all nodes and passes them to \a consumer.
		void forEach(const consumer<const Node&, const NodeInfo&>& consumer) const;

	private:
		const NodeContainerData& m_nodeContainerData;
		utils::SpinReaderWriterLock::ReaderLockGuard m_readLock;
	};

	/// A write only view on top of node container.
	class NodeContainerModifier : utils::MoveOnly {
	public:
		/// Creates a view around \a nodeContainerData with lock context \a readLock.
		NodeContainerModifier(NodeContainerData& nodeContainerData, utils::SpinReaderWriterLock::ReaderLockGuard&& readLock);

	public:
		/// Adds \a node to the collection with \a source.
		/// \note Node sources can be promoted but never demoted.
		bool add(const Node& node, NodeSource source);

		/// Adds connection states for the service identified by \a serviceId to all nodes with \a role.
		void addConnectionStates(ServiceIdentifier serviceId, ionet::NodeRoles role);

		/// Gets connection state for the service identified by \a serviceId and the node with \a identityKey.
		ConnectionState& provisionConnectionState(ServiceIdentifier serviceId, const Key& identityKey);

		/// Ages all connections for the service identified by \a serviceId for nodes with \a identities.
		void ageConnections(ServiceIdentifier serviceId, const utils::KeySet& identities);

		/// Ages all connection bans for the service identified by \a serviceId given \a maxConnectionBanAge and
		/// \a numConsecutiveFailuresBeforeBanning.
		void ageConnectionBans(ServiceIdentifier serviceId, uint32_t maxConnectionBanAge, uint32_t numConsecutiveFailuresBeforeBanning);

		/// Increments the number of successful interactions for the node identified by \a identityKey.
		void incrementSuccesses(const Key& identityKey);

		/// Increments the number of failed interactions for the node identified by \a identityKey.
		void incrementFailures(const Key& identityKey);

	private:
		void autoProvisionConnectionStates(NodeData& nodeData);

		bool ensureAtLeastOneEmptySlot();

		void incrementInteraction(const Key& identityKey, const consumer<NodeInfo&>& incrementer);

	private:
		NodeContainerData& m_nodeContainerData;
		utils::SpinReaderWriterLock::ReaderLockGuard m_readLock;
		utils::SpinReaderWriterLock::WriterLockGuard m_writeLock;
	};

	/// A collection of nodes.
	class NodeContainer {
	public:
		/// Creates a node container.
		NodeContainer();

		/// Creates a node container that can contain at most \a maxNodes nodes around a custom time supplier (\a timeSupplier).
		NodeContainer(size_t maxNodes, const supplier<Timestamp>& timeSupplier);

		/// Destroys a node container.
		~NodeContainer();

	public:
		/// Gets a read only view of the nodes.
		NodeContainerView view() const;

		/// Gets a write only view of the nodes.
		NodeContainerModifier modifier();

	private:
		std::unique_ptr<NodeContainerData> m_pImpl;
		mutable utils::SpinReaderWriterLock m_lock;
	};

	/// Finds all active nodes in \a view.
	NodeSet FindAllActiveNodes(const NodeContainerView& view);
}}
