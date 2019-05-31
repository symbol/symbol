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

#include "catapult/ionet/NodeContainer.h"
#include "catapult/ionet/NodeInteractionResult.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/test/nodeps/LockTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace ionet {

#define TEST_CLASS NodeContainerTests

	// region test utils

	namespace {
		using BasicNodeDataContainer = test::BasicNodeDataContainer;

		bool AddUnchecked(
				NodeContainer& container,
				const Key& identityKey,
				const std::string& nodeName,
				NodeSource nodeSource,
				NodeRoles roles = NodeRoles::None) {
			return container.modifier().add(test::CreateNamedNode(identityKey, nodeName, roles), nodeSource);
		}

		void Add(
				NodeContainer& container,
				const Key& identityKey,
				const std::string& nodeName,
				NodeSource nodeSource,
				NodeRoles roles = NodeRoles::None) {
			// Act:
			auto addResult = AddUnchecked(container, identityKey, nodeName, nodeSource, roles);

			// Sanity:
			EXPECT_TRUE(addResult) << "add failed for: " << nodeName;
		}

		auto SeedThreeNodes(NodeContainer& container) {
			auto keys = test::GenerateRandomDataVector<Key>(3);
			Add(container, keys[0], "bob", NodeSource::Dynamic);
			Add(container, keys[1], "alice", NodeSource::Local);
			Add(container, keys[2], "charlie", NodeSource::Dynamic);
			return keys;
		}

		auto SeedFiveNodes(NodeContainer& container) {
			auto keys = SeedThreeNodes(container);
			keys.push_back(test::GenerateRandomByteArray<Key>());
			keys.push_back(test::GenerateRandomByteArray<Key>());
			Add(container, keys[3], "dolly", NodeSource::Dynamic);
			Add(container, keys[4], "ed", NodeSource::Static);
			return keys;
		}

		void AssertEmpty(const NodeContainer& container) {
			auto view = container.view();
			auto pairs = test::CollectAll(view);

			// Assert:
			EXPECT_EQ(0u, pairs.size());
			EXPECT_TRUE(pairs.empty());
		}
	}

	// endregion

	// region constructor

	TEST(TEST_CLASS, ContainerIsInitiallyEmpty) {
		// Act:
		NodeContainer container;

		// Assert:
		AssertEmpty(container);
	}

	// endregion

	// region time

	TEST(TEST_CLASS, DefaultContainerTimeIsAlwaysZero) {
		// Arrange:
		NodeContainer container;
		const auto& view = container.view();

		// Act:
		auto time1 = view.time();
		auto time2 = view.time();
		auto time3 = view.time();

		// Assert:
		EXPECT_EQ(Timestamp(), time1);
		EXPECT_EQ(Timestamp(), time2);
		EXPECT_EQ(Timestamp(), time3);
	}

	TEST(TEST_CLASS, ContainerTimeDelegatesToTimeSupplier) {
		// Arrange:
		auto numCalls = 0u;
		NodeContainer container(3, [&numCalls]() { return Timestamp(++numCalls * 2); });
		const auto& view = container.view();

		// Act:
		auto time1 = view.time();
		auto time2 = view.time();
		auto time3 = view.time();

		// Assert:
		EXPECT_EQ(3u, numCalls);
		EXPECT_EQ(Timestamp(2), time1);
		EXPECT_EQ(Timestamp(4), time2);
		EXPECT_EQ(Timestamp(6), time3);
	}

	// endregion

	// region contains

	TEST(TEST_CLASS, ContainsReturnsTrueWhenNodeIsKnown) {
		// Arrange:
		NodeContainer container;
		auto keys = test::GenerateRandomDataVector<Key>(5);

		// - seed 10 keys
		for (const auto& key : keys) {
			Add(container, key, "", NodeSource::Dynamic);
			Add(container, test::GenerateRandomByteArray<Key>(), "", NodeSource::Dynamic);
		}

		// Sanity:
		const auto& view = container.view();
		EXPECT_EQ(10u, view.size());

		// Act + Assert:
		for (const auto& key : keys)
			EXPECT_TRUE(view.contains(key));
	}

	TEST(TEST_CLASS, ContainsReturnsFalseWhenNodeIsUnknown) {
		// Arrange:
		NodeContainer container;

		// - seed 10 keys
		for (const auto& key : test::GenerateRandomDataVector<Key>(10))
			Add(container, key, "", NodeSource::Dynamic);

		// Sanity:
		const auto& view = container.view();
		EXPECT_EQ(10u, view.size());

		// Act + Assert:
		for (const auto& key : test::GenerateRandomDataVector<Key>(5))
			EXPECT_FALSE(view.contains(key));
	}

	// endregion

	// region add

	TEST(TEST_CLASS, CanAddSingleNode) {
		// Arrange:
		NodeContainer container;
		auto key = test::GenerateRandomByteArray<Key>();

		// Act:
		Add(container, key, "bob", NodeSource::Dynamic);

		// Assert:
		const auto& view = container.view();
		EXPECT_EQ(1u, view.size());

		auto expectedContents = BasicNodeDataContainer{ { key, "bob", NodeSource::Dynamic } };
		EXPECT_EQ(expectedContents, test::CollectAll(view));
	}

	TEST(TEST_CLASS, CanAddMultipleNodes) {
		// Arrange:
		NodeContainer container;
		auto keys = test::GenerateRandomDataVector<Key>(3);

		// Act:
		Add(container, keys[0], "bob", NodeSource::Dynamic);
		Add(container, keys[1], "alice", NodeSource::Local);
		Add(container, keys[2], "charlie", NodeSource::Dynamic);

		// Assert:
		const auto& view = container.view();
		EXPECT_EQ(3u, view.size());

		auto expectedContents = BasicNodeDataContainer{
			{ keys[0], "bob", NodeSource::Dynamic },
			{ keys[1], "alice", NodeSource::Local },
			{ keys[2], "charlie", NodeSource::Dynamic }
		};
		EXPECT_EQ(expectedContents, test::CollectAll(view));
	}

	TEST(TEST_CLASS, CanPromoteNodeSource) {
		// Arrange:
		NodeContainer container;
		auto key = test::GenerateRandomByteArray<Key>();

		// Act: promote from remote to local
		Add(container, key, "bob", NodeSource::Dynamic);
		Add(container, key, "bob2", NodeSource::Local);

		// Assert: promotion is allowed
		const auto& view = container.view();
		EXPECT_EQ(1u, view.size());

		auto expectedContents = BasicNodeDataContainer{ { key, "bob2", NodeSource::Local } };
		EXPECT_EQ(expectedContents, test::CollectAll(view));
	}

	TEST(TEST_CLASS, CannotDemoteNodeSource) {
		// Arrange:
		NodeContainer container;
		auto key = test::GenerateRandomByteArray<Key>();

		// Act: demote from local to remote
		Add(container, key, "bob", NodeSource::Local);
		Add(container, key, "bob2", NodeSource::Dynamic);

		// Assert: demotion is not allowed
		const auto& view = container.view();
		EXPECT_EQ(1u, view.size());

		auto expectedContents = BasicNodeDataContainer{ { key, "bob", NodeSource::Local } };
		EXPECT_EQ(expectedContents, test::CollectAll(view));
	}

	TEST(TEST_CLASS, NewerDataFromSameSourcePreemptsOlderData) {
		// Arrange:
		NodeContainer container;
		auto key = test::GenerateRandomByteArray<Key>();

		// Act: push a name change from the same source
		Add(container, key, "bob", NodeSource::Static);
		Add(container, key, "bob2", NodeSource::Static);

		// Assert: data from the new source is selected
		const auto& view = container.view();
		EXPECT_EQ(1u, view.size());

		auto expectedContents = BasicNodeDataContainer{ { key, "bob2", NodeSource::Static } };
		EXPECT_EQ(expectedContents, test::CollectAll(view));
	}

	// endregion

	// region add - pruning

	namespace {
		Timestamp ZeroTimeSupplier() {
			return Timestamp(0);
		}
	}

	TEST(TEST_CLASS, AddWhenFullRemovesOldestCandidateNode) {
		// Arrange:
		NodeContainer container(3, ZeroTimeSupplier);
		auto keys = test::GenerateRandomDataVector<Key>(4);

		// Act:
		Add(container, keys[0], "bob", NodeSource::Dynamic);
		Add(container, keys[1], "alice", NodeSource::Dynamic);
		Add(container, keys[2], "charlie", NodeSource::Dynamic);
		Add(container, keys[3], "doris", NodeSource::Dynamic);

		// Assert: bob is pruned because it is oldest node and candidate for pruning
		const auto& view = container.view();
		EXPECT_EQ(3u, view.size());

		auto expectedContents = BasicNodeDataContainer{
			{ keys[1], "alice", NodeSource::Dynamic },
			{ keys[2], "charlie", NodeSource::Dynamic },
			{ keys[3], "doris", NodeSource::Dynamic },
		};
		EXPECT_EQ(expectedContents, test::CollectAll(view));
	}

	TEST(TEST_CLASS, PromotionWhenFullDoesNotRemoveAnyNode) {
		// Arrange:
		NodeContainer container(3, ZeroTimeSupplier);
		auto keys = test::GenerateRandomDataVector<Key>(3);

		// Act:
		Add(container, keys[0], "bob", NodeSource::Dynamic);
		Add(container, keys[1], "alice", NodeSource::Dynamic);
		Add(container, keys[2], "charlie", NodeSource::Dynamic);
		Add(container, keys[1], "alice", NodeSource::Static);

		// Assert: no node is pruned because last add merely updates existing node
		const auto& view = container.view();
		EXPECT_EQ(3u, view.size());

		auto expectedContents = BasicNodeDataContainer{
			{ keys[0], "bob", NodeSource::Dynamic },
			{ keys[1], "alice", NodeSource::Static },
			{ keys[2], "charlie", NodeSource::Dynamic }
		};
		EXPECT_EQ(expectedContents, test::CollectAll(view));
	}

	TEST(TEST_CLASS, AddPruningPreservesNonDynamicNodes) {
		// Arrange:
		NodeContainer container(3, ZeroTimeSupplier);
		auto keys = test::GenerateRandomDataVector<Key>(4);

		// Act:
		Add(container, keys[0], "bob", NodeSource::Static);
		Add(container, keys[1], "alice", NodeSource::Local);
		Add(container, keys[2], "charlie", NodeSource::Dynamic);
		Add(container, keys[3], "doris", NodeSource::Dynamic);

		// Assert: charlie is pruned because it is the oldest dynamic node (static and local nodes are never pruned)
		const auto& view = container.view();
		EXPECT_EQ(3u, view.size());

		auto expectedContents = BasicNodeDataContainer{
			{ keys[0], "bob", NodeSource::Static },
			{ keys[1], "alice", NodeSource::Local },
			{ keys[3], "doris", NodeSource::Dynamic }
		};
		EXPECT_EQ(expectedContents, test::CollectAll(view));
	}

	TEST(TEST_CLASS, AddPruningPrunesNodesWithWorstSourceFirst) {
		// Arrange:
		NodeContainer container(3, ZeroTimeSupplier);
		auto keys = test::GenerateRandomDataVector<Key>(4);

		// Act:
		Add(container, keys[0], "bob", NodeSource::Dynamic);
		Add(container, keys[1], "alice", NodeSource::Dynamic_Incoming);
		Add(container, keys[2], "charlie", NodeSource::Dynamic_Incoming);
		Add(container, keys[3], "doris", NodeSource::Dynamic);

		// Assert: alice is pruned because it is the oldest node with the worst source
		const auto& view = container.view();
		EXPECT_EQ(3u, view.size());

		auto expectedContents = BasicNodeDataContainer{
			{ keys[0], "bob", NodeSource::Dynamic },
			{ keys[2], "charlie", NodeSource::Dynamic_Incoming },
			{ keys[3], "doris", NodeSource::Dynamic }
		};
		EXPECT_EQ(expectedContents, test::CollectAll(view));
	}

	TEST(TEST_CLASS, AddPruningPreservesNodesWithActiveConnections) {
		// Arrange:
		NodeContainer container(3, ZeroTimeSupplier);
		auto keys = test::GenerateRandomDataVector<Key>(4);

		// Act:
		Add(container, keys[0], "bob", NodeSource::Dynamic);
		container.modifier().provisionConnectionState(ServiceIdentifier(123), keys[0]).Age = 1;
		Add(container, keys[1], "alice", NodeSource::Dynamic);
		container.modifier().provisionConnectionState(ServiceIdentifier(123), keys[1]).Age = 1;
		Add(container, keys[2], "charlie", NodeSource::Dynamic);
		container.modifier().provisionConnectionState(ServiceIdentifier(123), keys[2]).Age = 0;
		Add(container, keys[3], "doris", NodeSource::Dynamic);

		// Assert: charlie is pruned because it is the oldest inactive node
		const auto& view = container.view();
		EXPECT_EQ(3u, view.size());

		auto expectedContents = BasicNodeDataContainer{
			{ keys[0], "bob", NodeSource::Dynamic },
			{ keys[1], "alice", NodeSource::Dynamic },
			{ keys[3], "doris", NodeSource::Dynamic }
		};
		EXPECT_EQ(expectedContents, test::CollectAll(view));
	}

	TEST(TEST_CLASS, AddHasNoEffectWhenThereAreNoPruneCandidates) {
		// Arrange:
		NodeContainer container(3, ZeroTimeSupplier);
		auto keys = test::GenerateRandomDataVector<Key>(4);

		// Act:
		Add(container, keys[0], "bob", NodeSource::Static);
		Add(container, keys[1], "alice", NodeSource::Static);
		Add(container, keys[2], "charlie", NodeSource::Static);
		auto addResult = AddUnchecked(container, keys[3], "doris", NodeSource::Static);

		// Assert: doris is not added because none of the nodes are prunable
		EXPECT_FALSE(addResult);

		const auto& view = container.view();
		EXPECT_EQ(3u, view.size());

		auto expectedContents = BasicNodeDataContainer{
			{ keys[0], "bob", NodeSource::Static },
			{ keys[1], "alice", NodeSource::Static },
			{ keys[2], "charlie", NodeSource::Static }
		};
		EXPECT_EQ(expectedContents, test::CollectAll(view));
	}

	// endregion

	// region getNodeInfo

	TEST(TEST_CLASS, NodeInfoIsInaccessibleForUnknownNode) {
		// Arrange:
		NodeContainer container;
		SeedThreeNodes(container);
		auto otherKey = test::GenerateRandomByteArray<Key>();

		// Act + Assert:
		EXPECT_THROW(container.view().getNodeInfo(otherKey), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, NodeInfoIsInitializedWhenNodeIsAdded) {
		// Arrange:
		NodeContainer container;
		auto key = test::GenerateRandomByteArray<Key>();

		// Act:
		Add(container, key, "bob", NodeSource::Dynamic);

		// Assert:
		const auto& view = container.view();
		EXPECT_EQ(1u, view.size());

		const auto& nodeInfo = view.getNodeInfo(key);
		EXPECT_EQ(NodeSource::Dynamic, nodeInfo.source());
		EXPECT_EQ(0u, nodeInfo.numConnectionStates());
	}

	TEST(TEST_CLASS, NodeInfoStateIsPreservedWhenSourceIsPromoted) {
		// Arrange:
		NodeContainer container;
		auto key = test::GenerateRandomByteArray<Key>();

		// - add an aged connection
		Add(container, key, "bob", NodeSource::Dynamic);
		container.modifier().provisionConnectionState(ServiceIdentifier(123), key).Age = 17;

		// Act: promote the node source
		Add(container, key, "bob", NodeSource::Static);

		// Assert:
		const auto& view = container.view();
		EXPECT_EQ(1u, view.size());

		const auto& nodeInfo = view.getNodeInfo(key);
		EXPECT_EQ(NodeSource::Static, nodeInfo.source());
		EXPECT_EQ(1u, nodeInfo.numConnectionStates());
		EXPECT_EQ(17u, nodeInfo.getConnectionState(ServiceIdentifier(123))->Age);
	}

	// endregion

	// region addConnectionStates

	TEST(TEST_CLASS, AddConnectionStatesHasNoEffectWhenNoExistingNodesHaveRequiredRole) {
		// Arrange:
		NodeContainer container;
		auto keys = SeedThreeNodes(container);

		// Act: add connection states *after* adding nodes
		container.modifier().addConnectionStates(ServiceIdentifier(123), NodeRoles::Api);

		// Assert:
		const auto& view = container.view();
		EXPECT_FALSE(!!view.getNodeInfo(keys[0]).getConnectionState(ServiceIdentifier(123)));
		EXPECT_FALSE(!!view.getNodeInfo(keys[1]).getConnectionState(ServiceIdentifier(123)));
		EXPECT_FALSE(!!view.getNodeInfo(keys[2]).getConnectionState(ServiceIdentifier(123)));
	}

	TEST(TEST_CLASS, AddConnectionStatesHasNoEffectWhenNoAddedNodesHaveRequiredRole) {
		// Arrange:
		NodeContainer container;

		// Act: add connection states *before* adding nodes
		container.modifier().addConnectionStates(ServiceIdentifier(123), NodeRoles::Api);
		auto keys = SeedThreeNodes(container);

		// Assert:
		const auto& view = container.view();
		EXPECT_FALSE(!!view.getNodeInfo(keys[0]).getConnectionState(ServiceIdentifier(123)));
		EXPECT_FALSE(!!view.getNodeInfo(keys[1]).getConnectionState(ServiceIdentifier(123)));
		EXPECT_FALSE(!!view.getNodeInfo(keys[2]).getConnectionState(ServiceIdentifier(123)));
	}

	namespace {
		auto SeedFiveNodesWithVaryingRoles(NodeContainer& container) {
			auto keys = test::GenerateRandomDataVector<Key>(5);
			Add(container, keys[0], "bob", NodeSource::Dynamic, NodeRoles::Api);
			Add(container, keys[1], "alice", NodeSource::Local, NodeRoles::Peer);
			Add(container, keys[2], "charlie", NodeSource::Dynamic, NodeRoles::None);
			Add(container, keys[3], "dolly", NodeSource::Dynamic, NodeRoles::Api | NodeRoles::Peer);
			Add(container, keys[4], "ed", NodeSource::Static, NodeRoles::Peer);
			return keys;
		}
	}

	TEST(TEST_CLASS, AddConnectionStatesAddsConnectionStatesToExistingNodesThatHaveRequiredRole) {
		// Arrange:
		NodeContainer container;
		auto keys = SeedFiveNodesWithVaryingRoles(container);

		// Act: add connection states *after* adding nodes
		container.modifier().addConnectionStates(ServiceIdentifier(123), NodeRoles::Api);

		// Assert:
		const auto& view = container.view();
		EXPECT_TRUE(!!view.getNodeInfo(keys[0]).getConnectionState(ServiceIdentifier(123)));
		EXPECT_FALSE(!!view.getNodeInfo(keys[1]).getConnectionState(ServiceIdentifier(123)));
		EXPECT_FALSE(!!view.getNodeInfo(keys[2]).getConnectionState(ServiceIdentifier(123)));
		EXPECT_TRUE(!!view.getNodeInfo(keys[3]).getConnectionState(ServiceIdentifier(123)));
		EXPECT_FALSE(!!view.getNodeInfo(keys[4]).getConnectionState(ServiceIdentifier(123)));
	}

	TEST(TEST_CLASS, AddConnectionStatesAddsConnectionStatesToAddedNodesThatHaveRequiredRole) {
		// Arrange:
		NodeContainer container;

		// Act: add connection states *before* adding nodes
		container.modifier().addConnectionStates(ServiceIdentifier(123), NodeRoles::Api);
		auto keys = SeedFiveNodesWithVaryingRoles(container);

		// Assert:
		const auto& view = container.view();
		EXPECT_TRUE(!!view.getNodeInfo(keys[0]).getConnectionState(ServiceIdentifier(123)));
		EXPECT_FALSE(!!view.getNodeInfo(keys[1]).getConnectionState(ServiceIdentifier(123)));
		EXPECT_FALSE(!!view.getNodeInfo(keys[2]).getConnectionState(ServiceIdentifier(123)));
		EXPECT_TRUE(!!view.getNodeInfo(keys[3]).getConnectionState(ServiceIdentifier(123)));
		EXPECT_FALSE(!!view.getNodeInfo(keys[4]).getConnectionState(ServiceIdentifier(123)));
	}

	TEST(TEST_CLASS, AddConnectionStatesAddsConnectionStatesToAddedNodesThatHaveUpgradedAndChangedRole) {
		// Arrange:
		NodeContainer container;

		// - add a service for api roles
		container.modifier().addConnectionStates(ServiceIdentifier(123), NodeRoles::Api);

		// - add a node that does not have matching roles
		auto key = test::GenerateRandomByteArray<Key>();
		Add(container, key, "bob", NodeSource::Dynamic, NodeRoles::Peer);

		// Sanity: the connection state is not present
		EXPECT_FALSE(!!container.view().getNodeInfo(key).getConnectionState(ServiceIdentifier(123)));

		// Act: promote the node with a changed (matching) role
		Add(container, key, "bob", NodeSource::Static, NodeRoles::Api);

		// Sanity: the connection state was added by promotion
		EXPECT_TRUE(!!container.view().getNodeInfo(key).getConnectionState(ServiceIdentifier(123)));
	}

	TEST(TEST_CLASS, AddConnectionStatesCanAddMultipleConnectionStatesToAddedMatchingNodes) {
		// Arrange:
		NodeContainer container;

		// - add multiple services
		container.modifier().addConnectionStates(ServiceIdentifier(123), NodeRoles::Api);
		container.modifier().addConnectionStates(ServiceIdentifier(124), NodeRoles::Peer);
		container.modifier().addConnectionStates(ServiceIdentifier(125), NodeRoles::None);
		container.modifier().addConnectionStates(ServiceIdentifier(126), NodeRoles::Api);

		// Act: add a node with matching roles
		auto key = test::GenerateRandomByteArray<Key>();
		Add(container, key, "bob", NodeSource::Dynamic, NodeRoles::Api);

		// Assert: connection states are present for the matching services (None matches everything)
		const auto& view = container.view();
		const auto& nodeInfo = view.getNodeInfo(key);
		EXPECT_TRUE(!!nodeInfo.getConnectionState(ServiceIdentifier(123)));
		EXPECT_FALSE(!!nodeInfo.getConnectionState(ServiceIdentifier(124)));
		EXPECT_TRUE(!!nodeInfo.getConnectionState(ServiceIdentifier(125)));
		EXPECT_TRUE(!!nodeInfo.getConnectionState(ServiceIdentifier(126)));
	}

	// endregion

	// region provisionConnectionState

	TEST(TEST_CLASS, ProvisionConnectionStateFailsWhenNodeIsUnknown) {
		// Arrange:
		NodeContainer container;
		SeedThreeNodes(container);
		auto otherKey = test::GenerateRandomByteArray<Key>();

		// Act + Assert:
		EXPECT_THROW(container.modifier().provisionConnectionState(ServiceIdentifier(123), otherKey), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, ProvisionConnectionStateAddsStateWhenNotPresent) {
		// Arrange:
		NodeContainer container;
		auto keys = SeedThreeNodes(container);

		// Act:
		const auto& connectionState = container.modifier().provisionConnectionState(ServiceIdentifier(123), keys[1]);

		// Assert:
		test::AssertZeroed(connectionState);
	}

	TEST(TEST_CLASS, ProvisionConnectionStateReturnsExistingStateWhenPresent) {
		// Arrange:
		NodeContainer container;
		auto keys = SeedThreeNodes(container);
		const auto& originalConnectionState = container.modifier().provisionConnectionState(ServiceIdentifier(123), keys[1]);

		// Act:
		const auto& connectionState = container.modifier().provisionConnectionState(ServiceIdentifier(123), keys[1]);

		// Assert:
		EXPECT_EQ(&originalConnectionState, &connectionState);
	}

	TEST(TEST_CLASS, ProvisionConnectionStateReturnsUniqueConnectionStatePerNode) {
		// Arrange:
		NodeContainer container;
		auto keys = SeedThreeNodes(container);

		// Act:
		const auto& connectionState1 = container.modifier().provisionConnectionState(ServiceIdentifier(123), keys[0]);
		const auto& connectionState2 = container.modifier().provisionConnectionState(ServiceIdentifier(123), keys[2]);

		// Assert:
		EXPECT_NE(&connectionState1, &connectionState2);
	}

	// endregion

	// region ageConnections

	TEST(TEST_CLASS, AgeConnectionsAgesZeroAgedMatchingConnections) {
		// Arrange:
		NodeContainer container;
		auto keys = SeedThreeNodes(container);
		{
			auto modifier = container.modifier();

			// Act:
			modifier.ageConnections(ServiceIdentifier(123), { keys[0], keys[2] });
		}

		// Assert: nodes { 0, 2 }  should have new state entries for id(123)
		auto view = container.view();
		EXPECT_EQ(1u, view.getNodeInfo(keys[0]).getConnectionState(ServiceIdentifier(123))->Age);
		EXPECT_FALSE(!!view.getNodeInfo(keys[1]).getConnectionState(ServiceIdentifier(123)));
		EXPECT_EQ(1u, view.getNodeInfo(keys[2]).getConnectionState(ServiceIdentifier(123))->Age);
	}

	TEST(TEST_CLASS, AgeConnectionsAgesNonzeroAgedMatchingConnections) {
		// Arrange:
		NodeContainer container;
		auto keys = SeedThreeNodes(container);
		{
			auto modifier = container.modifier();
			modifier.provisionConnectionState(ServiceIdentifier(123), keys[0]).Age = 1;
			modifier.provisionConnectionState(ServiceIdentifier(123), keys[1]).Age = 2;
			modifier.provisionConnectionState(ServiceIdentifier(123), keys[2]).Age = 3;

			// Act:
			modifier.ageConnections(ServiceIdentifier(123), { keys[0], keys[2] });
		}

		// Assert: nodes { 0, 2 } are aged, node { 1 } is cleared
		auto view = container.view();
		EXPECT_EQ(2u, view.getNodeInfo(keys[0]).getConnectionState(ServiceIdentifier(123))->Age);
		EXPECT_EQ(0u, view.getNodeInfo(keys[1]).getConnectionState(ServiceIdentifier(123))->Age);
		EXPECT_EQ(4u, view.getNodeInfo(keys[2]).getConnectionState(ServiceIdentifier(123))->Age);
	}

	TEST(TEST_CLASS, AgeConnectionsOnlyAffectsConnectionStatesWithMatchingIdentifiers) {
		// Arrange:
		NodeContainer container;
		auto keys = SeedThreeNodes(container);
		{
			auto modifier = container.modifier();
			modifier.provisionConnectionState(ServiceIdentifier(123), keys[0]).Age = 1;
			modifier.provisionConnectionState(ServiceIdentifier(123), keys[1]).Age = 2;
			modifier.provisionConnectionState(ServiceIdentifier(123), keys[2]).Age = 3;

			// Act:
			modifier.ageConnections(ServiceIdentifier(124), { keys[0], keys[2] });
		}

		// Assert:
		auto view = container.view();
		const auto& nodeInfo1 = view.getNodeInfo(keys[0]);
		const auto& nodeInfo2 = view.getNodeInfo(keys[1]);
		const auto& nodeInfo3 = view.getNodeInfo(keys[2]);

		// - nodes { 0, 2 }  should have new state entries for id(124)
		EXPECT_EQ(1u, nodeInfo1.getConnectionState(ServiceIdentifier(124))->Age);
		EXPECT_FALSE(!!nodeInfo2.getConnectionState(ServiceIdentifier(124)));
		EXPECT_EQ(1u, nodeInfo3.getConnectionState(ServiceIdentifier(124))->Age);

		// - no id(123) ages were changed
		EXPECT_EQ(1u, nodeInfo1.getConnectionState(ServiceIdentifier(123))->Age);
		EXPECT_EQ(2u, nodeInfo2.getConnectionState(ServiceIdentifier(123))->Age);
		EXPECT_EQ(3u, nodeInfo3.getConnectionState(ServiceIdentifier(123))->Age);

		// - each info has the correct number of states
		EXPECT_EQ(2u, nodeInfo1.numConnectionStates());
		EXPECT_EQ(1u, nodeInfo2.numConnectionStates());
		EXPECT_EQ(2u, nodeInfo3.numConnectionStates());
	}

	// endregion

	// region ageConnectionBans

	TEST(TEST_CLASS, AgeConnectionBansAgesMatchingConnections) {
		// Arrange:
		NodeContainer container;
		auto keys = SeedThreeNodes(container);
		{
			auto modifier = container.modifier();
			modifier.provisionConnectionState(ServiceIdentifier(123), keys[0]).NumConsecutiveFailures = 3;
			modifier.provisionConnectionState(ServiceIdentifier(123), keys[0]).BanAge = 11;
			modifier.provisionConnectionState(ServiceIdentifier(123), keys[1]).NumConsecutiveFailures = 4;
			modifier.provisionConnectionState(ServiceIdentifier(123), keys[1]).BanAge = 100;
			modifier.provisionConnectionState(ServiceIdentifier(123), keys[2]).NumConsecutiveFailures = 3;

			// Act:
			modifier.ageConnectionBans(ServiceIdentifier(123), 100, 3);
		}

		// Assert: all nodes are aged
		auto view = container.view();
		EXPECT_EQ(12u, view.getNodeInfo(keys[0]).getConnectionState(ServiceIdentifier(123))->BanAge);
		EXPECT_EQ(0u, view.getNodeInfo(keys[1]).getConnectionState(ServiceIdentifier(123))->BanAge);
		EXPECT_EQ(1u, view.getNodeInfo(keys[2]).getConnectionState(ServiceIdentifier(123))->BanAge);
	}

	TEST(TEST_CLASS, AgeConnectionBansOnlyAffectsConnectionStatesWithMatchingIdentifiers) {
		// Arrange:
		NodeContainer container;
		auto keys = SeedThreeNodes(container);
		{
			auto modifier = container.modifier();
			modifier.provisionConnectionState(ServiceIdentifier(124), keys[0]).NumConsecutiveFailures = 3;
			modifier.provisionConnectionState(ServiceIdentifier(124), keys[2]).NumConsecutiveFailures = 4;

			modifier.provisionConnectionState(ServiceIdentifier(123), keys[0]).NumConsecutiveFailures = 3;
			modifier.provisionConnectionState(ServiceIdentifier(123), keys[1]).NumConsecutiveFailures = 4;
			modifier.provisionConnectionState(ServiceIdentifier(123), keys[2]).NumConsecutiveFailures = 3;

			// Act:
			modifier.ageConnectionBans(ServiceIdentifier(124), 100, 3);
		}

		// Assert:
		auto view = container.view();
		const auto& nodeInfo1 = view.getNodeInfo(keys[0]);
		const auto& nodeInfo2 = view.getNodeInfo(keys[1]);
		const auto& nodeInfo3 = view.getNodeInfo(keys[2]);

		// - id(124) ban ages were incremented
		EXPECT_EQ(1u, nodeInfo1.getConnectionState(ServiceIdentifier(124))->BanAge);
		EXPECT_FALSE(!!nodeInfo2.getConnectionState(ServiceIdentifier(124)));
		EXPECT_EQ(1u, nodeInfo3.getConnectionState(ServiceIdentifier(124))->BanAge);

		// - no id(123) ban ages were not changed
		EXPECT_EQ(0u, nodeInfo1.getConnectionState(ServiceIdentifier(123))->BanAge);
		EXPECT_EQ(0u, nodeInfo2.getConnectionState(ServiceIdentifier(123))->BanAge);
		EXPECT_EQ(0u, nodeInfo3.getConnectionState(ServiceIdentifier(123))->BanAge);
	}

	// endregion

	// region incrementSuccesses / incrementFailures

	TEST(TEST_CLASS, NoIncrementWhenNodeIsNotFound) {
		// Arrange:
		auto identityKey = test::GenerateRandomByteArray<Key>();
		NodeContainer container;

		// Act:
		{
			auto modifier = container.modifier();
			modifier.incrementSuccesses(identityKey);
			modifier.incrementFailures(identityKey);
		}

		// Assert: no node was added to the container
		EXPECT_FALSE(container.view().contains(identityKey));
	}

	namespace {
		template<typename TIncrement>
		void AssertCanAddInteraction(uint32_t successesPerIncrement, uint32_t failuresPerIncrement, TIncrement increment) {
			// Arrange:
			auto time1 = Timestamp();
			auto time2 = Timestamp(NodeInteractionsContainer::BucketDuration().millis());
			auto time3 = Timestamp(NodeInteractionsContainer::InteractionDuration().millis());

			auto timestampIndex = 0u;
			std::vector<Timestamp> timestamps{ time1, time2, time2, time3, time3, time3 };

			auto identityKey = test::GenerateRandomByteArray<Key>();
			NodeContainer container(3, [&timestampIndex, &timestamps]() { return timestamps[timestampIndex++]; });
			Add(container, identityKey, "bob", NodeSource::Dynamic);

			// Act:
			{
				auto modifier = container.modifier();
				for (auto i = 0u; i < timestamps.size(); ++i)
					increment(modifier, identityKey);
			}

			// Assert: if pruning did not occur, these would not all be the same
			auto view = container.view();
			const auto& nodeInfo = view.getNodeInfo(identityKey);
			test::AssertNodeInteractions(5 * successesPerIncrement, 5 * failuresPerIncrement, nodeInfo.interactions(time1), "time 1");
			test::AssertNodeInteractions(5 * successesPerIncrement, 5 * failuresPerIncrement, nodeInfo.interactions(time2), "time 2");
			test::AssertNodeInteractions(5 * successesPerIncrement, 5 * failuresPerIncrement, nodeInfo.interactions(time3), "time 3");
		}
	}

	TEST(TEST_CLASS, IncrementsSuccessesOnSuccess) {
		// Assert:
		AssertCanAddInteraction(1, 0, [](auto& modifier, const auto& identityKey) { modifier.incrementSuccesses(identityKey); });
	}

	TEST(TEST_CLASS, IncrementsFailuresOnFailure) {
		// Assert:
		AssertCanAddInteraction(0, 1, [](auto& modifier, const auto& identityKey) { modifier.incrementFailures(identityKey); });
	}

	// endregion

	// region FindAllActiveNodes

	TEST(TEST_CLASS, FindAllActiveNodesReturnsEmptySetWhenNoNodesAreActive) {
		// Arrange:
		NodeContainer container;
		SeedFiveNodes(container);

		// Act:
		auto nodes = FindAllActiveNodes(container.view());

		// Assert:
		EXPECT_TRUE(nodes.empty());
	}

	TEST(TEST_CLASS, FindAllActiveNodesReturnsAllNodesWithAnyActiveConnection) {
		// Arrange:
		NodeContainer container;
		auto keys = SeedFiveNodes(container);
		{
			auto modifier = container.modifier();
			modifier.provisionConnectionState(ServiceIdentifier(111), keys[0]).Age = 1;
			modifier.provisionConnectionState(ServiceIdentifier(333), keys[2]).Age = 3;
			modifier.provisionConnectionState(ServiceIdentifier(111), keys[3]).Age = 0;
			modifier.provisionConnectionState(ServiceIdentifier(111), keys[4]).Age = 1;
		}

		// Act:
		auto nodes = FindAllActiveNodes(container.view());
		auto identities = test::ExtractNodeIdentities(nodes);

		// Assert:
		// - 0 => id(111)
		// - 1 => inactive
		// - 2 => id(333)
		// - 3 => inactive
		// - 4 => id(111)
		EXPECT_EQ(utils::KeySet({ keys[0], keys[2], keys[4] }), identities);
	}

	// endregion

	// region synchronization

	namespace {
		auto CreateLockProvider() {
			return std::make_unique<NodeContainer>();
		}
	}

	DEFINE_LOCK_PROVIDER_TESTS(TEST_CLASS)

	// endregion
}}
