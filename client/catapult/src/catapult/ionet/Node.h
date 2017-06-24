#pragma once
#include "catapult/model/NetworkInfo.h"
#include "catapult/types.h"
#include <iosfwd>
#include <string>

namespace catapult { namespace ionet {

	/// A node's publicly accessible endpoint.
	struct NodeEndpoint {
		/// The host.
		std::string Host;

		/// The port.
		unsigned short Port;
	};

	/// A unique node identifier.
	struct NodeIdentity {
		/// The public key.
		Key PublicKey;

		/// The (optional) friendly name.
		std::string Name;
	};

	/// A node in the P2P network.
	/// \note This class does not support assignment (as a result of const members) in order to allow
	///       precomputation of printable names.
	struct Node {
	public:
		/// The endpoint.
		const NodeEndpoint Endpoint;

		/// The identity.
		const NodeIdentity Identity;

		/// The network identifier.
		const model::NetworkIdentifier NetworkIdentifier;

	public:
		/// Creates a default node.
		Node();

		/// Creates a node around \a endpoint and \a identity for a network identified by \a networkIdentifier.
		Node(
				const NodeEndpoint& endpoint,
				const NodeIdentity& identity,
				model::NetworkIdentifier networkIdentifier);

	public:
		/// Returns \c true if this node is equal to \a rhs.
		bool operator==(const Node& rhs) const;

		/// Returns \c true if this node is not equal to \a rhs.
		bool operator!=(const Node& rhs) const;

	public:
		/// Insertion operator for outputting \a node to \a out.
		friend std::ostream& operator<<(std::ostream& out, const Node& node);

	private:
		std::string m_printableName;
	};
}}
