#include "nodediscovery/src/PeersProcessor.h"
#include "nodediscovery/src/NodePingRequestor.h"
#include "catapult/ionet/NodeContainer.h"
#include "catapult/utils/ArraySet.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace nodediscovery {

#define TEST_CLASS PeersProcessorTests

	namespace {
		constexpr auto Network_Identifier = model::NetworkIdentifier::Mijin_Test;

		// region MockNodePingRequestor

		class MockNodePingRequestor : public NodePingRequestor {
		public:
			utils::KeySet pingedIdentities() const {
				return m_pingedIdentities;
			}

			void setResponseNode(const Key& identityKey, const ionet::Node& responseNode) {
				m_pingResponses.emplace(identityKey, responseNode);
			}

		public:
			void requestPing(const ionet::Node& node, const PingCallback& callback) override {
				m_pingedIdentities.insert(node.identityKey());

				auto iter = m_pingResponses.find(node.identityKey());
				if (m_pingResponses.cend() == iter)
					callback(NodePingResult::Failure_Timeout, ionet::Node());
				else
					callback(NodePingResult::Success, iter->second);
			}

		public:
			size_t numActiveConnections() const override {
				CATAPULT_THROW_RUNTIME_ERROR("numActiveConnections - not supported in mock");
			}

			size_t numTotalPingRequests() const override {
				CATAPULT_THROW_RUNTIME_ERROR("numTotalPingRequests - not supported in mock");
			}

			size_t numSuccessfulPingRequests() const override {
				CATAPULT_THROW_RUNTIME_ERROR("numSuccessfulPingRequests - not supported in mock");
			}

			void shutdown() override {
				CATAPULT_THROW_RUNTIME_ERROR("shutdown - not supported in mock");
			}

		private:
			utils::KeySet m_pingedIdentities;
			std::unordered_map<Key, ionet::Node, utils::ArrayHasher<Key>> m_pingResponses;
		};

		// endregion

		auto CaptureNode(std::vector<ionet::Node>& nodes) {
			return [&nodes](const auto& node) {
				nodes.push_back(node);
			};
		}

		struct TestContext {
		public:
			TestContext() : Processor(NodeContainer, PingRequestor, Network_Identifier, CaptureNode(ResponseNodes))
			{}

		public:
			ionet::NodeContainer NodeContainer;
			MockNodePingRequestor PingRequestor;
			std::vector<ionet::Node> ResponseNodes;
			PeersProcessor Processor;
		};

		ionet::NodeMetadata CreateNamedMetadata(const std::string& name) {
			return ionet::NodeMetadata(Network_Identifier, name);
		}

		std::vector<ionet::Node> ToNodes(const std::vector<Key>& keys) {
			std::vector<ionet::Node> nodes;
			auto i = 0u;
			for (const auto& key : keys)
				nodes.emplace_back(key, ionet::NodeEndpoint(), CreateNamedMetadata(std::to_string(++i)));

			return nodes;
		}
	}

	TEST(TEST_CLASS, NoPingRequestsWhenCandidateNodesAreEmpty) {
		// Arrange:
		TestContext context;

		// Act: process an empty container
		context.Processor.process(ionet::NodeSet());

		// Assert: no pings and no responses
		EXPECT_TRUE(context.PingRequestor.pingedIdentities().empty());
		EXPECT_TRUE(context.ResponseNodes.empty());
	}

	TEST(TEST_CLASS, NoPingRequestsWhenAllCandidateNodesAreKnown) {
		// Arrange:
		TestContext context;

		// - add nodes to the node container
		auto keys = test::GenerateRandomDataVector<Key>(3);
		auto nodes = ToNodes(keys);
		for (const auto& node : nodes)
			context.NodeContainer.modifier().add(node, ionet::NodeSource::Dynamic);

		// Act: process nodes that are all known
		context.Processor.process(ionet::NodeSet(nodes.cbegin(), nodes.cend()));

		// Assert: no pings and no responses
		EXPECT_TRUE(context.PingRequestor.pingedIdentities().empty());
		EXPECT_TRUE(context.ResponseNodes.empty());
	}

	TEST(TEST_CLASS, NoResponseNodesWhenPingFails) {
		// Arrange:
		TestContext context;

		auto keys = test::GenerateRandomDataVector<Key>(3);
		auto nodes = ToNodes(keys);

		// Act: process unknown nodes (ping requestor returns failure for unconfigured nodes)
		context.Processor.process(ionet::NodeSet(nodes.cbegin(), nodes.cend()));

		// Assert: ping attempts but no successful responses
		EXPECT_EQ(utils::KeySet({ keys[0], keys[1], keys[2] }), context.PingRequestor.pingedIdentities());
		EXPECT_TRUE(context.ResponseNodes.empty());
	}

	TEST(TEST_CLASS, NoResponseNodesWhenResponseNodeIdentityIsIncompatible) {
		// Arrange:
		TestContext context;

		auto key = test::GenerateRandomData<Key_Size>();
		auto candidateNode = test::CreateNamedNode(key, "candidate");

		// - configure the ping response node to have a different key
		context.PingRequestor.setResponseNode(key, test::CreateNamedNode(test::GenerateRandomData<Key_Size>(), "response"));

		// Act: process candidate node
		context.Processor.process(ionet::NodeSet{ candidateNode });

		// Assert: ping attempt but no successful response
		EXPECT_EQ(utils::KeySet({ key }), context.PingRequestor.pingedIdentities());
		EXPECT_TRUE(context.ResponseNodes.empty());
	}

	TEST(TEST_CLASS, NoResponseNodesWhenResponseNodeNetworkIsIncompatible) {
		// Arrange:
		TestContext context;

		auto key = test::GenerateRandomData<Key_Size>();
		auto candidateNode = test::CreateNamedNode(key, "candidate");

		// - configure the ping response node to have a different network (processor is configured with Mijin_Test)
		context.PingRequestor.setResponseNode(key, ionet::Node(key, candidateNode.endpoint(), ionet::NodeMetadata()));

		// Act: process candidate node
		context.Processor.process(ionet::NodeSet{ candidateNode });

		// Assert: ping attempt but no successful response
		EXPECT_EQ(utils::KeySet({ key }), context.PingRequestor.pingedIdentities());
		EXPECT_TRUE(context.ResponseNodes.empty());
	}

	TEST(TEST_CLASS, ResponseNodesWhenResponseNodeHasExplicitHost) {
		// Arrange:
		TestContext context;

		auto key = test::GenerateRandomData<Key_Size>();
		auto candidateNode = ionet::Node(key, { "alice.com", 987 }, CreateNamedMetadata("candidate"));

		// - create a different (but compatible response node)
		context.PingRequestor.setResponseNode(key, ionet::Node(key, { "bob.com", 123 }, CreateNamedMetadata("bobby")));

		// Act: process candidate node
		context.Processor.process(ionet::NodeSet{ candidateNode });

		// Assert: ping attempt and successful response
		EXPECT_EQ(utils::KeySet({ key }), context.PingRequestor.pingedIdentities());
		ASSERT_EQ(1u, context.ResponseNodes.size());

		const auto& responseNode = context.ResponseNodes[0];
		EXPECT_EQ(key, responseNode.identityKey()); // from both
		EXPECT_EQ("bob.com", responseNode.endpoint().Host); // from response
		EXPECT_EQ(123u, responseNode.endpoint().Port); // from response
		EXPECT_EQ("bobby", context.ResponseNodes[0].metadata().Name); // from response
	}

	TEST(TEST_CLASS, ResponseNodesWhenResponseNodeHasImplicitHost) {
		// Arrange:
		TestContext context;

		auto key = test::GenerateRandomData<Key_Size>();
		auto candidateNode = ionet::Node(key, { "alice.com", 987 }, CreateNamedMetadata("candidate"));

		// - create a different (but compatible response node)
		context.PingRequestor.setResponseNode(key, ionet::Node(key, { "", 123 }, CreateNamedMetadata("bobby")));

		// Act: process candidate node
		context.Processor.process(ionet::NodeSet{ candidateNode });

		// Assert: ping attempt and successful response
		EXPECT_EQ(utils::KeySet({ key }), context.PingRequestor.pingedIdentities());
		ASSERT_EQ(1u, context.ResponseNodes.size());

		const auto& responseNode = context.ResponseNodes[0];
		EXPECT_EQ(key, responseNode.identityKey()); // from both
		EXPECT_EQ("alice.com", responseNode.endpoint().Host); // from candidate
		EXPECT_EQ(987u, responseNode.endpoint().Port); // from candidate
		EXPECT_EQ("bobby", context.ResponseNodes[0].metadata().Name); // from response
	}

	TEST(TEST_CLASS, MultipleResponseNodesWhenMultipleCandidatesAreNewAndSuccessful) {
		// Arrange:
		TestContext context;

		// - add 2/5 nodes to the node container
		auto keys = test::GenerateRandomDataVector<Key>(5);
		auto nodes = ToNodes(keys);
		context.NodeContainer.modifier().add(nodes[2], ionet::NodeSource::Dynamic);
		context.NodeContainer.modifier().add(nodes[4], ionet::NodeSource::Dynamic);

		// - mark 3/5 nodes as successful (2/3 unknown and 1/2 known)
		context.PingRequestor.setResponseNode(keys[0], nodes[0]);
		context.PingRequestor.setResponseNode(keys[2], nodes[2]);
		context.PingRequestor.setResponseNode(keys[3], nodes[3]);

		// Act:
		context.Processor.process(ionet::NodeSet(nodes.cbegin(), nodes.cend()));

		// Assert: three pings (2/5 in node container were filtered out)
		EXPECT_EQ(utils::KeySet({ keys[0], keys[1], keys[3] }), context.PingRequestor.pingedIdentities());

		// - two successful nodes (only 2/3 new nodes are configured to ping successfully)
		ASSERT_EQ(2u, context.ResponseNodes.size());
		EXPECT_EQ(utils::KeySet({ keys[0], keys[3] }), test::ExtractNodeIdentities(context.ResponseNodes));
	}
}}
