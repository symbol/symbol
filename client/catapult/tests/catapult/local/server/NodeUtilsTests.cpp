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

#include "catapult/local/server/NodeUtils.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/extensions/ProcessBootstrapper.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/test/other/MutableCatapultConfiguration.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local {

#define TEST_CLASS NodeUtilsTests

	// region SeedNodeContainer

	namespace {
		auto CreateBootstrapper(const config::CatapultConfiguration& config) {
			return extensions::ProcessBootstrapper(config, "", extensions::ProcessDisposition::Production, "bootstrapper");
		}

		auto CreateCatapultConfiguration(const std::string& bootKey, const std::string& host, const std::string& name) {
			test::MutableCatapultConfiguration config;
			config.Node.Local.Host = host;
			config.Node.Local.FriendlyName = name;
			config.User.BootKey = bootKey;
			return config.ToConst();
		}
	}

	TEST(TEST_CLASS, SeedNodeContainerAddsOnlyLocalNodeWhenThereAreNoStaticNodes) {
		// Arrange:
		auto hexPrivateKey = test::GenerateRandomHexString(2 * Key::Size);
		auto bootstrapper = CreateBootstrapper(CreateCatapultConfiguration(hexPrivateKey, "127.0.0.1", "LOCAL"));

		// Act:
		ionet::NodeContainer nodes;
		SeedNodeContainer(nodes, bootstrapper);

		// Assert:
		auto nodesView = nodes.view();
		EXPECT_EQ(1u, nodesView.size());
		auto expectedContents = test::BasicNodeDataContainer{
			{ crypto::KeyPair::FromString(hexPrivateKey).publicKey(), "LOCAL", ionet::NodeSource::Local }
		};
		EXPECT_EQ(expectedContents, test::CollectAll(nodesView));
	}

	TEST(TEST_CLASS, SeedNodeContainerAddsLocalAndStaticNodes) {
		// Arrange:
		auto hexPrivateKey = test::GenerateRandomHexString(2 * Key::Size);
		auto bootstrapper = CreateBootstrapper(CreateCatapultConfiguration(hexPrivateKey, "127.0.0.1", "LOCAL"));

		auto keys = test::GenerateRandomDataVector<Key>(3);
		bootstrapper.addStaticNodes({
			test::CreateNamedNode(keys[0], "alice"),
			test::CreateNamedNode(keys[1], "bob"),
			test::CreateNamedNode(keys[2], "charlie")
		});

		// Act:
		ionet::NodeContainer nodes;
		SeedNodeContainer(nodes, bootstrapper);

		// Assert:
		auto nodesView = nodes.view();
		EXPECT_EQ(4u, nodesView.size());
		auto expectedContents = test::BasicNodeDataContainer{
			{ crypto::KeyPair::FromString(hexPrivateKey).publicKey(), "LOCAL", ionet::NodeSource::Local },
			{ keys[0], "alice", ionet::NodeSource::Static },
			{ keys[1], "bob", ionet::NodeSource::Static },
			{ keys[2], "charlie", ionet::NodeSource::Static }
		};
		EXPECT_EQ(expectedContents, test::CollectAll(nodesView));
	}

	namespace {
		void SeedWithLengths(
				ionet::NodeContainer& nodes,
				const std::string& bootKey,
				const Key& peerKey,
				size_t localHostSize,
				size_t localNameSize,
				size_t peerHostSize,
				size_t peerNameSize) {
			// Arrange:
			CATAPULT_LOG(debug)
					<< "seed with lengths: " << localHostSize << ", " << localNameSize
					<< ", " << peerHostSize << ", " << peerNameSize;

			auto config = CreateCatapultConfiguration(bootKey, std::string(localHostSize, 'm'), std::string(localNameSize, 'l'));
			auto bootstrapper = CreateBootstrapper(config);

			auto peerEndpoint = ionet::NodeEndpoint{ std::string(peerHostSize, 'q'), 1234 };
			auto peerMetadata = ionet::NodeMetadata(model::NetworkIdentifier::Zero, std::string(peerNameSize, 'p'));
			bootstrapper.addStaticNodes({ ionet::Node(peerKey, peerEndpoint, peerMetadata) });

			// Act:
			SeedNodeContainer(nodes, bootstrapper);
		}
	}

	TEST(TEST_CLASS, SeedNodeContainerSucceedsWhenMaxStringLengthsAreUsed) {
		// Arrange:
		auto hexPrivateKey = test::GenerateRandomHexString(2 * Key::Size);
		auto peerKey = test::GenerateRandomByteArray<Key>();

		// Act:
		ionet::NodeContainer nodes;
		SeedWithLengths(nodes, hexPrivateKey, peerKey, 255, 255, 255, 255);

		// Assert:
		auto nodesView = nodes.view();
		EXPECT_EQ(2u, nodesView.size());
		auto expectedContents = test::BasicNodeDataContainer{
			{ crypto::KeyPair::FromString(hexPrivateKey).publicKey(), std::string(255, 'l'), ionet::NodeSource::Local },
			{ peerKey, std::string(255, 'p'), ionet::NodeSource::Static }
		};
		EXPECT_EQ(expectedContents, test::CollectAll(nodesView));
	}

	TEST(TEST_CLASS, SeedNodeContainerFailsWhenMaxStringLengthsAreExceeded) {
		// Arrange:
		auto hexPrivateKey = test::GenerateRandomHexString(2 * Key::Size);
		auto peerKey = test::GenerateRandomByteArray<Key>();
		ionet::NodeContainer nodes;

		// Act + Assert:
		EXPECT_THROW(SeedWithLengths(nodes, hexPrivateKey, peerKey, 256, 255, 255, 255), catapult_invalid_argument);
		EXPECT_THROW(SeedWithLengths(nodes, hexPrivateKey, peerKey, 255, 256, 255, 255), catapult_invalid_argument);
		EXPECT_THROW(SeedWithLengths(nodes, hexPrivateKey, peerKey, 255, 255, 256, 255), catapult_invalid_argument);
		EXPECT_THROW(SeedWithLengths(nodes, hexPrivateKey, peerKey, 255, 255, 255, 256), catapult_invalid_argument);
	}

	// endregion

	// region CreateNodeContainerSubscriberAdapter

	TEST(TEST_CLASS, NodeContainerSubscriberAdapter_NotifyNodeAddsDynamicNode) {
		// Arrange: create a container and register a connection state
		ionet::NodeContainer nodes;
		nodes.modifier().addConnectionStates(ionet::ServiceIdentifier(1), ionet::NodeRoles::Peer);

		auto key = test::GenerateRandomByteArray<Key>();
		auto node = test::CreateNamedNode(key, "alice", ionet::NodeRoles::Peer);

		// Act: notify the node
		auto pSubscriber = CreateNodeContainerSubscriberAdapter(nodes);
		pSubscriber->notifyNode(node);

		// Assert: node was added
		auto nodesView = nodes.view();
		EXPECT_EQ(1u, nodesView.size());
		auto expectedContents = test::BasicNodeDataContainer{ { key, "alice", ionet::NodeSource::Dynamic } };
		EXPECT_EQ(expectedContents, test::CollectAll(nodesView));

		// - node has expected (auto registered) connection state
		const auto& nodeInfo = nodesView.getNodeInfo(key);
		EXPECT_EQ(1u, nodeInfo.numConnectionStates());

		const auto* pConnectionState = nodeInfo.getConnectionState(ionet::ServiceIdentifier(1));
		ASSERT_TRUE(!!pConnectionState);
		EXPECT_EQ(0u, pConnectionState->Age);
	}

	TEST(TEST_CLASS, NodeContainerSubscriberAdapter_NotifyIncomingNodeAddsDynamicIncomingNode) {
		// Arrange: create a container and register a connection state
		ionet::NodeContainer nodes;
		nodes.modifier().addConnectionStates(ionet::ServiceIdentifier(1), ionet::NodeRoles::Peer);

		auto key = test::GenerateRandomByteArray<Key>();

		// Act: notify the node
		auto pSubscriber = CreateNodeContainerSubscriberAdapter(nodes);
		pSubscriber->notifyIncomingNode(key, ionet::ServiceIdentifier(2));

		// Assert: node was added
		auto nodesView = nodes.view();
		EXPECT_EQ(1u, nodesView.size());
		auto expectedContents = test::BasicNodeDataContainer{ { key, "", ionet::NodeSource::Dynamic_Incoming } };
		EXPECT_EQ(expectedContents, test::CollectAll(nodesView));

		// - node has expected (incoming) connection state
		const auto& nodeInfo = nodesView.getNodeInfo(key);
		EXPECT_EQ(1u, nodeInfo.numConnectionStates());

		const auto* pConnectionState = nodeInfo.getConnectionState(ionet::ServiceIdentifier(2));
		ASSERT_TRUE(!!pConnectionState);
		EXPECT_EQ(1u, pConnectionState->Age);
	}

	TEST(TEST_CLASS, NodeContainerSubscriberAdapter_NotifyNodeCanBeFollowedByNotifyIncomingNode) {
		// Arrange: create a container and register a connection state
		ionet::NodeContainer nodes;
		nodes.modifier().addConnectionStates(ionet::ServiceIdentifier(1), ionet::NodeRoles::Peer);

		auto key = test::GenerateRandomByteArray<Key>();
		auto node = test::CreateNamedNode(key, "alice", ionet::NodeRoles::Peer);

		// Act: notify the node
		auto pSubscriber = CreateNodeContainerSubscriberAdapter(nodes);
		pSubscriber->notifyNode(node);
		pSubscriber->notifyIncomingNode(key, ionet::ServiceIdentifier(2));

		// Assert: node was added
		auto nodesView = nodes.view();
		EXPECT_EQ(1u, nodesView.size());
		auto expectedContents = test::BasicNodeDataContainer{ { key, "alice", ionet::NodeSource::Dynamic } };
		EXPECT_EQ(expectedContents, test::CollectAll(nodesView));

		// - node has expected connection states
		const auto& nodeInfo = nodesView.getNodeInfo(key);
		EXPECT_EQ(2u, nodeInfo.numConnectionStates());

		const auto* pConnectionState1 = nodeInfo.getConnectionState(ionet::ServiceIdentifier(1));
		ASSERT_TRUE(!!pConnectionState1);
		EXPECT_EQ(0u, pConnectionState1->Age);

		const auto* pConnectionState2 = nodeInfo.getConnectionState(ionet::ServiceIdentifier(2));
		ASSERT_TRUE(!!pConnectionState2);
		EXPECT_EQ(1u, pConnectionState2->Age);
	}

	TEST(TEST_CLASS, NodeContainerSubscriberAdapter_NotifyIncomingNodeCanBeFollowedByNotifyNode) {
		// Arrange: create a container and register a connection state
		ionet::NodeContainer nodes;
		nodes.modifier().addConnectionStates(ionet::ServiceIdentifier(1), ionet::NodeRoles::Peer);

		auto key = test::GenerateRandomByteArray<Key>();
		auto node = test::CreateNamedNode(key, "alice", ionet::NodeRoles::Peer);

		// Act: notify the node
		auto pSubscriber = CreateNodeContainerSubscriberAdapter(nodes);
		pSubscriber->notifyIncomingNode(key, ionet::ServiceIdentifier(2));
		pSubscriber->notifyNode(node);

		// Assert: node was added
		auto nodesView = nodes.view();
		EXPECT_EQ(1u, nodesView.size());
		auto expectedContents = test::BasicNodeDataContainer{ { key, "alice", ionet::NodeSource::Dynamic } };
		EXPECT_EQ(expectedContents, test::CollectAll(nodesView));

		// - node has expected connection states
		const auto& nodeInfo = nodesView.getNodeInfo(key);
		EXPECT_EQ(2u, nodeInfo.numConnectionStates());

		const auto* pConnectionState1 = nodeInfo.getConnectionState(ionet::ServiceIdentifier(1));
		ASSERT_TRUE(!!pConnectionState1);
		EXPECT_EQ(0u, pConnectionState1->Age);

		const auto* pConnectionState2 = nodeInfo.getConnectionState(ionet::ServiceIdentifier(2));
		ASSERT_TRUE(!!pConnectionState2);
		EXPECT_EQ(1u, pConnectionState2->Age);
	}

	namespace {
		std::unique_ptr<ionet::NodeContainer> CreateFullNodeContainer() {
			// Arrange: fill the node container with static nodes that are ineligible for pruning
			auto pNodes = std::make_unique<ionet::NodeContainer>(3, []() { return Timestamp(0); });
			auto modifier = pNodes->modifier();
			for (auto i = 0u; i < 3u; ++i) {
				auto key = test::GenerateRandomByteArray<Key>();
				auto node = test::CreateNamedNode(key, "alice" + std::to_string(i), ionet::NodeRoles::Peer);
				modifier.add(node, ionet::NodeSource::Static);
			}

			return pNodes;
		}
	}

	TEST(TEST_CLASS, NodeContainerSubscriberAdapter_NotifyNodeDoesNotAddNodeWhenContainerIsFull) {
		// Arrange: create a full container and register a connection state
		auto pNodes = CreateFullNodeContainer();
		pNodes->modifier().addConnectionStates(ionet::ServiceIdentifier(1), ionet::NodeRoles::Peer);

		auto key = test::GenerateRandomByteArray<Key>();
		auto node = test::CreateNamedNode(key, "alice", ionet::NodeRoles::Peer);

		// Act: notify the node
		auto pSubscriber = CreateNodeContainerSubscriberAdapter(*pNodes);
		pSubscriber->notifyNode(node);

		// Assert: node was not added
		auto nodesView = pNodes->view();
		EXPECT_EQ(3u, nodesView.size());
		EXPECT_FALSE(nodesView.contains(key));
	}

	TEST(TEST_CLASS, NodeContainerSubscriberAdapter_NotifyIncomingNodeDoesNotAddNodeWhenContainerIsFull) {
		// Arrange: create a full container and register a connection state
		auto pNodes = CreateFullNodeContainer();
		pNodes->modifier().addConnectionStates(ionet::ServiceIdentifier(1), ionet::NodeRoles::Peer);

		auto key = test::GenerateRandomByteArray<Key>();

		// Act: notify the node
		auto pSubscriber = CreateNodeContainerSubscriberAdapter(*pNodes);
		pSubscriber->notifyIncomingNode(key, ionet::ServiceIdentifier(2));

		// Assert: node was not added
		auto nodesView = pNodes->view();
		EXPECT_EQ(3u, nodesView.size());
		EXPECT_FALSE(nodesView.contains(key));
	}

	// endregion
}}
