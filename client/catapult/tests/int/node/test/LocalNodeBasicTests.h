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

#pragma once
#include "LocalNodeTestUtils.h"
#include "catapult/config/CatapultDataDirectory.h"
#include "catapult/io/IndexFile.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/other/mocks/MockBlockChangeSubscriber.h"
#include "tests/test/other/mocks/MockStateChangeSubscriber.h"

namespace catapult { namespace test {

	/// Container of basic tests for LocalNode.
	template<typename TTraits>
	class LocalNodeBasicTests {
	private:
		using TestContext = typename TTraits::LocalNodeTestContext;

	public:
		// region basic tests - boot / shutdown

		static void AssertCanBootLocalNodeWithoutPeers() {
			// Act: create the node with no peers (really it is a peer to itself)
			TestContext context(NodeFlag::Custom_Peers, {});

			context.waitForNumActiveReaders(0);
			auto stats = context.stats();
			auto nodes = context.localNode().nodes();

			// Assert: check stats
			EXPECT_EQ(0u, stats.NumActiveReaders);
			EXPECT_EQ(0u, stats.NumActiveWriters);
			TTraits::AssertBoot(stats);

			// - check nodes
			EXPECT_EQ(1u, nodes.size());
			auto expectedContents = BasicNodeDataContainer{ { context.publicKey(), "LOCAL", ionet::NodeSource::Local } };
			EXPECT_EQ(expectedContents, CollectAll(nodes));
		}

		static void AssertCanBootLocalNodeWithPeers() {
			// Act: create the node with custom peers
			TestContext context(NodeFlag::Custom_Peers | NodeFlag::With_Partner, {});

			context.waitForNumActiveWriters(1);
			auto stats = context.stats();
			auto nodes = context.localNode().nodes();

			// Assert: check stats
			EXPECT_EQ(0u, stats.NumActiveReaders);
			EXPECT_EQ(1u, stats.NumActiveWriters);
			TTraits::AssertBoot(stats);

			// - check nodes
			EXPECT_EQ(2u, nodes.size());
			auto expectedContents = BasicNodeDataContainer{
				{ context.publicKey(), "LOCAL", ionet::NodeSource::Local },
				{ context.partnerPublicKey(), "PARTNER", ionet::NodeSource::Static }
			};
			EXPECT_EQ(expectedContents, CollectAll(nodes));
		}

		static void AssertCanShutdownLocalNode() {
			// Arrange:
			TestContext context(NodeFlag::Regular);

			// Act:
			context.localNode().shutdown();

			// Assert:
			context.assertShutdown();
		}

		// endregion

		// region basic tests - other

		static void AssertLocalNodeSchedulesAllTasks() {
			// Act:
			TestContext context(NodeFlag::Regular);

			context.waitForNumScheduledTasks(TTraits::Num_Tasks);
			auto stats = context.stats();

			// Assert:
			EXPECT_EQ(TTraits::Num_Tasks, stats.NumScheduledTasks);
		}

		static void AssertNodeSubscriberIsWiredUpToNodeContainer() {
			// Arrange:
			TestContext context(NodeFlag::Regular);

			auto identity = model::NodeIdentity{ GenerateRandomByteArray<Key>(), "11.22.33.44" };
			auto node = ionet::Node(identity);

			// Sanity:
			EXPECT_FALSE(context.localNode().nodes().contains(identity));

			// Act:
			context.nodeSubscriber().notifyNode(node);

			// Assert:
			auto nodeContainerView = context.localNode().nodes();
			ASSERT_TRUE(nodeContainerView.contains(identity));
			EXPECT_EQ(ionet::NodeSource::Dynamic, nodeContainerView.getNodeInfo(identity).source());
		}

		// endregion

		// region basic tests - nemesis subscriptions

		static void AssertLocalNodeTriggersNemesisSubscribersAtHeightOne() {
			// Arrange:
			TestContext context(NodeFlag::Require_Explicit_Boot);

			// Act:
			const mocks::MockBlockChangeSubscriber* pBlockChangeSubscriberRaw = nullptr;
			const mocks::MockStateChangeSubscriber* pStateChangeSubscriberRaw = nullptr;
			auto pLocalNode = context.boot([&](auto& bootstrapper) {
				auto pBlockChangeSubscriber = std::make_unique<mocks::MockBlockChangeSubscriber>();
				pBlockChangeSubscriberRaw = pBlockChangeSubscriber.get();
				bootstrapper.subscriptionManager().addBlockChangeSubscriber(std::move(pBlockChangeSubscriber));

				auto pStateChangeSubscriber = std::make_unique<mocks::MockStateChangeSubscriber>();
				pStateChangeSubscriberRaw = pStateChangeSubscriber.get();
				bootstrapper.subscriptionManager().addStateChangeSubscriber(std::move(pStateChangeSubscriber));
			});

			// Assert:
			EXPECT_EQ(1u, pBlockChangeSubscriberRaw->blockElements().size());
			EXPECT_EQ(0u, pBlockChangeSubscriberRaw->dropBlocksAfterHeights().size());

			EXPECT_EQ(1u, pStateChangeSubscriberRaw->numScoreChanges());
			EXPECT_EQ(1u, pStateChangeSubscriberRaw->numStateChanges());

			auto dataDirectory = config::CatapultDataDirectory(context.dataDirectory());
			EXPECT_EQ(2u, io::IndexFile(dataDirectory.spoolDir("state_change").file("index_server.dat")).get());
			EXPECT_EQ(2u, io::IndexFile(dataDirectory.spoolDir("state_change").file("index.dat")).get());
			EXPECT_EQ(2u, io::IndexFile(dataDirectory.rootDir().file("commit_step.dat")).get());
		}

