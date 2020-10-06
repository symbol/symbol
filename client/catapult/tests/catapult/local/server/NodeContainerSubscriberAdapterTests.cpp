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

#include "catapult/local/server/NodeContainerSubscriberAdapter.h"
#include "catapult/ionet/NodeContainer.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local {

#define TEST_CLASS NodeContainerSubscriberAdapterTests

	namespace {
		ionet::NodeContainer CreateNodeContainer(
				size_t maxNodes = std::numeric_limits<size_t>::max(),
				model::NodeIdentityEqualityStrategy equalityStrategy = model::NodeIdentityEqualityStrategy::Key_And_Host) {
			ionet::BanSettings banSettings;
			banSettings.DefaultBanDuration = utils::TimeSpan::FromHours(1);
			banSettings.MaxBannedNodes = 50;
			return ionet::NodeContainer(maxNodes, equalityStrategy, banSettings, []() { return Timestamp(0); }, [](auto) { return true; });
		}

		std::unique_ptr<subscribers::NodeSubscriber> CreateNodeContainerSubscriberAdapter(ionet::NodeContainer& nodes) {
			return local::CreateNodeContainerSubscriberAdapter(nodes, { "3", "1.2.3", "1.1.1.1.", "1000.2000.3000", "9.8.7" });
		}

		void AssertSingleNode(
				const ionet::NodeContainerView& nodesView,
				const model::NodeIdentity& identity,
				const std::string& endpointHost,
				const std::string& name,
				ionet::NodeSource source) {
			if (1 != nodesView.size())
				CATAPULT_THROW_INVALID_ARGUMENT_1("expected container to only have single node", nodesView.size());

			const ionet::Node* pLastNode = nullptr;
			const ionet::NodeInfo *pLastNodeInfo = nullptr;
			nodesView.forEach([&pLastNode, &pLastNodeInfo](const auto& node, const auto& nodeInfo) {
				pLastNode = &node;
				pLastNodeInfo = &nodeInfo;
			});

			ASSERT_TRUE(!!pLastNode);
			ASSERT_TRUE(!!pLastNodeInfo);

			EXPECT_EQ(identity.PublicKey, pLastNode->identity().PublicKey);
			EXPECT_EQ(identity.Host, pLastNode->identity().Host);

			EXPECT_EQ(endpointHost, pLastNode->endpoint().Host);
			EXPECT_EQ(name, pLastNode->metadata().Name);

			EXPECT_EQ(source, pLastNodeInfo->source());
		}
	}

	// region notifyNode

	namespace {
		void AssertNotifyNodeAddsDynamicNode(const std::string& identityHost) {
			// Arrange: create a container and register a connection state
			auto nodes = CreateNodeContainer();
			nodes.modifier().addConnectionStates(ionet::ServiceIdentifier(1), ionet::NodeRoles::Peer);

			auto identity = model::NodeIdentity{ test::GenerateRandomByteArray<Key>(), identityHost };
			auto node = test::CreateNamedNode(identity, "alice", ionet::NodeRoles::Peer);

			// Act: notify the node
			auto pSubscriber = CreateNodeContainerSubscriberAdapter(nodes);
			pSubscriber->notifyNode(node);

			// Assert: node was added (endpoint host comes from node)
			auto nodesView = nodes.view();
			EXPECT_EQ(1u, nodesView.size());
			AssertSingleNode(nodesView, identity, "", "alice", ionet::NodeSource::Dynamic);

			// - node has expected (auto registered) connection state
			const auto& nodeInfo = nodesView.getNodeInfo(identity);
			EXPECT_EQ(1u, nodeInfo.numConnectionStates());

			const auto* pConnectionState = nodeInfo.getConnectionState(ionet::ServiceIdentifier(1));
			ASSERT_TRUE(!!pConnectionState);
			EXPECT_EQ(0u, pConnectionState->Age);
		}
	}

	TEST(TEST_CLASS, NotifyNodeAddsDynamicNodeWithRemoteNetwork) {
		AssertNotifyNodeAddsDynamicNode("11.22.33.44");
	}

	TEST(TEST_CLASS, NotifyNodeAddsDynamicNodeWithLocalNetwork) {
		AssertNotifyNodeAddsDynamicNode("9.8.7.1");
	}

	// endregion

	// region notifyIncomingNode

	namespace {
		void AssertNotifyIncomingNodeAddsDynamicIncomingNode(const std::string& identityHost, const std::string& expectedIdentityHost) {
			// Arrange: create a container and register a connection state
			auto nodes = CreateNodeContainer();
			nodes.modifier().addConnectionStates(ionet::ServiceIdentifier(1), ionet::NodeRoles::Peer);

			auto identity = model::NodeIdentity{ test::GenerateRandomByteArray<Key>(), identityHost };

			// Act: notify the node
			auto pSubscriber = CreateNodeContainerSubscriberAdapter(nodes);
			auto notifyIncomingNodeResult = pSubscriber->notifyIncomingNode(identity, ionet::ServiceIdentifier(2));

			// Assert: node was added (endpoint host comes from identity)
			EXPECT_TRUE(notifyIncomingNodeResult);

			auto nodesView = nodes.view();
			EXPECT_EQ(1u, nodesView.size());

			auto adjustedIdentity = model::NodeIdentity{ identity.PublicKey, expectedIdentityHost };
			AssertSingleNode(nodesView, adjustedIdentity, expectedIdentityHost, "", ionet::NodeSource::Dynamic_Incoming);

			// - node has expected (incoming) connection state
			const auto& nodeInfo = nodesView.getNodeInfo(adjustedIdentity);
			EXPECT_EQ(1u, nodeInfo.numConnectionStates());

			const auto* pConnectionState = nodeInfo.getConnectionState(ionet::ServiceIdentifier(2));
			ASSERT_TRUE(!!pConnectionState);
			EXPECT_EQ(1u, pConnectionState->Age);
		}
	}

	TEST(TEST_CLASS, NotifyIncomingNodeAddsDynamicIncomingNodeWithRemoteNetwork) {
		AssertNotifyIncomingNodeAddsDynamicIncomingNode("11.22.33.44", "11.22.33.44");
	}

	TEST(TEST_CLASS, NotifyIncomingNodeAddsDynamicIncomingNodeWithLocalNetwork) {
		AssertNotifyIncomingNodeAddsDynamicIncomingNode("9.8.7.1", "127.0.0.1");
	}

	// endregion

	// region notifyNode / notifyIncomingNode combinations

	namespace {
		void AssertMergedConnectionStates(const ionet::NodeInfo& nodeInfo) {
			EXPECT_EQ(2u, nodeInfo.numConnectionStates());

			const auto* pConnectionState1 = nodeInfo.getConnectionState(ionet::ServiceIdentifier(1));
			ASSERT_TRUE(!!pConnectionState1);
			EXPECT_EQ(0u, pConnectionState1->Age);

			const auto* pConnectionState2 = nodeInfo.getConnectionState(ionet::ServiceIdentifier(2));
			ASSERT_TRUE(!!pConnectionState2);
			EXPECT_EQ(1u, pConnectionState2->Age);
		}
	}

	TEST(TEST_CLASS, NotifyNodeCanBeFollowedByNotifyIncomingNode) {
		// Arrange: create a container and register a connection state
		auto nodes = CreateNodeContainer();
		nodes.modifier().addConnectionStates(ionet::ServiceIdentifier(1), ionet::NodeRoles::Peer);

		auto identity = model::NodeIdentity{ test::GenerateRandomByteArray<Key>(), "11.22.33.44" };
		auto node = test::CreateNamedNode(identity, "alice", ionet::NodeRoles::Peer);

		// Act: notify the node
		auto pSubscriber = CreateNodeContainerSubscriberAdapter(nodes);
		pSubscriber->notifyNode(node);
		auto notifyIncomingNodeResult = pSubscriber->notifyIncomingNode(identity, ionet::ServiceIdentifier(2));

		// Assert: node was added (endpoint host comes from node)
		EXPECT_TRUE(notifyIncomingNodeResult);

		auto nodesView = nodes.view();
		EXPECT_EQ(1u, nodesView.size());
		AssertSingleNode(nodesView, identity, "", "alice", ionet::NodeSource::Dynamic);

		// - node has merged original and new connection states
		AssertMergedConnectionStates(nodesView.getNodeInfo(identity));
	}

	TEST(TEST_CLASS, NotifyNodeCanBeFollowedByNotifyIncomingNodeWithHostChange) {
		// Arrange: create a container and register a connection state
		auto nodes = CreateNodeContainer(5, model::NodeIdentityEqualityStrategy::Host);
		nodes.modifier().addConnectionStates(ionet::ServiceIdentifier(1), ionet::NodeRoles::Peer);

		auto identityPublicKey = test::GenerateRandomByteArray<Key>();
		auto identity1 = model::NodeIdentity{ identityPublicKey, "11.22.33.44" };
		auto identity2 = model::NodeIdentity{ identityPublicKey, "22.33.44.33" };
		auto node = test::CreateNamedNode(identity1, "alice", ionet::NodeRoles::Peer);

		// Act: notify the node
		auto pSubscriber = CreateNodeContainerSubscriberAdapter(nodes);
		pSubscriber->notifyNode(node);
		auto notifyIncomingNodeResult = pSubscriber->notifyIncomingNode(identity2, ionet::ServiceIdentifier(2));

		// Assert: node added but not migrated (downgraded source cannot trigger key migration)
		EXPECT_TRUE(notifyIncomingNodeResult);

		auto nodesView = nodes.view();
		EXPECT_EQ(2u, nodesView.size());

		// - node has only original connection state (downgrade doesn't allow update)
		const auto& nodeInfo = nodesView.getNodeInfo(identity2);
		EXPECT_EQ(1u, nodeInfo.numConnectionStates());

		const auto* pConnectionState2 = nodeInfo.getConnectionState(ionet::ServiceIdentifier(2));
		ASSERT_TRUE(!!pConnectionState2);
		EXPECT_EQ(1u, pConnectionState2->Age);
	}

	TEST(TEST_CLASS, NotifyIncomingNodeCanBeFollowedByNotifyNode) {
		// Arrange: create a container and register a connection state
		auto nodes = CreateNodeContainer();
		nodes.modifier().addConnectionStates(ionet::ServiceIdentifier(1), ionet::NodeRoles::Peer);

		auto identity = model::NodeIdentity{ test::GenerateRandomByteArray<Key>(), "11.22.33.44" };
		auto node = test::CreateNamedNode(identity, "alice", ionet::NodeRoles::Peer);

		// Act: notify the node
		auto pSubscriber = CreateNodeContainerSubscriberAdapter(nodes);
		auto notifyIncomingNodeResult = pSubscriber->notifyIncomingNode(identity, ionet::ServiceIdentifier(2));
		pSubscriber->notifyNode(node);

		// Assert: node was added (endpoint host comes from node)
		EXPECT_TRUE(notifyIncomingNodeResult);

		auto nodesView = nodes.view();
		EXPECT_EQ(1u, nodesView.size());
		AssertSingleNode(nodesView, identity, "", "alice", ionet::NodeSource::Dynamic);

		// - node has merged original and new connection states
		AssertMergedConnectionStates(nodesView.getNodeInfo(identity));
	}

	TEST(TEST_CLASS, NotifyIncomingNodeCanBeFollowedByNotifyNodeWithHostChange) {
		// Arrange: create a container and register a connection state
		auto nodes = CreateNodeContainer(5, model::NodeIdentityEqualityStrategy::Host);
		nodes.modifier().addConnectionStates(ionet::ServiceIdentifier(1), ionet::NodeRoles::Peer);

		auto identityPublicKey = test::GenerateRandomByteArray<Key>();
		auto identity1 = model::NodeIdentity{ identityPublicKey, "11.22.33.44" };
		auto identity2 = model::NodeIdentity{ identityPublicKey, "22.33.44.33" };
		auto node1 = test::CreateNamedNode(identity1, "alice", ionet::NodeRoles::Peer);
		auto node2 = test::CreateNamedNode(identity2, "alice", ionet::NodeRoles::Peer);

		// - add the node to the container
		auto pSubscriber = CreateNodeContainerSubscriberAdapter(nodes);
		pSubscriber->notifyNode(node1);

		{
			// Sanity:
			auto nodesView = nodes.view();
			EXPECT_EQ(1u, nodesView.size());
			AssertSingleNode(nodesView, identity1, "", "alice", ionet::NodeSource::Dynamic);
		}

		// Act: notify the node and simulate a two phase identity change
		auto notifyIncomingNodeResult = pSubscriber->notifyIncomingNode(identity2, ionet::ServiceIdentifier(2));
		pSubscriber->notifyNode(node2);

		// Assert: node was added (endpoint host comes from node)
		EXPECT_TRUE(notifyIncomingNodeResult);

		auto nodesView = nodes.view();
		EXPECT_EQ(1u, nodesView.size());
		AssertSingleNode(nodesView, identity2, "", "alice", ionet::NodeSource::Dynamic);

		// - node has merged original and new connection states
		AssertMergedConnectionStates(nodesView.getNodeInfo(identity2));
	}

	// endregion

	// region notifyNode / notifyIncomingNode full node container

	namespace {
		void FillNodeContainer(ionet::NodeContainer& nodes, size_t maxNodes) {
			// Arrange: fill the node container with static nodes that are ineligible for pruning
			auto modifier = nodes.modifier();
			for (auto i = 0u; i < maxNodes; ++i) {
				auto key = test::GenerateRandomByteArray<Key>();
				auto node = test::CreateNamedNode(key, "alice" + std::to_string(i), ionet::NodeRoles::Peer);
				modifier.add(node, ionet::NodeSource::Static);
			}
		}
	}

	TEST(TEST_CLASS, NotifyNodeDoesNotAddNodeWhenContainerIsFull) {
		// Arrange: create a full container and register a connection state
		auto nodes = CreateNodeContainer(3);
		FillNodeContainer(nodes, 3);
		nodes.modifier().addConnectionStates(ionet::ServiceIdentifier(1), ionet::NodeRoles::Peer);

		auto identity = model::NodeIdentity{ test::GenerateRandomByteArray<Key>(), "11.22.33.44" };
		auto node = test::CreateNamedNode(identity, "alice", ionet::NodeRoles::Peer);

		// Act: notify the node
		auto pSubscriber = CreateNodeContainerSubscriberAdapter(nodes);
		pSubscriber->notifyNode(node);

		// Assert: node was not added
		auto nodesView = nodes.view();
		EXPECT_EQ(3u, nodesView.size());
		EXPECT_FALSE(nodesView.contains(identity));
	}

	TEST(TEST_CLASS, NotifyIncomingNodeDoesNotAddNodeWhenContainerIsFull) {
		// Arrange: create a full container and register a connection state
		auto nodes = CreateNodeContainer(3);
		FillNodeContainer(nodes, 3);
		nodes.modifier().addConnectionStates(ionet::ServiceIdentifier(1), ionet::NodeRoles::Peer);

		auto identity = model::NodeIdentity{ test::GenerateRandomByteArray<Key>(), "11.22.33.44" };

		// Act: notify the node
		auto pSubscriber = CreateNodeContainerSubscriberAdapter(nodes);
		auto notifyIncomingNodeResult = pSubscriber->notifyIncomingNode(identity, ionet::ServiceIdentifier(2));

		// Assert: node was not added
		EXPECT_FALSE(notifyIncomingNodeResult);

		auto nodesView = nodes.view();
		EXPECT_EQ(3u, nodesView.size());
		EXPECT_FALSE(nodesView.contains(identity));
	}

	// endregion

	// region isBanned

	TEST(TEST_CLASS, NotifyBanAddsNodeToBanList) {
		// Arrange:
		auto nodes = CreateNodeContainer();
		auto identity = model::NodeIdentity{ test::GenerateRandomByteArray<Key>(), "11.22.33.44" };

		// Sanity:
		EXPECT_FALSE(nodes.view().isBanned(identity));

		// Act:
		auto pSubscriber = CreateNodeContainerSubscriberAdapter(nodes);
		pSubscriber->notifyBan(identity, 123);

		// Assert:
		auto nodesView = nodes.view();
		EXPECT_EQ(0u, nodesView.size());
		EXPECT_TRUE(nodesView.isBanned(identity));
	}

	TEST(TEST_CLASS, NotifyBanForwardsNodeIdentityToBannedNodeIdentitySink) {
		// Arrange:
		auto nodes = CreateNodeContainer();
		auto identity = model::NodeIdentity{ test::GenerateRandomByteArray<Key>(), "11.22.33.44" };
		std::vector<model::NodeIdentity> bannedNodeIdentities;
		extensions::BannedNodeIdentitySink bannedNodeIdentitySink = [&bannedNodeIdentities](const auto& bannedNodeIdentity) {
			bannedNodeIdentities.push_back(bannedNodeIdentity);
		};

		// Act:
		auto pSubscriber = CreateNodeContainerSubscriberAdapter(nodes, {}, bannedNodeIdentitySink);
		pSubscriber->notifyBan(identity, 123);

		// Assert:
		ASSERT_EQ(1u, bannedNodeIdentities.size());
		EXPECT_EQ(identity.PublicKey, bannedNodeIdentities[0].PublicKey);
		EXPECT_EQ(identity.Host, bannedNodeIdentities[0].Host);
	}

	// endregion
}}
