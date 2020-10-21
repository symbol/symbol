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

#include "NodeTestUtils.h"
#include "catapult/ionet/NodeInteractionResult.h"
#include "catapult/utils/Casting.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	// region NodeEndpoint / Node factories

	ionet::NodeEndpoint CreateLocalHostNodeEndpoint() {
		return CreateLocalHostNodeEndpoint(GetLocalHostPort());
	}

	ionet::NodeEndpoint CreateLocalHostNodeEndpoint(unsigned short port) {
		return { "127.0.0.1", port };
	}

	ionet::Node CreateLocalHostNode(const Key& publicKey) {
		return CreateLocalHostNode(publicKey, GetLocalHostPort());
	}

	ionet::Node CreateLocalHostNode(const Key& publicKey, unsigned short port) {
		return { { publicKey, "127.0.0.1" }, CreateLocalHostNodeEndpoint(port), ionet::NodeMetadata() };
	}

	ionet::Node CreateNamedNode(const Key& identityKey, const std::string& name, ionet::NodeRoles roles) {
		return CreateNamedNode({ identityKey, "fake-host-from-create-named-node" }, name, roles);
	}

	ionet::Node CreateNamedNode(const model::NodeIdentity& identity, const std::string& name, ionet::NodeRoles roles) {
		return CreateNamedNode(identity, name, ionet::NodeVersion(), roles);
	}

	ionet::Node CreateNamedNode(
			const model::NodeIdentity& identity,
			const std::string& name,
			ionet::NodeVersion version,
			ionet::NodeRoles roles) {
		auto metadata = ionet::NodeMetadata(model::UniqueNetworkFingerprint(), name);
		metadata.Version = version;
		metadata.Roles = roles;
		return ionet::Node(identity, ionet::NodeEndpoint(), metadata);
	}

	// endregion

	// region BasicNodeData

	std::ostream& operator<<(std::ostream& out, const BasicNodeData& data) {
		out << data.Name << " (source " << data.Source << ") " << data.IdentityKey;
		return out;
	}

	BasicNodeDataContainer CollectAll(const ionet::NodeContainerView& view) {
		BasicNodeDataContainer basicDataContainer;
		view.forEach([&basicDataContainer](const auto& node, const auto& nodeInfo) {
			basicDataContainer.insert({ node.identity().PublicKey, node.metadata().Name, nodeInfo.source() });
		});

		return basicDataContainer;
	}

	// endregion

	// region general utils

	void AddNodeInteractions(
			ionet::NodeContainerModifier& modifier,
			const model::NodeIdentity& identity,
			size_t numSuccesses,
			size_t numFailures) {
		for (auto i = 0u; i < numSuccesses; ++i)
			modifier.incrementSuccesses(identity);

		for (auto i = 0u; i < numFailures; ++i)
			modifier.incrementFailures(identity);
	}

	model::NodeIdentitySet ToIdentitiesSet(const std::vector<model::NodeIdentity>& seedIdentities) {
		auto identities = model::CreateNodeIdentitySet(model::NodeIdentityEqualityStrategy::Key_And_Host);

		for (const auto& identity : seedIdentities)
			identities.insert(identity);

		return identities;
	}

	model::NodeIdentitySet ToIdentitiesSet(const std::vector<Key>& identityKeys, const std::string& host) {
		std::vector<model::NodeIdentity> identities;

		for (const auto& identityKey : identityKeys)
			identities.push_back({ identityKey, host });

		return ToIdentitiesSet(identities);
	}

	// endregion

	// region custom asserts

	void AssertZeroed(const ionet::ConnectionState& connectionState) {
		// Assert:
		EXPECT_EQ(0u, connectionState.Age);
		EXPECT_EQ(0u, connectionState.NumConsecutiveFailures);
		EXPECT_EQ(0u, connectionState.BanAge);
	}

	void AssertNodeInteractions(
			uint32_t expectedNumSuccesses,
			uint32_t expectedNumFailures,
			const ionet::NodeInteractions& interactions,
			const std::string& message) {
		// Assert:
		EXPECT_EQ(expectedNumSuccesses, interactions.NumSuccesses) << message;
		EXPECT_EQ(expectedNumFailures, interactions.NumFailures) << message;
	}

	namespace {
		template<typename TSet>
		void AssertEqualSets(const TSet& expected, const TSet& actual) {
			EXPECT_EQ(expected.size(), actual.size());

			auto areEqual = true;
			for (const auto& expectedValue : expected)
				areEqual = areEqual && actual.cend() != actual.find(expectedValue);

			if (areEqual)
				return;

			std::ostringstream out;
			out << "[expected]" << std::endl;
			for (const auto& identity : expected)
				out << " * " << identity << std::endl;

			out << "[actual]" << std::endl;
			for (const auto& identity : actual)
				out << " * " << identity << std::endl;

			EXPECT_TRUE(areEqual) << out.str();
		}
	}

	void AssertEqualIdentities(const model::NodeIdentitySet& expected, const model::NodeIdentitySet& actual) {
		AssertEqualSets(expected, actual);
	}

	void AssertEqualNodes(const ionet::NodeSet& expected, const ionet::NodeSet& actual) {
		AssertEqualSets(expected, actual);
	}

	// endregion
}}
