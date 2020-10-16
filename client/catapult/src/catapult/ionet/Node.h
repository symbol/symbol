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
#include "NodeVersion.h"
#include "catapult/model/NetworkIdentifier.h"
#include "catapult/model/NodeIdentity.h"
#include "catapult/utils/Hashers.h"
#include <unordered_set>

namespace catapult { namespace ionet {

	// region NodeEndpoint

	/// Publicly accessible node endpoint.
	struct NodeEndpoint {
		/// Host (may or may not be resolved).
		std::string Host;

		/// Port.
		unsigned short Port;
	};

	// endregion

	// region NodeMetadata

	/// Additional node information.
	struct NodeMetadata {
	public:
		/// Creates default metadata.
		NodeMetadata() : NodeMetadata(model::UniqueNetworkFingerprint())
		{}

		/// Creates metadata for a node in the network identified by \a networkFingerprint.
		explicit NodeMetadata(const model::UniqueNetworkFingerprint& networkFingerprint) : NodeMetadata(networkFingerprint, "")
		{}

		/// Creates metadata for a node with \a name in the network identified by \a networkFingerprint.
		NodeMetadata(const model::UniqueNetworkFingerprint& networkFingerprint, const std::string& name)
				: NodeMetadata(networkFingerprint, name, NodeVersion(), NodeRoles::None)
		{}

		/// Creates metadata for a node with \a name, \a version and \a roles in the network identified by \a networkFingerprint.
		NodeMetadata(
				const model::UniqueNetworkFingerprint& networkFingerprint,
				const std::string& name,
				NodeVersion version,
				NodeRoles roles)
				: NetworkFingerprint(networkFingerprint)
				, Name(name)
				, Version(version)
				, Roles(roles)
		{}

	public:
		/// Network fingerprint.
		model::UniqueNetworkFingerprint NetworkFingerprint;

		/// Friendly name (optional).
		std::string Name;

		/// Version.
		NodeVersion Version;

		/// Role(s).
		NodeRoles Roles;
	};

	// endregion

	// region Node

	/// Catapult network node.
	class Node {
	public:
		/// Creates a default node.
		Node();

		/// Creates a node around a unique \a identity.
		explicit Node(const model::NodeIdentity& identity);

		/// Creates a node around a unique \a identity with \a endpoint and \a metadata.
		Node(const model::NodeIdentity& identity, const NodeEndpoint& endpoint, const NodeMetadata& metadata);

	public:
		/// Gets the unique identity.
		const model::NodeIdentity& identity() const;

		/// Gets the endpoint.
		const NodeEndpoint& endpoint() const;

		/// Gets the metadata.
		const NodeMetadata& metadata() const;

	public:
		/// Insertion operator for outputting \a node to \a out.
		friend std::ostream& operator<<(std::ostream& out, const Node& node);

	private:
		model::NodeIdentity m_identity;
		NodeEndpoint m_endpoint;
		NodeMetadata m_metadata;

		std::string m_printableName;
	};

	// endregion
}}
