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
		/// The host.
		std::string Host;

		/// The port.
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
		/// The network identifier.
		model::NetworkIdentifier NetworkIdentifier;

		/// The (optional) friendly name.
		std::string Name;

		/// The version.
		NodeVersion Version;

		/// The role(s).
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
