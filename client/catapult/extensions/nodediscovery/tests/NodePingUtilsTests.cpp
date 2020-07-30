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

#include "nodediscovery/src/NodePingUtils.h"
#include "catapult/ionet/NodeContainer.h"
#include "catapult/ionet/Packet.h"
#include "nodediscovery/tests/test/NodeDiscoveryTestUtils.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace nodediscovery {

#define TEST_CLASS NodePingUtilsTests

	// region TryParseNodePacket

	TEST(TEST_CLASS, TryParseNodePacketFailsWhenPacketDoesNotContainExactlyOneNetworkNode) {
		// Arrange: change the packet size so it looks like it contains two nodes
		auto identityKey = test::GenerateRandomByteArray<Key>();
		auto pPacket = test::CreateNodePushPingPacket(identityKey, ionet::NodeVersion(1234), "alice.com", "xyz");
		pPacket->Size += pPacket->Size - SizeOf32<ionet::Packet>();

		// Act:
		ionet::Node node;
		auto result = TryParseNodePacket(*pPacket, node);

		// Assert:
		EXPECT_FALSE(result);
	}

	TEST(TEST_CLASS, TryParseNodePacketSucceedsWhenPacketContainsExactlyOneNetworkNode) {
		// Arrange:
		auto identityKey = test::GenerateRandomByteArray<Key>();
		auto pPacket = test::CreateNodePushPingPacket(identityKey, ionet::NodeVersion(1234), "alice.com", "xyz");

		// Act:
		ionet::Node node;
		auto result = TryParseNodePacket(*pPacket, node);

		// Assert:
		EXPECT_TRUE(result);
		EXPECT_EQ(identityKey, node.identity().PublicKey);
		EXPECT_EQ("", node.identity().Host);
		EXPECT_EQ("alice.com", node.endpoint().Host);
		EXPECT_EQ(ionet::NodeVersion(1234), node.metadata().Version);
		EXPECT_EQ("xyz", node.metadata().Name);
	}

	// endregion

	// region TryParseNodesPacket

	namespace {
		model::NodeIdentity GenerateRandomNodeIdentity() {
			return { test::GenerateRandomByteArray<Key>(), "" };
		}
	}

	TEST(TEST_CLASS, TryParseNodesPacketFailsWhenPacketPayloadIsMalformed) {
		// Arrange:
		std::vector<ionet::Node> nodes{ test::CreateNamedNode(test::GenerateRandomByteArray<Key>(), "a") };
		auto pPacket = test::CreateNodePushPeersPacket(nodes);
		pPacket->Size += 1;

		// Act:
		ionet::NodeSet parsedNodes;
		auto result = TryParseNodesPacket(*pPacket, parsedNodes);

		// Assert:
		EXPECT_FALSE(result);
		EXPECT_TRUE(parsedNodes.empty());
	}

	TEST(TEST_CLASS, TryParseNodesPacketSucceedsWhenPacketPayloadIsEmpty) {
		// Arrange:
		std::vector<ionet::Node> nodes;
		auto pPacket = test::CreateNodePushPeersPacket(nodes);

		// Act:
		ionet::NodeSet parsedNodes;
		auto result = TryParseNodesPacket(*pPacket, parsedNodes);

		// Assert:
		EXPECT_TRUE(result);
		EXPECT_TRUE(parsedNodes.empty());
	}

	TEST(TEST_CLASS, TryParseNodesPacketSucceedsWhenPacketPayloadContainsSingleNode) {
		// Arrange:
		std::vector<ionet::Node> nodes{ test::CreateNamedNode(GenerateRandomNodeIdentity(), "a") };
		auto pPacket = test::CreateNodePushPeersPacket(nodes);

		// Act:
		ionet::NodeSet parsedNodes;
		auto result = TryParseNodesPacket(*pPacket, parsedNodes);

		// Assert:
		EXPECT_TRUE(result);
		EXPECT_EQ(1u, parsedNodes.size());
		test::AssertEqualNodes(ionet::NodeSet(nodes.cbegin(), nodes.cend()), parsedNodes);
	}

	TEST(TEST_CLASS, TryParseNodesPacketSucceedsWhenPacketPayloadContainsMultipleUniqueNodes) {
		// Arrange:
		std::vector<ionet::Node> nodes{
			test::CreateNamedNode(GenerateRandomNodeIdentity(), "a"),
			test::CreateNamedNode(GenerateRandomNodeIdentity(), "bc"),
			test::CreateNamedNode(GenerateRandomNodeIdentity(), "def")
		};
		auto pPacket = test::CreateNodePushPeersPacket(nodes);

		// Act:
		ionet::NodeSet parsedNodes;
		auto result = TryParseNodesPacket(*pPacket, parsedNodes);

		// Assert:
		EXPECT_TRUE(result);
		EXPECT_EQ(3u, parsedNodes.size());
		test::AssertEqualNodes(ionet::NodeSet(nodes.cbegin(), nodes.cend()), parsedNodes);
	}

	TEST(TEST_CLASS, TryParseNodesPacketSucceedsWhenPacketPayloadContainsMultipleNodesWithDuplicates) {
		// Arrange: create a payload with 3 unique nodes and 2 duplicates
		std::vector<ionet::Node> nodes{
			test::CreateNamedNode(GenerateRandomNodeIdentity(), "a"),
			test::CreateNamedNode(GenerateRandomNodeIdentity(), "bc"),
			test::CreateNamedNode(GenerateRandomNodeIdentity(), "def")
		};
		nodes.push_back(nodes[2]);
		nodes.push_back(nodes[0]);
		auto pPacket = test::CreateNodePushPeersPacket(nodes);

		// Sanity:
		EXPECT_EQ(5u, nodes.size());

		// Act:
		ionet::NodeSet parsedNodes;
		auto result = TryParseNodesPacket(*pPacket, parsedNodes);

		// Assert:
		EXPECT_TRUE(result);
		EXPECT_EQ(3u, parsedNodes.size());
		test::AssertEqualNodes(ionet::NodeSet(nodes.cbegin(), nodes.cend()), parsedNodes);
	}

	// endregion

	// region IsNodeCompatible

	TEST(TEST_CLASS, IsNodeCompatibleReturnsFalseWhenNetworkDoesNotMatch) {
		// Arrange:
		ionet::Node node({ test::GenerateRandomByteArray<Key>(), "11.22.33.44" });

		// Act + Assert:
		EXPECT_FALSE(IsNodeCompatible(node, test::CreateNodeDiscoveryNetworkFingerprint(), node.identity().PublicKey));
	}

	TEST(TEST_CLASS, IsNodeCompatibleReturnsFalseWhenIdentityDoesNotMatch) {
		// Arrange:
		ionet::Node node({ test::GenerateRandomByteArray<Key>(), "11.22.33.44" });

		// Act + Assert:
		EXPECT_FALSE(IsNodeCompatible(node, node.metadata().NetworkFingerprint, test::GenerateRandomByteArray<Key>()));
	}

	TEST(TEST_CLASS, IsNodeCompatibleReturnsTrueWhenAllChecksPass) {
		// Arrange:
		ionet::Node node({ test::GenerateRandomByteArray<Key>(), "11.22.33.44" });

		// Act + Assert:
		EXPECT_TRUE(IsNodeCompatible(node, node.metadata().NetworkFingerprint, node.identity().PublicKey));
	}

	// endregion

	// region SelectUnknownNodes

	namespace {
		ionet::NodeSet CreateNodes(size_t numNodes) {
			ionet::NodeSet nodes;
			for (auto i = 0u; i < numNodes; ++i)
				nodes.emplace(test::CreateNamedNode(test::GenerateRandomByteArray<Key>(), ""));

			return nodes;
		}

		template<typename TTraits>
		void AddNodes(ionet::NodeContainer& nodeContainer, const ionet::NodeSet& nodesToAdd) {
			auto modifier = nodeContainer.modifier();
			for (const auto& node : nodesToAdd)
				modifier.add(TTraits::Transform(node), ionet::NodeSource::Dynamic);
		}

		const ionet::Node& GetNodeAt(const ionet::NodeSet& nodes, int index) {
			auto iter = nodes.cbegin();
			std::advance(iter, index);
			return *iter;
		}

		struct KeyAndHostMatchTraits {
			static ionet::Node Transform(const ionet::Node& node) {
				return node;
			}
		};

		struct KeyMatchTraits {
			static ionet::Node Transform(const ionet::Node& node) {
				// this more closely simulates production behavior where nodes from peers requests don't include identity hosts
				return ionet::Node({ node.identity().PublicKey, "" }, node.endpoint(), node.metadata());
			}
		};
	}

