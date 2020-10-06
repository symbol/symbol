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
#include "BannedNodes.h"
#include "NodeInfo.h"
#include "NodeSet.h"
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

	/// Read only view on top of node container.
	class NodeContainerView : utils::MoveOnly {
	public:
		/// Creates a view around \a nodeContainerData and \a bannedNodes with lock context \a readLock.
		NodeContainerView(
				const NodeContainerData& nodeContainerData,
				const BannedNodes& bannedNodes,
				utils::SpinReaderWriterLock::ReaderLockGuard&& readLock);

	public:
		/// Number of nodes.
		size_t size() const;

		/// Number of actively banned nodes.
		size_t bannedNodesSize() const;

		/// Number of total nodes in ban container.
		size_t bannedNodesDeepSize() const;

		/// Gets the current container time.
		Timestamp time() const;

		/// Returns \c true if the node with \a identity is in the container, \c false otherwise.
		bool contains(const model::NodeIdentity& identity) const;

		/// Gets the node info for the node with \a identity.
		const NodeInfo& getNodeInfo(const model::NodeIdentity& identity) const;

		/// Returns \c true if \a identity is banned.
		bool isBanned(const model::NodeIdentity& identity) const;

		/// Iterates over all nodes and passes unbanned nodes to \a consumer.
		void forEach(const consumer<const Node&, const NodeInfo&>& consumer) const;

	private:
		const NodeContainerData& m_nodeContainerData;
		const BannedNodes& m_bannedNodes;
		utils::SpinReaderWriterLock::ReaderLockGuard m_readLock;
	};

	/// Write only view on top of node container.
	class NodeContainerModifier : utils::MoveOnly {
	public:
		/// Creates a view around \a nodeContainerData and \a bannedNodes with lock context \a writeLock.
		NodeContainerModifier(
				NodeContainerData& nodeContainerData,
				BannedNodes& bannedNodes,
				utils::SpinReaderWriterLock::WriterLockGuard&& writeLock);

	public:
		/// Adds \a node to the collection with \a source.
		/// \note Node sources can be promoted but never demoted.
		bool add(const Node& node, NodeSource source);

		/// Adds connection states for the service identified by \a serviceId to all nodes with \a role.
		void addConnectionStates(ServiceIdentifier serviceId, ionet::NodeRoles role);

		/// Gets the connection state for the service identified by \a serviceId and the node with \a identity.
		ConnectionState& provisionConnectionState(ServiceIdentifier serviceId, const model::NodeIdentity& identity);

		/// Ages all connections for the service identified by \a serviceId for nodes with \a identities.
		void ageConnections(ServiceIdentifier serviceId, const model::NodeIdentitySet& identities);

		/// Ages all connection bans for the service identified by \a serviceId given \a maxConnectionBanAge and
		/// \a numConsecutiveFailuresBeforeBanning.
		void ageConnectionBans(ServiceIdentifier serviceId, uint32_t maxConnectionBanAge, uint32_t numConsecutiveFailuresBeforeBanning);

		/// Increments the number of successful interactions for the node identified by \a identity.
		void incrementSuccesses(const model::NodeIdentity& identity);

		/// Increments the number of failed interactions for the node identified by \a identity.
		void incrementFailures(const model::NodeIdentity& identity);

		/// Bans \a identity due to \a reason.
		void ban(const model::NodeIdentity& identity, uint32_t reason);

		/// Prunes banned nodes.
		void pruneBannedNodes();

	private:
		void autoProvisionConnectionStates(NodeData& nodeData);

		bool ensureAtLeastOneEmptySlot();

		void incrementInteraction(const model::NodeIdentity& identity, const consumer<NodeInfo&>& incrementer);

	private:
		NodeContainerData& m_nodeContainerData;
		BannedNodes& m_bannedNodes;
		utils::SpinReaderWriterLock::WriterLockGuard m_writeLock;
	};

	/// Container of nodes.
	class NodeContainer {
	public:
		/// Creates a node container.
		NodeContainer();

		/// Creates a node container that can contain at most \a maxNodes nodes with specified equality strategy (\a equalityStrategy)
		/// around a custom time supplier (\a timeSupplier) and node version predicate (\a versionPredicate).
		/// Node banning is supported and configured by \a banSettings.
		NodeContainer(
				size_t maxNodes,
				model::NodeIdentityEqualityStrategy equalityStrategy,
				const BanSettings& banSettings,
				const supplier<Timestamp>& timeSupplier,
				const predicate<NodeVersion>& versionPredicate);

		/// Destroys a node container.
		~NodeContainer();

	public:
		/// Gets a read only view of the nodes.
		NodeContainerView view() const;

		/// Gets a write only view of the nodes.
		NodeContainerModifier modifier();

	private:
		std::unique_ptr<NodeContainerData> m_pImpl;
		BannedNodes m_bannedNodes;
		mutable utils::SpinReaderWriterLock m_lock;
	};

	/// Creates a node version predicate that returns \c true when version is within \a minVersion and \a maxVersion, inclusive.
	predicate<NodeVersion> CreateRangeNodeVersionPredicate(NodeVersion minVersion, NodeVersion maxVersion);

	/// Finds all active nodes in \a view.
	NodeSet FindAllActiveNodes(const NodeContainerView& view);

	/// Finds all active nodes in \a view that pass \a includePredicate.
	NodeSet FindAllActiveNodes(const NodeContainerView& view, const predicate<NodeSource>& includePredicate);
}}
