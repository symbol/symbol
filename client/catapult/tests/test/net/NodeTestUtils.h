#pragma once
#include "catapult/ionet/NodeContainer.h"
#include "tests/TestHarness.h"
#include <unordered_set>

namespace catapult { namespace test {

	/// Creates a node endpoint referencing the local host with the specified \a port.
	inline ionet::NodeEndpoint CreateLocalHostNodeEndpoint(unsigned short port = Local_Host_Port) {
		return { "127.0.0.1", port };
	}

	/// Creates a node referencing the local host with the specified \a port and public key (\a publicKey).
	inline ionet::Node CreateLocalHostNode(const Key& publicKey, unsigned short port = Local_Host_Port) {
		return { publicKey, CreateLocalHostNodeEndpoint(port), ionet::NodeMetadata() };
	}

	/// Creates a node with \a identityKey, \a name and \a roles.
	inline ionet::Node CreateNamedNode(const Key& identityKey, const std::string& name, ionet::NodeRoles roles = ionet::NodeRoles::None) {
		auto metadata = ionet::NodeMetadata(model::NetworkIdentifier::Zero, name);
		metadata.Roles = roles;
		return ionet::Node(identityKey, ionet::NodeEndpoint(), metadata);
	}

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
		/// The node identity key.
		Key IdentityKey;

		/// The node name.
		std::string Name;

		/// The node source.
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

	/// Asserts that \a connectionState is zeroed.
	void AssertZeroed(const ionet::ConnectionState& connectionState);
}}
