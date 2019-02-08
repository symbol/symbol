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
#include "catapult/ionet/NodeContainer.h"
#include "tests/TestHarness.h"
#include <unordered_set>

namespace catapult { namespace test {

	/// Creates a node endpoint referencing the local host with the default port.
	ionet::NodeEndpoint CreateLocalHostNodeEndpoint();

	/// Creates a node endpoint referencing the local host with the specified \a port.
	ionet::NodeEndpoint CreateLocalHostNodeEndpoint(unsigned short port);

	/// Creates a node referencing the local host with the default port and specified public key (\a publicKey).
	ionet::Node CreateLocalHostNode(const Key& publicKey);

	/// Creates a node referencing the local host with the specified \a port and public key (\a publicKey).
	ionet::Node CreateLocalHostNode(const Key& publicKey, unsigned short port);

	/// Creates a node with \a identityKey, \a name and \a roles.
	ionet::Node CreateNamedNode(const Key& identityKey, const std::string& name, ionet::NodeRoles roles = ionet::NodeRoles::None);

	/// Extracts all node identities from \a nodes.
	template<typename TNodeContainer>
	utils::KeySet ExtractNodeIdentities(const TNodeContainer& nodes) {
		utils::KeySet identities;
		for (const auto& node : nodes)
			identities.emplace(node.identityKey());

		return identities;
	}

	/// Basic node data used to spot check the contents of a NodeContainer.
	struct BasicNodeData {
	public:
		/// Node identity key.
		Key IdentityKey;

		/// Node name.
		std::string Name;

		/// Node source.
		ionet::NodeSource Source;

	public:
		/// Returns \c true if this data is equal to \a rhs.
		bool operator==(const BasicNodeData& rhs) const {
			return IdentityKey == rhs.IdentityKey && Name == rhs.Name && Source == rhs.Source;
		}
	};

	/// Insertion operator for outputting \a data to \a out.
	std::ostream& operator<<(std::ostream& out, const BasicNodeData& data);

	/// Hasher object for basic node data.
	struct BasicNodeDataHasher {
		/// Hashes \a data.
		size_t operator()(const BasicNodeData& data) const {
			return utils::ArrayHasher<Key>()(data.IdentityKey);
		}
	};

	/// A set of basic node data elements.
	using BasicNodeDataContainer = std::unordered_set<BasicNodeData, BasicNodeDataHasher>;

	/// Collects all basic node data information from \a view.
	BasicNodeDataContainer CollectAll(const ionet::NodeContainerView& view);

	/// Adds interactions (\a numSuccesses and \a numFailures) to the node identified by \a identityKey contained in \a modifier.
	void AddNodeInteractions(ionet::NodeContainerModifier& modifier, const Key& identityKey, size_t numSuccesses, size_t numFailures);

	/// Asserts that \a connectionState is zeroed.
	void AssertZeroed(const ionet::ConnectionState& connectionState);

	/// Asserts \a interactions has \a expectedNumSuccesses and \a expectedNumFailures;
	/// \a message is used to output additional information.
	void AssertNodeInteractions(
			uint32_t expectedNumSuccesses,
			uint32_t expectedNumFailures,
			const ionet::NodeInteractions& interactions,
			const std::string& message = "");
}}
