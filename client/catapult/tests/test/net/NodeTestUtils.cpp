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
		return { publicKey, CreateLocalHostNodeEndpoint(port), ionet::NodeMetadata() };
	}

	ionet::Node CreateNamedNode(const Key& identityKey, const std::string& name, ionet::NodeRoles roles) {
		auto metadata = ionet::NodeMetadata(model::NetworkIdentifier::Zero, name);
		metadata.Roles = roles;
		return ionet::Node(identityKey, ionet::NodeEndpoint(), metadata);
	}

	std::ostream& operator<<(std::ostream& out, const BasicNodeData& data) {
		out << data.Name << " (source " << data.Source << ") " << data.IdentityKey;
		return out;
	}

	BasicNodeDataContainer CollectAll(const ionet::NodeContainerView& view) {
		BasicNodeDataContainer basicDataContainer;
		view.forEach([&basicDataContainer](const auto& node, const auto& nodeInfo) {
			basicDataContainer.insert({ node.identityKey(), node.metadata().Name, nodeInfo.source() });
		});

		return basicDataContainer;
	}

	void AddNodeInteractions(ionet::NodeContainerModifier& modifier, const Key& identityKey, size_t numSuccesses, size_t numFailures) {
		for (auto i = 0u; i < numSuccesses; ++i)
			modifier.incrementSuccesses(identityKey);

		for (auto i = 0u; i < numFailures; ++i)
			modifier.incrementFailures(identityKey);
	}

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
}}
