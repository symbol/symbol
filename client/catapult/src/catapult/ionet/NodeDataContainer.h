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

namespace catapult { namespace ionet {

	/// Catapult node with all associated information.
	struct NodeData {
	public:
		/// Creates node data around \a node, \a source and \a nodeId.
		NodeData(const Node& node, NodeSource source, size_t nodeId)
				: Node(node)
				, NodeInfo(source)
				, NodeId(nodeId)
				, HasIdentityUpdateInProgress(false)
		{}

	public:
		/// Global node descriptor.
		ionet::Node Node;

		/// Local supplemental node information.
		ionet::NodeInfo NodeInfo;

		/// Local node identifier.
		size_t NodeId;

		/// \c true if part one of a two phase identity update has been detected.
		/// \note Any identity change will require a two phase update where phase one is triggered by an accepted incoming connection,
		///       which will likely have a lower NodeSource.
		bool HasIdentityUpdateInProgress;
	};

	/// Container of nodes and associated data.
	class NodeDataContainer {
	public:
		/// Codes returned by prepare insert.
		enum class PrepareInsertCode {
			/// Insert is not allowed because it would conflict with existing data.
			Conflict,

			/// Insert is not allowed because existing data was received from a better source.
			Redundant,

			/// Insert is allowed.
			Allowed
		};

	public:
		/// Creates container with \a equalityStrategy.
		explicit NodeDataContainer(model::NodeIdentityEqualityStrategy equalityStrategy);

	public:
		/// Gets the container size.
		size_t size() const;

		/// Tries to get the data associated with \a identity.
		const NodeData* tryGet(const model::NodeIdentity& identity) const;

		/// Tries to get the data associated with \a identity.
		NodeData* tryGet(const model::NodeIdentity& identity);

		/// Iterates over all nodes and passes them to \a consumer.
		void forEach(const consumer<const Node&, const NodeInfo&>& consumer) const;

		/// Iterates over all nodes and passes them to \a consumer.
		void forEach(const consumer<NodeData&>& consumer);

	public:
		/// Prepares to insert the node identified by \a identity with \a source.
		std::pair<NodeData*, PrepareInsertCode> prepareInsert(const model::NodeIdentity& identity, NodeSource source);

		/// Inserts new node data (\a nodeData).
		NodeData* insert(const NodeData& nodeData);

		/// Erases data for the node identified by \a identity.
		void erase(const model::NodeIdentity& identity);

	public:
		/// Tries to find the worst contained node.
		const NodeData* tryFindWorst() const;

	private:
		const model::NodeIdentityEqualityStrategy m_equalityStrategy;
		model::NodeIdentityMap<NodeData> m_nodeMap;
		std::unordered_map<Key, std::string, utils::ArrayHasher<Key>> m_keyHostMap;
	};
}}
