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

#include "nodediscovery/src/PeersProcessor.h"
#include "catapult/ionet/NodeContainer.h"
#include "catapult/utils/ArraySet.h"
#include "nodediscovery/tests/test/NodeDiscoveryTestUtils.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace nodediscovery {

#define TEST_CLASS PeersProcessorTests

	namespace {
		// region MockNodePingRequestInitiator

		class MockNodePingRequestInitiator {
		private:
			using OperationCallback = consumer<net::NodeRequestResult, const ionet::Node&>;

		public:
			explicit MockNodePingRequestInitiator(
					model::NodeIdentityEqualityStrategy equalityStrategy = model::NodeIdentityEqualityStrategy::Key_And_Host)
					: m_pingedIdentities(model::CreateNodeIdentitySet(equalityStrategy))
					, m_pingResponses(model::CreateNodeIdentityMap<ionet::Node>(equalityStrategy))
			{}

		public:
			const auto& pingedIdentities() const {
				return m_pingedIdentities;
			}

			void setResponseNode(const model::NodeIdentity& identity, const ionet::Node& responseNode) {
				m_pingResponses.emplace(identity, responseNode);
			}

		public:
			void operator()(const ionet::Node& node, const OperationCallback& callback) {
				m_pingedIdentities.insert(node.identity());

				auto iter = m_pingResponses.find(node.identity());
				if (m_pingResponses.cend() == iter)
					callback(net::NodeRequestResult::Failure_Timeout, ionet::Node());
				else
					callback(net::NodeRequestResult::Success, iter->second);
			}

		public:
			consumer<const ionet::Node&, const OperationCallback&> ref() {
				return [this](const auto& node, const auto& callback) {
					return (*this)(node, callback);
				};
			}

		private:
			model::NodeIdentitySet m_pingedIdentities;
			model::NodeIdentityMap<ionet::Node> m_pingResponses;
		};

		// endregion

		auto CaptureNode(std::vector<ionet::Node>& nodes) {
			return [&nodes](const auto& node) {
				nodes.push_back(node);
			};
		}

		struct TestContext {
		public:
			TestContext()
					: ServerPublicKey(test::GenerateRandomByteArray<Key>())
					, Processor(
							ServerPublicKey,
							NodeContainer,
							PingRequestInitiator.ref(),
							test::CreateNodeDiscoveryNetworkFingerprint(),
							CaptureNode(ResponseNodes))
			{}

		public:
			Key ServerPublicKey;
			ionet::NodeContainer NodeContainer;
			MockNodePingRequestInitiator PingRequestInitiator;
			std::vector<ionet::Node> ResponseNodes;
			PeersProcessor Processor;
		};

		ionet::NodeMetadata CreateNamedMetadata(const std::string& name) {
			return ionet::NodeMetadata(test::CreateNodeDiscoveryNetworkFingerprint(), name);
		}

		std::vector<ionet::Node> ToNodes(const std::vector<model::NodeIdentity>& identities) {
			std::vector<ionet::Node> nodes;
			auto i = 0u;
			for (const auto& identity : identities)
				nodes.emplace_back(identity, ionet::NodeEndpoint(), CreateNamedMetadata(std::to_string(++i)));

			return nodes;
		}
	}

	TEST(TEST_CLASS, NoPingRequestsWhenCandidateNodesAreEmpty) {
		// Arrange:
		TestContext context;

		// Act: process an empty container
		context.Processor.process(ionet::NodeSet());

		// Assert: no pings and no responses
		EXPECT_TRUE(context.PingRequestInitiator.pingedIdentities().empty());
		EXPECT_TRUE(context.ResponseNodes.empty());
	}

	TEST(TEST_CLASS, NoPingRequestsWhenAllCandidateNodesAreKnown) {
		// Arrange:
		TestContext context;

		// - add nodes to the node container
		auto identities = std::vector<model::NodeIdentity>{
			{ test::GenerateRandomByteArray<Key>(), "11.22.33.44" },
			{ test::GenerateRandomByteArray<Key>(), "22.33.44.55" },
			{ test::GenerateRandomByteArray<Key>(), "33.44.55.66" }
		};
		auto nodes = ToNodes(identities);
		for (const auto& node : nodes)
			context.NodeContainer.modifier().add(node, ionet::NodeSource::Dynamic);

		// Act: process nodes that are all known
		context.Processor.process(ionet::NodeSet(nodes.cbegin(), nodes.cend()));

		// Assert: no pings and no responses
		EXPECT_TRUE(context.PingRequestInitiator.pingedIdentities().empty());
		EXPECT_TRUE(context.ResponseNodes.empty());
	}

	TEST(TEST_CLASS, NoPingRequestWhenCandidateNodeAndServerSharePublicKey) {
		// Arrange:
		TestContext context;

		auto identities = std::vector<model::NodeIdentity>{
			{ context.ServerPublicKey, "22.33.44.55" }
		};
		auto nodes = ToNodes(identities);

		// Act: process local node
		context.Processor.process(ionet::NodeSet(nodes.cbegin(), nodes.cend()));

		// Assert: no pings and no responses
		EXPECT_TRUE(context.PingRequestInitiator.pingedIdentities().empty());
		EXPECT_TRUE(context.ResponseNodes.empty());
	}

	TEST(TEST_CLASS, NoResponseNodesWhenPingFails) {
		// Arrange:
		TestContext context;

		auto identities = std::vector<model::NodeIdentity>{
			{ test::GenerateRandomByteArray<Key>(), "11.22.33.44" },
			{ test::GenerateRandomByteArray<Key>(), "22.33.44.55" },
			{ test::GenerateRandomByteArray<Key>(), "33.44.55.66" }
		};
		auto nodes = ToNodes(identities);

		// Act: process unknown nodes (ping requestor returns failure for unconfigured nodes)
		context.Processor.process(ionet::NodeSet(nodes.cbegin(), nodes.cend()));

		// Assert: ping attempts but no successful responses
		test::AssertEqualIdentities(test::ToIdentitiesSet(identities), context.PingRequestInitiator.pingedIdentities());
		EXPECT_TRUE(context.ResponseNodes.empty());
	}

	TEST(TEST_CLASS, NoResponseNodesWhenResponseNodeIdentityIsIncompatible) {
		// Arrange:
		TestContext context;

		auto identity = model::NodeIdentity{ test::GenerateRandomByteArray<Key>(), "11.22.33.44" };
		auto candidateNode = test::CreateNamedNode(identity, "candidate");

		// - configure the ping response node to have a different key
		context.PingRequestInitiator.setResponseNode(identity, test::CreateNamedNode(test::GenerateRandomByteArray<Key>(), "response"));

		// Act: process candidate node
		context.Processor.process(ionet::NodeSet{ candidateNode });

		// Assert: ping attempt but no successful response
		test::AssertEqualIdentities(test::ToIdentitiesSet({ identity }), context.PingRequestInitiator.pingedIdentities());
		EXPECT_TRUE(context.ResponseNodes.empty());
	}

	TEST(TEST_CLASS, NoResponseNodesWhenResponseNodeNetworkIsIncompatible) {
		// Arrange:
		TestContext context;

		auto identity = model::NodeIdentity{ test::GenerateRandomByteArray<Key>(), "11.22.33.44" };
		auto candidateNode = test::CreateNamedNode(identity, "candidate");

		// - configure the ping response node to have a different network (processor is configured with Private_Test)
		context.PingRequestInitiator.setResponseNode(identity, ionet::Node(identity, candidateNode.endpoint(), ionet::NodeMetadata()));

		// Act: process candidate node
		context.Processor.process(ionet::NodeSet{ candidateNode });

		// Assert: ping attempt but no successful response
		test::AssertEqualIdentities(test::ToIdentitiesSet({ identity }), context.PingRequestInitiator.pingedIdentities());
		EXPECT_TRUE(context.ResponseNodes.empty());
	}

	TEST(TEST_CLASS, ResponseNodesWhenResponseNodeHasExplicitHost) {
		// Arrange:
		TestContext context;

		auto identity = model::NodeIdentity{ test::GenerateRandomByteArray<Key>(), "11.22.33.44" };
		auto candidateNode = ionet::Node(identity, { "alice.com", 987 }, CreateNamedMetadata("candidate"));

		// - create a different (but compatible response node)
		context.PingRequestInitiator.setResponseNode(identity, ionet::Node(identity, { "bob.com", 123 }, CreateNamedMetadata("bobby")));

		// Act: process candidate node
		context.Processor.process(ionet::NodeSet{ candidateNode });

		// Assert: ping attempt and successful response
		test::AssertEqualIdentities(test::ToIdentitiesSet({ identity }), context.PingRequestInitiator.pingedIdentities());
		ASSERT_EQ(1u, context.ResponseNodes.size());

		const auto& responseNode = context.ResponseNodes[0];
		EXPECT_EQ(identity.PublicKey, responseNode.identity().PublicKey); // from both
		EXPECT_EQ("11.22.33.44", responseNode.identity().Host); // from candidate
		EXPECT_EQ("bob.com", responseNode.endpoint().Host); // from response
		EXPECT_EQ(123u, responseNode.endpoint().Port); // from response
		EXPECT_EQ("bobby", context.ResponseNodes[0].metadata().Name); // from response
	}

	TEST(TEST_CLASS, ResponseNodesWhenResponseNodeHasImplicitHost) {
		// Arrange:
		TestContext context;

		auto identity = model::NodeIdentity{ test::GenerateRandomByteArray<Key>(), "11.22.33.44" };
		auto candidateNode = ionet::Node(identity, { "alice.com", 987 }, CreateNamedMetadata("candidate"));

		// - create a different (but compatible response node)
		context.PingRequestInitiator.setResponseNode(identity, ionet::Node(identity, { "", 123 }, CreateNamedMetadata("bobby")));

		// Act: process candidate node
		context.Processor.process(ionet::NodeSet{ candidateNode });

		// Assert: ping attempt and successful response
		test::AssertEqualIdentities(test::ToIdentitiesSet({ identity }), context.PingRequestInitiator.pingedIdentities());
		ASSERT_EQ(1u, context.ResponseNodes.size());

		const auto& responseNode = context.ResponseNodes[0];
		EXPECT_EQ(identity.PublicKey, responseNode.identity().PublicKey); // from both
		EXPECT_EQ("11.22.33.44", responseNode.identity().Host); // from candidate
		EXPECT_EQ("alice.com", responseNode.endpoint().Host); // from candidate
		EXPECT_EQ(987u, responseNode.endpoint().Port); // from candidate
		EXPECT_EQ("bobby", context.ResponseNodes[0].metadata().Name); // from response
	}

	TEST(TEST_CLASS, MultipleResponseNodesWhenMultipleCandidatesAreNewAndSuccessful) {
		// Arrange:
		TestContext context;

		// - add 2/5 nodes to the node container
		auto identities = std::vector<model::NodeIdentity>{
			{ test::GenerateRandomByteArray<Key>(), "11.22.33.44" },
			{ test::GenerateRandomByteArray<Key>(), "22.33.44.55" },
			{ test::GenerateRandomByteArray<Key>(), "33.44.55.66" },
			{ context.ServerPublicKey, "44.55.66.77" },
			{ test::GenerateRandomByteArray<Key>(), "55.66.77.88" },
			{ test::GenerateRandomByteArray<Key>(), "66.77.88.99" }
		};
		auto nodes = ToNodes(identities);
		context.NodeContainer.modifier().add(nodes[2], ionet::NodeSource::Dynamic);
		context.NodeContainer.modifier().add(nodes[5], ionet::NodeSource::Dynamic);

		// - mark 3/5 nodes as successful (2/3 unknown and 1/2 known)
		context.PingRequestInitiator.setResponseNode(identities[0], nodes[0]);
		context.PingRequestInitiator.setResponseNode(identities[2], nodes[2]);
		context.PingRequestInitiator.setResponseNode(identities[4], nodes[4]);

		// Act:
		context.Processor.process(ionet::NodeSet(nodes.cbegin(), nodes.cend()));

		// Assert: three pings (2/5 in node container were filtered out)
		test::AssertEqualIdentities(
				test::ToIdentitiesSet({ identities[0], identities[1], identities[4] }),
				context.PingRequestInitiator.pingedIdentities());

		// - two successful nodes (only 2/3 new nodes are configured to ping successfully)
		ASSERT_EQ(2u, context.ResponseNodes.size());
		test::AssertEqualIdentities(
				test::ToIdentitiesSet({ identities[0], identities[4] }),
				test::ExtractNodeIdentities(context.ResponseNodes));
	}
}}
