/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "nodediscovery/src/handlers/NodeDiscoveryHandlers.h"
#include "catapult/ionet/NetworkNode.h"
#include "catapult/utils/Functional.h"
#include "nodediscovery/tests/test/NodeDiscoveryTestUtils.h"
#include "tests/test/core/PacketPayloadTestUtils.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace handlers {

#define TEST_CLASS NodeDiscoveryHandlersTests

	// region RegisterNodeDiscoveryPushPingHandler

	namespace {
		auto CreateNodePushPingPacket(const Key& identityKey, const std::string& host) {
			return test::CreateNodePushPingPacket(identityKey, ionet::NodeVersion(1234), host, "");
		}

		std::pair<ionet::Node, size_t> RegisterAndExecutePushPingHandler(
				const ionet::Packet& packet,
				ionet::ServerPacketHandlerContext& handlerContext) {
			ionet::ServerPacketHandlers handlers;
			auto networkFingerprint = test::CreateNodeDiscoveryNetworkFingerprint();

			ionet::Node capturedNode;
			auto numConsumerCalls = 0u;
			RegisterNodeDiscoveryPushPingHandler(handlers, networkFingerprint, [&capturedNode, &numConsumerCalls](const auto& node) {
				capturedNode = node;
				++numConsumerCalls;
			});

			// Act:
			EXPECT_TRUE(handlers.process(packet, handlerContext));

			// Assert:
			test::AssertNoResponse(handlerContext);
			return std::make_pair(capturedNode, numConsumerCalls);
		}
	}

	TEST(TEST_CLASS, PushPingHandler_FailsWhenPacketDoesNotContainExactlyOneNetworkNode) {
		// Arrange: decrease the node size by one so that the packet size is too large
		auto identityKey = test::GenerateRandomByteArray<Key>();
		auto pPacket = CreateNodePushPingPacket(identityKey, "");
		--(reinterpret_cast<ionet::NetworkNode&>(*pPacket->Data())).Size;

		std::string host = "alice.com";
		ionet::ServerPacketHandlerContext handlerContext(identityKey, host);

		// Act:
		auto result = RegisterAndExecutePushPingHandler(*pPacket, handlerContext);

		// Assert:
		EXPECT_EQ(0u, result.second);
	}

	TEST(TEST_CLASS, PushPingHandler_FailsWhenIdentityKeyDoesNotMatch) {
		// Arrange: use a random identity key
		auto identityKey = test::GenerateRandomByteArray<Key>();
		auto pPacket = CreateNodePushPingPacket(test::GenerateRandomByteArray<Key>(), "");

		std::string host = "alice.com";
		ionet::ServerPacketHandlerContext handlerContext(identityKey, host);

		// Act:
		auto result = RegisterAndExecutePushPingHandler(*pPacket, handlerContext);

		// Assert:
		EXPECT_EQ(0u, result.second);
	}

	TEST(TEST_CLASS, PushPingHandler_FailsWhenNetworkDoesNotMatch) {
		// Arrange:
		auto identityKey = test::GenerateRandomByteArray<Key>();
		auto pPacket = CreateNodePushPingPacket(identityKey, "");
		reinterpret_cast<ionet::NetworkNode&>(*pPacket->Data()).NetworkIdentifier = static_cast<model::NetworkIdentifier>(123);

		std::string host = "alice.com";
		ionet::ServerPacketHandlerContext handlerContext(identityKey, host);

		// Act:
		auto result = RegisterAndExecutePushPingHandler(*pPacket, handlerContext);

		// Assert:
		EXPECT_EQ(0u, result.second);
	}

	TEST(TEST_CLASS, PushPingHandler_SucceedsWhenProvidedExplicitHost) {
		// Arrange:
		auto identityKey = test::GenerateRandomByteArray<Key>();
		auto pPacket = CreateNodePushPingPacket(identityKey, "bob.org");

		std::string host = "11.22.33.44";
		ionet::ServerPacketHandlerContext handlerContext(identityKey, host);

		// Act:
		auto result = RegisterAndExecutePushPingHandler(*pPacket, handlerContext);

		// Assert:
		EXPECT_EQ(1u, result.second);

		// - explicitly provided host should be used
		const auto& node = result.first;
		EXPECT_EQ(identityKey, node.identity().PublicKey);
		EXPECT_EQ("11.22.33.44", node.identity().Host);
		EXPECT_EQ("bob.org", node.endpoint().Host);
		EXPECT_EQ(ionet::NodeVersion(1234), node.metadata().Version);
	}

	TEST(TEST_CLASS, PushPingHandler_SucceedsWhenProvidedImplicitHost) {
		// Arrange:
		auto identityKey = test::GenerateRandomByteArray<Key>();
		auto pPacket = CreateNodePushPingPacket(identityKey, "");

		std::string host = "11.22.33.44";
		ionet::ServerPacketHandlerContext handlerContext(identityKey, host);

		// Act:
		auto result = RegisterAndExecutePushPingHandler(*pPacket, handlerContext);

		// Assert:
		EXPECT_EQ(1u, result.second);

		// - host from handlerContext should be used
		const auto& node = result.first;
		EXPECT_EQ(identityKey, node.identity().PublicKey);
		EXPECT_EQ("11.22.33.44", node.identity().Host);
		EXPECT_EQ("11.22.33.44", node.endpoint().Host);
		EXPECT_EQ(ionet::NodeVersion(1234), node.metadata().Version);
	}

	// endregion

	// region RegisterNodeDiscoveryPullPingHandler

	namespace {
		template<typename TAssert>
		void RunPullPingHandlerTest(uint32_t packetExtraSize, TAssert assertFunc) {
			// Arrange:
			ionet::ServerPacketHandlers handlers;
			auto pNetworkNode = utils::UniqueToShared(test::CreateNetworkNode("host", "alice"));
			RegisterNodeDiscoveryPullPingHandler(handlers, pNetworkNode);

			// - create a valid request
			auto pPacket = ionet::CreateSharedPacket<ionet::Packet>();
			pPacket->Type = ionet::PacketType::Node_Discovery_Pull_Ping;
			pPacket->Size += packetExtraSize;

			// Act:
			ionet::ServerPacketHandlerContext handlerContext;
			EXPECT_TRUE(handlers.process(*pPacket, handlerContext));

			// Assert:
			assertFunc(*pNetworkNode, handlerContext);
		}
	}

	TEST(TEST_CLASS, PullPingHandler_DoesNotRespondToMalformedRequest) {
		// Arrange:
		RunPullPingHandlerTest(1, [](const auto&, const auto& handlerContext) {
			// Assert: malformed packet is ignored
			test::AssertNoResponse(handlerContext);
		});
	}

	TEST(TEST_CLASS, PullPingHandler_WritesLocalNodeInformationInResponseToValidRequest) {
		// Arrange:
		RunPullPingHandlerTest(0, [](const auto& networkNode, const auto& handlerContext) {
			// Assert: network node is written
			test::AssertPacketHeader(
					handlerContext,
					sizeof(ionet::Packet) + networkNode.Size,
					ionet::PacketType::Node_Discovery_Pull_Ping);

			const auto* pResponse = test::GetSingleBufferData(handlerContext);
			EXPECT_EQ_MEMORY(pResponse, &networkNode, networkNode.Size);
		});
	}

	// endregion

	// region RegisterNodeDiscoveryPushPeersHandler

	namespace {
		std::pair<std::vector<ionet::Node>, size_t> RegisterAndExecutePushPeersHandler(
				const std::vector<ionet::Node>& nodes,
				uint32_t packetExtraSize = 0) {
			// Arrange:
			ionet::ServerPacketHandlers handlers;
			std::vector<ionet::Node> capturedNodes;
			auto numConsumerCalls = 0u;
			RegisterNodeDiscoveryPushPeersHandler(handlers, [&capturedNodes, &numConsumerCalls](const auto& pushedNodes) {
				capturedNodes.insert(capturedNodes.end(), pushedNodes.cbegin(), pushedNodes.cend());
				++numConsumerCalls;
			});

			auto pPacket = test::CreateNodePushPeersPacket(nodes);
			pPacket->Size += packetExtraSize;

			// Act:
			ionet::ServerPacketHandlerContext handlerContext;
			EXPECT_TRUE(handlers.process(*pPacket, handlerContext));

			// Assert:
			test::AssertNoResponse(handlerContext);
			return std::make_pair(capturedNodes, numConsumerCalls);
		}
	}

	TEST(TEST_CLASS, PushPeersHandler_DoesNotForwardMalformedEntityToConsumer) {
		// Arrange:
		std::vector<ionet::Node> nodes{ test::CreateNamedNode(test::GenerateRandomByteArray<Key>(), "a") };

		// Act:
		auto result = RegisterAndExecutePushPeersHandler(nodes, 1);

		// Assert:
		EXPECT_EQ(0u, result.second);
	}

	TEST(TEST_CLASS, PushPeersHandler_DoesNotForwardEmptyPayloadToConsumer) {
		// Arrange:
		std::vector<ionet::Node> nodes;

		// Act:
		auto result = RegisterAndExecutePushPeersHandler(nodes);

		// Assert:
		EXPECT_EQ(0u, result.second);
	}

	TEST(TEST_CLASS, PushPeersHandler_ForwardsSingleEntityPayloadToConsumer) {
		// Arrange:
		std::vector<ionet::Node> nodes{ test::CreateNamedNode({ test::GenerateRandomByteArray<Key>(), "" }, "a") };

		// Act:
		auto result = RegisterAndExecutePushPeersHandler(nodes);

		// Assert:
		EXPECT_EQ(1u, result.second);
		EXPECT_EQ(1u, result.first.size());
		test::AssertEqualIdentities(test::ExtractNodeIdentities(nodes), test::ExtractNodeIdentities(result.first));
	}

	TEST(TEST_CLASS, PushPeersHandler_ForwardsMultipleEntityPayloadToConsumer) {
		// Arrange:
		std::vector<ionet::Node> nodes{
			test::CreateNamedNode({ test::GenerateRandomByteArray<Key>(), "" }, "a"),
			test::CreateNamedNode({ test::GenerateRandomByteArray<Key>(), "" }, "bc"),
			test::CreateNamedNode({ test::GenerateRandomByteArray<Key>(), "" }, "def")
		};

		// Act:
		auto result = RegisterAndExecutePushPeersHandler(nodes);

		// Assert:
		EXPECT_EQ(1u, result.second);
		EXPECT_EQ(3u, result.first.size());
		test::AssertEqualIdentities(test::ExtractNodeIdentities(nodes), test::ExtractNodeIdentities(result.first));
	}

	// endregion

	// region RegisterNodeDiscoveryPullPeersHandler

	namespace {
		template<typename TAssert>
		void RunPullPeersHandlerTest(const ionet::NodeSet& nodes, uint32_t packetExtraSize, TAssert assertFunc) {
			// Arrange:
			ionet::ServerPacketHandlers handlers;
			RegisterNodeDiscoveryPullPeersHandler(handlers, [nodes]() { return nodes; });

			// - create a valid request
			auto pPacket = ionet::CreateSharedPacket<ionet::Packet>();
			pPacket->Type = ionet::PacketType::Node_Discovery_Pull_Peers;
			pPacket->Size += packetExtraSize;

			// Act:
			ionet::ServerPacketHandlerContext handlerContext;
			EXPECT_TRUE(handlers.process(*pPacket, handlerContext));

			// Assert:
			assertFunc(handlerContext);
		}
	}

	TEST(TEST_CLASS, PullPeersHandler_DoesNotRespondToMalformedRequest) {
		// Arrange:
		ionet::NodeSet nodes{ test::CreateNamedNode(test::GenerateRandomByteArray<Key>(), "a") };

		// Act:
		RunPullPeersHandlerTest(nodes, 1, [](const auto& handlerContext) {
			// Assert: malformed packet is ignored
			test::AssertNoResponse(handlerContext);
		});
	}

	TEST(TEST_CLASS, PullPeersHandler_RespondsWhenNoNodesAreAvailable) {
		// Act:
		RunPullPeersHandlerTest(ionet::NodeSet(), 0, [](const auto& handlerContext) {
			// Assert: response with no nodes
			test::AssertPacketHeader(handlerContext, sizeof(ionet::Packet), ionet::PacketType::Node_Discovery_Pull_Peers);
			EXPECT_TRUE(handlerContext.response().buffers().empty());
		});
	}

	TEST(TEST_CLASS, PullPeersHandler_RespondsWhenSingleNodeIsAvailable) {
		// Arrange:
		ionet::NodeSet nodes{ test::CreateNamedNode(test::GenerateRandomByteArray<Key>(), "a") };
		auto networkNodes = test::PackAllNodes(nodes);

		// Act:
		RunPullPeersHandlerTest(nodes, 0, [&networkNodes](const auto& handlerContext) {
			// Assert: response with single node
			test::AssertPacketHeader(
					handlerContext,
					sizeof(ionet::Packet) + networkNodes[0]->Size,
					ionet::PacketType::Node_Discovery_Pull_Peers);

			const auto* pResponse = test::GetSingleBufferData(handlerContext);
			EXPECT_EQ_MEMORY(pResponse, networkNodes[0].get(), networkNodes[0]->Size);
		});
	}

	TEST(TEST_CLASS, PullPeersHandler_RespondsWhenMultipleNodesAreAvailable) {
		// Arrange:
		ionet::NodeSet nodes{
			test::CreateNamedNode(test::GenerateRandomByteArray<Key>(), "a"),
			test::CreateNamedNode(test::GenerateRandomByteArray<Key>(), "bc"),
			test::CreateNamedNode(test::GenerateRandomByteArray<Key>(), "def")
		};
		auto networkNodes = test::PackAllNodes(nodes);

		// Act:
		RunPullPeersHandlerTest(nodes, 0, [&networkNodes](const auto& handlerContext) mutable {
			// Assert: response with three nodes
			auto payloadSize = utils::Sum(networkNodes, [](const auto& pNetworkNode) { return pNetworkNode->Size; });
			test::AssertPacketHeader(handlerContext, sizeof(ionet::Packet) + payloadSize, ionet::PacketType::Node_Discovery_Pull_Peers);

			// - one buffer per node
			const auto& buffers = handlerContext.response().buffers();
			ASSERT_EQ(3u, buffers.size());

			// - each node in the (unordered) response corresponds to an original node
			for (auto i = 0u; i < 3; ++i) {
				const auto& responseNetworkNode = reinterpret_cast<const ionet::NetworkNode&>(*buffers[i].pData);
				auto networkNodeIter = std::find_if(networkNodes.cbegin(), networkNodes.cend(), [&responseNetworkNode](
						const auto& pNetworkNode) {
					return responseNetworkNode.IdentityKey == pNetworkNode->IdentityKey;
				});

				if (networkNodes.cend() != networkNodeIter) {
					const auto& networkNode = **networkNodeIter;
					EXPECT_EQ_MEMORY(buffers[i].pData, &networkNode, networkNode.Size) << "buffer at " << i;
					networkNodes.erase(networkNodeIter);
				}
			}

			EXPECT_TRUE(networkNodes.empty());
		});
	}

	// endregion
}}