#define SELECT_UNKNOWN_NODES_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, SelectUnknownNodes##TEST_NAME##_KeyAndHost) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<KeyAndHostMatchTraits>(); } \
	TEST(TEST_CLASS, SelectUnknownNodes##TEST_NAME##_Key) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<KeyMatchTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	SELECT_UNKNOWN_NODES_TEST(ReturnsEmptySetWhenAllNodesAreKnown) {
		// Arrange: add all input nodes to the container
		auto inputNodes = CreateNodes(5);
		ionet::NodeContainer nodeContainer;
		AddNodes<TTraits>(nodeContainer, CreateNodes(5));
		AddNodes<TTraits>(nodeContainer, inputNodes);

		// Act:
		auto unknownNodes = SelectUnknownNodes(nodeContainer.view(), inputNodes);

		// Assert:
		EXPECT_TRUE(unknownNodes.empty());
	}

	SELECT_UNKNOWN_NODES_TEST(ReturnsInputSubsetWhenSomeNodesAreKnown) {
		// Arrange: add some input nodes to the container
		auto inputNodes = CreateNodes(5);
		ionet::NodeContainer nodeContainer;
		AddNodes<TTraits>(nodeContainer, CreateNodes(5));
		AddNodes<TTraits>(nodeContainer, { GetNodeAt(inputNodes, 0), GetNodeAt(inputNodes, 2), GetNodeAt(inputNodes, 4) });

		// Act:
		auto unknownNodes = SelectUnknownNodes(nodeContainer.view(), inputNodes);

		// Assert:
		EXPECT_EQ(2u, unknownNodes.size());
		test::AssertEqualNodes(ionet::NodeSet({ GetNodeAt(inputNodes, 1), GetNodeAt(inputNodes, 3) }), unknownNodes);
	}

	SELECT_UNKNOWN_NODES_TEST(ReturnsInputSetWhenAllNodesAreUnknown) {
		// Arrange: add no input nodes to the container
		auto inputNodes = CreateNodes(5);
		ionet::NodeContainer nodeContainer;
		AddNodes<TTraits>(nodeContainer, CreateNodes(5));

		// Act:
		auto unknownNodes = SelectUnknownNodes(nodeContainer.view(), inputNodes);

		// Assert:
		EXPECT_EQ(5u, unknownNodes.size());
		test::AssertEqualNodes(inputNodes, unknownNodes);
	}

	// endregion
}}
