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

#include "catapult/ionet/NetworkNode.h"
#include "tests/test/nodeps/NumericTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace ionet {

#define TEST_CLASS NetworkNodeTests

	// region basic

	TEST(TEST_CLASS, EntityHasExpectedSize) {
		// Arrange:
		auto expectedSize =
				sizeof(uint32_t) // size of network node
				+ sizeof(uint16_t) // port
				+ Key_Size // identity key
				+ sizeof(model::NetworkIdentifier) // network identifier
				+ sizeof(NodeVersion) // version
				+ sizeof(NodeRoles) // roles
				+ sizeof(uint8_t) // host size
				+ sizeof(uint8_t); // friendly name size

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(NetworkNode));
		EXPECT_EQ(49u, sizeof(NetworkNode));
	}

	// endregion

	// region CalculateRealSize

	TEST(TEST_CLASS, CanCalculateRealSizeWithReasonableValues) {
		// Arrange:
		NetworkNode networkNode;
		networkNode.Size = 0;
		networkNode.HostSize = 12;
		networkNode.FriendlyNameSize = 37;

		// Act:
		auto realSize = NetworkNode::CalculateRealSize(networkNode);

		// Assert:
		EXPECT_EQ(sizeof(NetworkNode) + 12 + 37, realSize);
	}

	TEST(TEST_CLASS, CalculateRealSizeDoesNotOverflowWithMaxValues) {
		// Arrange:
		NetworkNode networkNode;
		networkNode.Size = 0;
		test::SetMaxValue(networkNode.HostSize);
		test::SetMaxValue(networkNode.FriendlyNameSize);

		// Act:
		auto realSize = NetworkNode::CalculateRealSize(networkNode);

		// Assert:
		EXPECT_EQ(sizeof(NetworkNode) + 0xFF + 0xFF, realSize);
		EXPECT_GE(std::numeric_limits<uint32_t>::max(), realSize);
	}

	// endregion

	// region PackNode

	namespace {
		Node CreateNodeForPackTests(const std::string& host, const std::string& name) {
			auto key = test::GenerateRandomByteArray<Key>();
			return Node(key, { host, 1234 }, { model::NetworkIdentifier::Mijin_Test, name, NodeVersion(7), NodeRoles::Peer });
		}

		void AssertPackedNode(
				const NetworkNode& node,
				const Key& expectedKey,
				const std::string& expectedHost,
				const std::string& expectedName) {
			const auto* host = reinterpret_cast<const char*>(&node + 1);
			const auto* name = host + expectedHost.size();

			// Assert:
			EXPECT_EQ(node.Size, sizeof(NetworkNode) + expectedHost.size() + expectedName.size());

			EXPECT_EQ(expectedKey, node.IdentityKey);
			EXPECT_EQ(1234u, node.Port);
			EXPECT_EQ(model::NetworkIdentifier::Mijin_Test, node.NetworkIdentifier);
			EXPECT_EQ(NodeVersion(7), node.Version);
			EXPECT_EQ(NodeRoles::Peer, node.Roles);

			ASSERT_EQ(expectedHost.size(), node.HostSize);
			EXPECT_EQ(expectedHost, std::string(host, node.HostSize));

			ASSERT_EQ(expectedName.size(), node.FriendlyNameSize);
			EXPECT_EQ(expectedName, std::string(name, node.FriendlyNameSize));
		}
	}

	TEST(TEST_CLASS, CanPackNodeWithNeitherHostNorName) {
		// Arrange:
		auto node = CreateNodeForPackTests("", "");

		// Act:
		auto pNetworkNode = PackNode(node);

		// Assert:
		AssertPackedNode(*pNetworkNode, node.identityKey(), "", "");
	}

	TEST(TEST_CLASS, CanPackNodeWithHostButNotName) {
		// Arrange:
		auto node = CreateNodeForPackTests("bob.com", "");

		// Act:
		auto pNetworkNode = PackNode(node);

		// Assert:
		AssertPackedNode(*pNetworkNode, node.identityKey(), "bob.com", "");
	}

	TEST(TEST_CLASS, CanPackNodeWithNameButNotHost) {
		// Arrange:
		auto node = CreateNodeForPackTests("", "supernode");

		// Act:
		auto pNetworkNode = PackNode(node);

		// Assert:
		AssertPackedNode(*pNetworkNode, node.identityKey(), "", "supernode");
	}

	TEST(TEST_CLASS, CanPackNodeWithHostAndName) {
		// Arrange:
		auto node = CreateNodeForPackTests("bob.com", "supernode");

		// Act:
		auto pNetworkNode = PackNode(node);

		// Assert:
		AssertPackedNode(*pNetworkNode, node.identityKey(), "bob.com", "supernode");
	}

	TEST(TEST_CLASS, CanPackNodeWithTruncatedHostAndName) {
		// Arrange: create strings that are too large for serialization
		auto node = CreateNodeForPackTests(std::string(500, 'h'), std::string(400, 'n'));

		// Act:
		auto pNetworkNode = PackNode(node);

		// Assert: the strings should have been truncated during packing
		AssertPackedNode(*pNetworkNode, node.identityKey(), std::string(0xFF, 'h'), std::string(0xFF, 'n'));
	}

	// endregion

	// region UnpackNode

	namespace {
		std::unique_ptr<NetworkNode> CreateNodeForUnpackTests(const std::string& host, const std::string& name) {
			return PackNode(CreateNodeForPackTests(host, name));
		}

		void AssertUnpackedNode(
				const Node& node,
				const Key& expectedKey,
				const std::string& expectedHost,
				const std::string& expectedName) {
			// Assert:
			EXPECT_EQ(expectedKey, node.identityKey());

			EXPECT_EQ(expectedHost, node.endpoint().Host);
			EXPECT_EQ(1234u, node.endpoint().Port);

			EXPECT_EQ(model::NetworkIdentifier::Mijin_Test, node.metadata().NetworkIdentifier);
			EXPECT_EQ(expectedName, node.metadata().Name);
			EXPECT_EQ(NodeVersion(7), node.metadata().Version);
			EXPECT_EQ(NodeRoles::Peer, node.metadata().Roles);
		}
	}

	TEST(TEST_CLASS, CanUnpackNodeWithNeitherHostNorName) {
		// Arrange:
		auto pNetworkNode = CreateNodeForUnpackTests("", "");

		// Act:
		auto node = UnpackNode(*pNetworkNode);

		// Assert:
		AssertUnpackedNode(node, pNetworkNode->IdentityKey, "", "");
	}

	TEST(TEST_CLASS, CanUnpackNodeWithHostButNotName) {
		// Arrange:
		auto pNetworkNode = CreateNodeForUnpackTests("bob.com", "");

		// Act:
		auto node = UnpackNode(*pNetworkNode);

		// Assert:
		AssertUnpackedNode(node, pNetworkNode->IdentityKey, "bob.com", "");
	}

	TEST(TEST_CLASS, CanUnpackNodeWithNameButNotHost) {
		// Arrange:
		auto pNetworkNode = CreateNodeForUnpackTests("", "supernode");

		// Act:
		auto node = UnpackNode(*pNetworkNode);

		// Assert:
		AssertUnpackedNode(node, pNetworkNode->IdentityKey, "", "supernode");
	}

	TEST(TEST_CLASS, CanUnpackNodeWithHostAndName) {
		// Arrange:
		auto pNetworkNode = CreateNodeForUnpackTests("bob.com", "supernode");

		// Act:
		auto node = UnpackNode(*pNetworkNode);

		// Assert:
		AssertUnpackedNode(node, pNetworkNode->IdentityKey, "bob.com", "supernode");
	}

	// endregion
}}
