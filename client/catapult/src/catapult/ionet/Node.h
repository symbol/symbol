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
#include "NodeRoles.h"
#include "catapult/model/NetworkInfo.h"
#include "catapult/utils/Hashers.h"
#include <unordered_set>

namespace catapult { namespace ionet {

	struct NodeVersion_tag {};

	/// 32-bit node version where first three bytes represent { major, minor, build } and last byte is user defined.
	using NodeVersion = utils::BaseValue<uint32_t, NodeVersion_tag>;

	/// A node's publicly accessible endpoint.
	struct NodeEndpoint {
		/// Host.
		std::string Host;

		/// Port.
		unsigned short Port;
	};

	/// Additional node information.
	struct NodeMetadata {
	public:
		/// Creates default metadata.
		NodeMetadata() : NodeMetadata(model::NetworkIdentifier::Zero)
		{}

		/// Creates metadata for a node in the network identified by \a networkIdentifier.
		explicit NodeMetadata(model::NetworkIdentifier networkIdentifier) : NodeMetadata(networkIdentifier, "")
		{}

		/// Creates metadata for a node with \a name in the network identified by \a networkIdentifier.
		NodeMetadata(model::NetworkIdentifier networkIdentifier, const std::string& name)
				: NodeMetadata(networkIdentifier, name, NodeVersion(), NodeRoles::None)
		{}

		/// Creates metadata for a node with \a name, \a version and \a roles in the network identified by \a networkIdentifier.
		NodeMetadata(model::NetworkIdentifier networkIdentifier, const std::string& name, NodeVersion version, NodeRoles roles)
				: NetworkIdentifier(networkIdentifier)
				, Name(name)
				, Version(version)
				, Roles(roles)
		{}

	public:
		/// Network identifier.
		model::NetworkIdentifier NetworkIdentifier;

		/// Friendly name (optional).
		std::string Name;

		/// Version.
		NodeVersion Version;

		/// Role(s).
		NodeRoles Roles;
	};

	/// A node in the catapult network.
	class Node {
	public:
		/// Creates a default node.
		Node();

		/// Creates a node around a unique identifier (\a identityKey) with \a endpoint and \a metadata.
		Node(const Key& identityKey, const NodeEndpoint& endpoint, const NodeMetadata& metadata);

	public:
		/// Gets the unique identifier (a public key).
		const Key& identityKey() const;

		/// Gets the endpoint.
		const NodeEndpoint& endpoint() const;

		/// Gets tne metadata.
		const NodeMetadata& metadata() const;

	public:
		/// Returns \c true if this node is equal to \a rhs.
		bool operator==(const Node& rhs) const;

		/// Returns \c true if this node is not equal to \a rhs.
		bool operator!=(const Node& rhs) const;

	public:
		/// Insertion operator for outputting \a node to \a out.
		friend std::ostream& operator<<(std::ostream& out, const Node& node);

	private:
		Key m_identityKey;
		NodeEndpoint m_endpoint;
		NodeMetadata m_metadata;

		std::string m_printableName;
	};

	/// Hasher object for a node.
	struct NodeHasher {
		/// Hashes \a node.
		size_t operator()(const Node& node) const {
			return utils::ArrayHasher<Key>()(node.identityKey());
		}
	};

	/// A set of nodes.
	using NodeSet = std::unordered_set<Node, NodeHasher>;
}}
