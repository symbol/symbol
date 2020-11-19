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

#include "nodediscovery/src/api/RemoteNodeApi.h"
#include "nodediscovery/tests/test/NodeDiscoveryTestUtils.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/test/other/RemoteApiTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace api {

	namespace {
		struct NodeInfoTraits {
			static auto Invoke(const RemoteNodeApi& api) {
				return api.nodeInfo();
			}

			static auto CreateValidResponsePacket() {
				auto pPacket = test::CreateNodePushPingPacket(Key{ { 12 } }, ionet::NodeVersion(1234), "alice.com", "xyz");
				pPacket->Type = ionet::PacketType::Node_Discovery_Pull_Ping;
				return pPacket;
			}

			static auto CreateMalformedResponsePacket() {
				// just change the size because no responses are intrinsically invalid
				auto pResponsePacket = CreateValidResponsePacket();
				--pResponsePacket->Size;
				return pResponsePacket;
			}

			static void ValidateRequest(const ionet::Packet& packet) {
				EXPECT_TRUE(ionet::IsPacketValid(packet, ionet::PacketType::Node_Discovery_Pull_Ping));
			}

			static void ValidateResponse(const ionet::Packet&, const ionet::Node& node) {
				EXPECT_EQ(Key{ { 12 } }, node.identity().PublicKey);
				EXPECT_EQ("", node.identity().Host);
				EXPECT_EQ("alice.com", node.endpoint().Host);
				EXPECT_EQ(ionet::NodeVersion(1234), node.metadata().Version);
				EXPECT_EQ("xyz", node.metadata().Name);
			}
		};

		struct PeersInfoTraits {
			static auto Invoke(const RemoteNodeApi& api) {
				return api.peersInfo();
			}

			static auto CreateValidResponsePacket() {
				auto pPacket = test::CreateNodePushPeersPacket({
					test::CreateNamedNode(Key{ { 12 } }, "alice"),
					test::CreateNamedNode(Key{ { 37 } }, "charlie"),
					test::CreateNamedNode(Key{ { 25 } }, "bob")
				});
				pPacket->Type = ionet::PacketType::Node_Discovery_Pull_Peers;
				return pPacket;
			}

			static auto CreateMalformedResponsePacket() {
				// just change the size because no responses are intrinsically invalid
				auto pResponsePacket = CreateValidResponsePacket();
				--pResponsePacket->Size;
				return pResponsePacket;
			}

			static void ValidateRequest(const ionet::Packet& packet) {
				EXPECT_TRUE(ionet::IsPacketValid(packet, ionet::PacketType::Node_Discovery_Pull_Peers));
			}

			static void ValidateResponse(const ionet::Packet&, const ionet::NodeSet& nodes) {
				ASSERT_EQ(3u, nodes.size());

				auto sortedNodes = std::vector<ionet::Node>(nodes.cbegin(), nodes.cend());
				std::sort(sortedNodes.begin(), sortedNodes.end(), [](const auto& lhs, const auto& rhs) {
					return lhs.identity().PublicKey[0] < rhs.identity().PublicKey[0];
				});

				EXPECT_EQ("alice", sortedNodes[0].metadata().Name);
				EXPECT_EQ(Key{ { 12 } }, sortedNodes[0].identity().PublicKey);
				EXPECT_EQ("", sortedNodes[0].identity().Host);

				EXPECT_EQ("bob", sortedNodes[1].metadata().Name);
				EXPECT_EQ(Key{ { 25 } }, sortedNodes[1].identity().PublicKey);
				EXPECT_EQ("", sortedNodes[1].identity().Host);

				EXPECT_EQ("charlie", sortedNodes[2].metadata().Name);
				EXPECT_EQ(Key{ { 37 } }, sortedNodes[2].identity().PublicKey);
				EXPECT_EQ("", sortedNodes[2].identity().Host);
			}
		};

		struct RemoteNodeApiTraits {
			static auto Create(ionet::PacketIo& packetIo) {
				return CreateRemoteNodeApi(packetIo);
			}
		};
	}

	DEFINE_REMOTE_API_TESTS_EMPTY_RESPONSE_INVALID(RemoteNodeApi, NodeInfo)
	DEFINE_REMOTE_API_TESTS_EMPTY_RESPONSE_VALID(RemoteNodeApi, PeersInfo)
}}
