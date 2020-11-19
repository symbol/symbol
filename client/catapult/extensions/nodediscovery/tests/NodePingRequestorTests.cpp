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

#include "nodediscovery/src/NodePingRequestor.h"
#include "nodediscovery/tests/test/NodeDiscoveryTestUtils.h"
#include "tests/test/net/BriefServerRequestorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace nodediscovery {

#define TEST_CLASS NodePingRequestorTests

	namespace {
		struct RequestorTestContext : public test::BriefServerRequestorTestContext<NodePingRequestor> {
		private:
			using BaseType = test::BriefServerRequestorTestContext<NodePingRequestor>;

		public:
			explicit RequestorTestContext(const utils::TimeSpan& timeout = utils::TimeSpan::FromMinutes(1))
					: BaseType(timeout, NodePingResponseCompatibilityChecker(test::CreateNodeDiscoveryNetworkFingerprint()))
			{}

		public:
			std::shared_ptr<ionet::Packet> createNodePingResponsePacket(ionet::NodeVersion version, const std::string& name) const {
				auto host = test::CreateLocalHostNodeEndpoint().Host;
				auto pPacket = test::CreateNodePushPingPacket(ServerPublicKey, version, host, name);
				pPacket->Type = ionet::PacketType::Node_Discovery_Pull_Ping;
				return pPacket;
			}
		};

		template<typename TAction>
		void RunConnectedTest(const RequestorTestContext& context, const std::shared_ptr<ionet::Packet>& pResponsePacket, TAction action) {
			// Act + Assert:
			test::RunBriefServerRequestorConnectedTest(context, pResponsePacket, action);
		}

		void AssertFailedConnection(const NodePingRequestor& requestor, const ionet::Node& responseNode) {
			// Assert:
			test::AssertBriefServerRequestorFailedConnection(requestor);
			EXPECT_EQ(Key(), responseNode.identity().PublicKey);
			EXPECT_EQ("", responseNode.identity().Host);
		}
	}

	TEST(TEST_CLASS, BeginRequestFailsWhenResponseNodeHasIncompatibleNetwork) {
		// Arrange: create a valid packet with a wrong network
		RequestorTestContext context;
		auto pPacket = context.createNodePingResponsePacket(ionet::NodeVersion(1234), "a");
		reinterpret_cast<ionet::NetworkNode&>(*pPacket->Data()).NetworkIdentifier = model::NetworkIdentifier::Zero;

		// Act:
		RunConnectedTest(context, pPacket, [](const auto& requestor, auto result, const auto& responseNode) {
			// Assert:
			EXPECT_EQ(net::NodeRequestResult::Failure_Incompatible, result);
			AssertFailedConnection(requestor, responseNode);
		});
	}

	TEST(TEST_CLASS, BeginRequestFailsWhenResponseNodeHasWrongIdentity) {
		// Arrange: create a valid packet with a wrong identity
		RequestorTestContext context;
		auto pPacket = context.createNodePingResponsePacket(ionet::NodeVersion(1234), "a");
		test::FillWithRandomData(reinterpret_cast<ionet::NetworkNode&>(*pPacket->Data()).IdentityKey);

		// Act:
		RunConnectedTest(context, pPacket, [](const auto& requestor, auto result, const auto& responseNode) {
			// Assert:
			EXPECT_EQ(net::NodeRequestResult::Failure_Incompatible, result);
			AssertFailedConnection(requestor, responseNode);
		});
	}

	TEST(TEST_CLASS, BeginRequestSucceedsWhenResponseNodeIsCompatible) {
		// Arrange:
		RequestorTestContext context;
		auto pPacket = context.createNodePingResponsePacket(ionet::NodeVersion(1234), "a");
		const auto& expectedIdentityKey = context.ServerPublicKey;

		// Act:
		RunConnectedTest(context, pPacket, [&expectedIdentityKey](const auto& requestor, auto result, const auto& responseNode) {
			// Assert:
			EXPECT_EQ(net::NodeRequestResult::Success, result);

			EXPECT_EQ(expectedIdentityKey, responseNode.identity().PublicKey);
			EXPECT_EQ("127.0.0.1", responseNode.identity().Host);
			EXPECT_EQ("127.0.0.1", responseNode.endpoint().Host);
			EXPECT_EQ(ionet::NodeVersion(1234), responseNode.metadata().Version);
			EXPECT_EQ("a", responseNode.metadata().Name);

			EXPECT_EQ(1u, requestor.numTotalRequests());
			EXPECT_EQ(1u, requestor.numSuccessfulRequests());
		});
	}
}}