		static void AssertLocalNodeDoesNotTriggerNemesisSubscribersAtHeightTwo() {
			// Arrange: boot the local node initially
			TestContext context(NodeFlag::Require_Explicit_Boot);
			context.boot();
			context.reset();

			// Act: reboot it
			const mocks::MockBlockChangeSubscriber* pBlockChangeSubscriberRaw = nullptr;
			const mocks::MockStateChangeSubscriber* pStateChangeSubscriberRaw = nullptr;
			auto pLocalNode = context.boot([&](auto& bootstrapper) {
				auto pBlockChangeSubscriber = std::make_unique<mocks::MockBlockChangeSubscriber>();
				pBlockChangeSubscriberRaw = pBlockChangeSubscriber.get();
				bootstrapper.subscriptionManager().addBlockChangeSubscriber(std::move(pBlockChangeSubscriber));

				auto pStateChangeSubscriber = std::make_unique<mocks::MockStateChangeSubscriber>();
				pStateChangeSubscriberRaw = pStateChangeSubscriber.get();
				bootstrapper.subscriptionManager().addStateChangeSubscriber(std::move(pStateChangeSubscriber));
			});

			// Sanity:
			EXPECT_EQ(Height(1), pLocalNode->cache().createView().height());

			// Assert:
			EXPECT_EQ(0u, pBlockChangeSubscriberRaw->blockElements().size());
			EXPECT_EQ(0u, pBlockChangeSubscriberRaw->dropBlocksAfterHeights().size());

			EXPECT_EQ(0u, pStateChangeSubscriberRaw->numScoreChanges());
			EXPECT_EQ(0u, pStateChangeSubscriberRaw->numStateChanges());
		}

		static void AssertLocalNodeCannotBootWhenCacheAndStorageHeightsAreInconsistent() {
			// Arrange: boot with storage height (2) ahead of cache height (1)
			TestContext context(NodeFlag::Require_Explicit_Boot);
			auto boot = [&context]() {
				context.boot([](auto& bootstrapper) {
					auto pBlock = GenerateBlockWithTransactions(0, Height(2));
					bootstrapper.subscriptionManager().fileStorage().saveBlock(BlockToBlockElement(*pBlock));
				});
			};

			// Act + Assert:
			EXPECT_THROW(boot(), catapult_runtime_error);
		}

		// endregion
	};

#define MAKE_LOCAL_NODE_BASIC_TEST(TRAITS_NAME, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { \
		test::LocalNodeBasicTests<TRAITS_NAME>::Assert##TEST_NAME(); \
	}

#define DEFINE_LOCAL_NODE_BASIC_TESTS(TRAITS_NAME) \
	MAKE_LOCAL_NODE_BASIC_TEST(TRAITS_NAME, CanBootLocalNodeWithoutPeers) \
	MAKE_LOCAL_NODE_BASIC_TEST(TRAITS_NAME, CanBootLocalNodeWithPeers) \
	MAKE_LOCAL_NODE_BASIC_TEST(TRAITS_NAME, CanShutdownLocalNode) \
	MAKE_LOCAL_NODE_BASIC_TEST(TRAITS_NAME, LocalNodeSchedulesAllTasks) \
	MAKE_LOCAL_NODE_BASIC_TEST(TRAITS_NAME, NodeSubscriberIsWiredUpToNodeContainer) \
	MAKE_LOCAL_NODE_BASIC_TEST(TRAITS_NAME, LocalNodeTriggersNemesisSubscribersAtHeightOne) \
	MAKE_LOCAL_NODE_BASIC_TEST(TRAITS_NAME, LocalNodeDoesNotTriggerNemesisSubscribersAtHeightTwo) \
	MAKE_LOCAL_NODE_BASIC_TEST(TRAITS_NAME, LocalNodeCannotBootWhenCacheAndStorageHeightsAreInconsistent)
}}
