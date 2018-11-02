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

#include "catapult/extensions/PeersConnectionTasks.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/test/net/mocks/MockPacketWriters.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

#define TEST_CLASS PeersConnectionTasksTests

	// region utils

	namespace {
		config::NodeConfiguration::ConnectionsSubConfiguration CreateConfiguration() {
			return { 5, 8, 250, 2 };
		}

		void Add(ionet::NodeContainer& container, const Key& identityKey, const std::string& nodeName, ionet::NodeRoles roles) {
			container.modifier().add(test::CreateNamedNode(identityKey, nodeName, roles), ionet::NodeSource::Dynamic);
		}
	}

	// endregion

	// region CreateNodeAger

	TEST(TEST_CLASS, NodeAger_AgesMatchingConnections) {
		// Arrange:
		auto serviceId = ionet::ServiceIdentifier(123);
		ionet::NodeContainer container;
		auto keys = test::GenerateRandomDataVector<Key>(3);
		Add(container, keys[0], "bob", ionet::NodeRoles::Peer);
		Add(container, keys[1], "alice", ionet::NodeRoles::Peer);
		Add(container, keys[2], "charlie", ionet::NodeRoles::Peer);
		{
			auto modifier = container.modifier();
			modifier.provisionConnectionState(serviceId, keys[0]).Age = 1;
			modifier.provisionConnectionState(serviceId, keys[1]).Age = 2;
			modifier.provisionConnectionState(serviceId, keys[2]).Age = 3;
		}

		// Act:
		auto ager = CreateNodeAger(serviceId, CreateConfiguration(), container);
		ager({ keys[0], keys[2] });

		// Assert: nodes { 0, 2 } are aged, node { 1 } is cleared
		auto view = container.view();
		EXPECT_EQ(2u, view.getNodeInfo(keys[0]).getConnectionState(serviceId)->Age);
		EXPECT_EQ(0u, view.getNodeInfo(keys[1]).getConnectionState(serviceId)->Age);
		EXPECT_EQ(4u, view.getNodeInfo(keys[2]).getConnectionState(serviceId)->Age);
	}

	TEST(TEST_CLASS, NodeAger_AgesMatchingConnectionBans) {
		// Arrange:
		auto serviceId = ionet::ServiceIdentifier(123);
		ionet::NodeContainer container;
		auto keys = test::GenerateRandomDataVector<Key>(3);
		Add(container, keys[0], "bob", ionet::NodeRoles::Peer);
		Add(container, keys[1], "alice", ionet::NodeRoles::Peer);
		Add(container, keys[2], "charlie", ionet::NodeRoles::Peer);
		{
			auto modifier = container.modifier();
			modifier.provisionConnectionState(serviceId, keys[0]).NumConsecutiveFailures = 3;
			modifier.provisionConnectionState(serviceId, keys[0]).BanAge = 11;
			modifier.provisionConnectionState(serviceId, keys[1]).NumConsecutiveFailures = 3;
			modifier.provisionConnectionState(serviceId, keys[1]).BanAge = 100;
			modifier.provisionConnectionState(serviceId, keys[2]).NumConsecutiveFailures = 3;
		}

		auto config = CreateConfiguration();
		config.MaxConnectionBanAge = 100;
		config.NumConsecutiveFailuresBeforeBanning = 3;

		// Act:
		auto ager = CreateNodeAger(serviceId, config, container);
		ager({});

		// Assert: all banned nodes with matching service are aged irrespective of identities passed to ager
		auto view = container.view();
		EXPECT_EQ(12u, view.getNodeInfo(keys[0]).getConnectionState(serviceId)->BanAge);
		EXPECT_EQ(0u, view.getNodeInfo(keys[1]).getConnectionState(serviceId)->BanAge);
		EXPECT_EQ(1u, view.getNodeInfo(keys[2]).getConnectionState(serviceId)->BanAge);
	}

	// endregion

	// region CreateNodeSelector

	TEST(TEST_CLASS, CanCreateNodeSelector) {
		// Arrange:
		ionet::NodeContainer container;
		auto selector = CreateNodeSelector(ionet::ServiceIdentifier(1), ionet::NodeRoles::Api, CreateConfiguration(), container);

		// Act:
		auto result = selector();

		// Assert:
		EXPECT_TRUE(result.AddCandidates.empty());
		EXPECT_TRUE(result.RemoveCandidates.empty());
	}

	TEST(TEST_CLASS, CreateNodeSelectorProvisionsConnectionStatesForRoleCompatibleNodes) {
		// Arrange:
		ionet::NodeContainer container;
		auto keys = test::GenerateRandomDataVector<Key>(3);
		Add(container, keys[0], "bob", ionet::NodeRoles::Api);
		Add(container, keys[1], "alice", ionet::NodeRoles::Peer);
		Add(container, keys[2], "charlie", ionet::NodeRoles::Api | ionet::NodeRoles::Peer);
		auto serviceId = ionet::ServiceIdentifier(1);

		// Act:
		CreateNodeSelector(serviceId, ionet::NodeRoles::Api, CreateConfiguration(), container);

		// Assert:
		const auto& view = container.view();
		EXPECT_TRUE(!!view.getNodeInfo(keys[0]).getConnectionState(serviceId));
		EXPECT_FALSE(!!view.getNodeInfo(keys[1]).getConnectionState(serviceId));
		EXPECT_TRUE(!!view.getNodeInfo(keys[2]).getConnectionState(serviceId));
	}

	// endregion

	// region ConnectPeersTask: utils

	namespace {
		std::vector<ionet::Node> SeedAlternatingServiceNodes(
				ionet::NodeContainer& container,
				uint32_t numNodes,
				ionet::ServiceIdentifier evenServiceId,
				ionet::ServiceIdentifier oddServiceId) {
			std::vector<ionet::Node> nodes;
			auto modifier = container.modifier();
			for (auto i = 0u; i < numNodes; ++i) {
				auto identityKey = test::GenerateRandomData<Key_Size>();
				auto node = test::CreateNamedNode(identityKey, "node " + std::to_string(i));
				modifier.add(node, ionet::NodeSource::Dynamic);
				nodes.push_back(node);

				auto serviceId = 0 == i % 2 ? evenServiceId : oddServiceId;
				auto& connectionState = modifier.provisionConnectionState(serviceId, identityKey);
				connectionState.Age = i + 1;
				connectionState.NumAttempts = 10;
				connectionState.NumSuccesses = 7;
				connectionState.NumFailures = 3;
				connectionState.NumConsecutiveFailures = 1;
				connectionState.BanAge = 123;
			}

			return nodes;
		}

		void ConnectSyncAll(mocks::MockPacketWriters& writers, const ionet::NodeSet& nodes) {
			for (const auto& node : nodes)
				writers.connectSync(node);
		}

		void RunConnectPeersTask(
				ionet::NodeContainer& container,
				net::PacketWriters& packetWriters,
				ionet::ServiceIdentifier serviceId,
				const NodeSelector& selector = NodeSelector()) {
			// Act:
			auto task = selector
					? CreateConnectPeersTask(container, packetWriters, serviceId, CreateConfiguration(), selector)
					: CreateConnectPeersTask(container, packetWriters, serviceId, ionet::NodeRoles::Peer, CreateConfiguration());
			auto result = task.Callback().get();

			// Assert:
			EXPECT_EQ("connect peers task", task.Name);
			EXPECT_EQ(thread::TaskResult::Continue, result);
		}
	}

	// endregion

	// region ConnectPeersTask: connection aging

	namespace {
		template<typename TRunTask>
		void AssertMatchingServiceNodesAreAged(TRunTask runTask) {
			// Arrange: prepare a container with alternating matching service nodes
			auto serviceId = ionet::ServiceIdentifier(3);
			ionet::NodeContainer container;
			auto nodes = SeedAlternatingServiceNodes(container, 10, ionet::ServiceIdentifier(9), serviceId);

			// - indicate 3 / 5 matching service nodes are active
			mocks::MockPacketWriters writers;
			ConnectSyncAll(writers, { nodes[1], nodes[5], nodes[7] });

			// Act: run selection that returns neither add nor remove candidates
			runTask(container, writers, serviceId);

			// Assert: nodes associated with service 3 were aged, others were untouched
			auto i = 0u;
			auto view = container.view();
			std::vector<uint32_t> expectedAges{ 3, 0, 7, 9, 0 }; // only for matching service 3
			for (const auto& node : nodes) {
				auto message = "node at " + std::to_string(i);
				const auto& nodeInfo = view.getNodeInfo(node.identityKey());
				const auto* pConnectionState = nodeInfo.getConnectionState(serviceId);

				if (0 == i % 2) {
					EXPECT_FALSE(!!pConnectionState) << message;
				} else {
					ASSERT_TRUE(!!pConnectionState) << message;
					EXPECT_EQ(expectedAges[i / 2], pConnectionState->Age) << message;
				}

				++i;
			}
		}
	}

	TEST(TEST_CLASS, ConnectPeersTask_MatchingServiceNodesAreAged) {
		// Assert:
		AssertMatchingServiceNodesAreAged([](auto& container, auto& writers, auto serviceId) {
			RunConnectPeersTask(container, writers, serviceId);
		});
	}

	// endregion

	// region ConnectPeersTask: remove candidates

	namespace {
		template<typename TRunTask>
		void AssertRemoveCandidatesAreClosedInWriters(TRunTask runTask) {
			// Arrange: prepare a container with alternating matching service nodes
			auto serviceId = ionet::ServiceIdentifier(3);
			ionet::NodeContainer container;
			auto nodes = SeedAlternatingServiceNodes(container, 10, ionet::ServiceIdentifier(9), serviceId);

			// - indicate 3 / 5 matching service nodes are active
			mocks::MockPacketWriters writers;
			ConnectSyncAll(writers, { nodes[1], nodes[5], nodes[7] });

			// Act: run selection that returns remove candidates
			auto removeCandidates = utils::KeySet{ nodes[1].identityKey(), nodes[9].identityKey() };
			runTask(container, writers, serviceId, removeCandidates);

			// Assert: remove candidates were removed from writers
			EXPECT_EQ(removeCandidates, writers.closedNodeIdentities());

			// - removed nodes are still aged if they are active during selection (their ages will be zeroed on next iteration)
			auto view = container.view();
			EXPECT_EQ(3u, view.getNodeInfo(nodes[1].identityKey()).getConnectionState(serviceId)->Age);
			EXPECT_EQ(0u, view.getNodeInfo(nodes[9].identityKey()).getConnectionState(serviceId)->Age);
		}
	}

	TEST(TEST_CLASS, ConnectPeersTask_RemoveCandidatesAreClosedInWriters) {
		// Assert:
		AssertRemoveCandidatesAreClosedInWriters([](auto& container, auto& writers, auto serviceId, const auto& removeCandidates) {
			RunConnectPeersTask(container, writers, serviceId, [&removeCandidates]() {
				auto result = NodeSelectionResult();
				result.RemoveCandidates = removeCandidates;
				return result;
			});
		});
	}

	// endregion

	// region ConnectPeersTask: add candidates

	namespace {
		bool IsConnectedNode(const mocks::MockPacketWriters& writers, const ionet::Node& searchNode) {
			const auto& nodes = writers.connectedNodes();
			return std::any_of(nodes.cbegin(), nodes.cend(), [searchNode](const auto& node) {
				return searchNode == node;
			});
		}

		void AssertConnectionState(
				const ionet::NodeContainerView& view,
				const Key& identityKey,
				ionet::ServiceIdentifier serviceId,
				uint32_t expectedNumAttempts,
				uint32_t expectedNumSuccesses,
				uint32_t expectedNumFailures,
				bool hasLastSuccess) {
			const auto& nodeInfo = view.getNodeInfo(identityKey);
			const auto& connectionState = *nodeInfo.getConnectionState(serviceId);
			EXPECT_EQ(expectedNumAttempts, connectionState.NumAttempts);
			EXPECT_EQ(expectedNumSuccesses, connectionState.NumSuccesses);
			EXPECT_EQ(expectedNumFailures, connectionState.NumFailures);

			if (hasLastSuccess) {
				EXPECT_EQ(0u, connectionState.NumConsecutiveFailures);
				EXPECT_EQ(0u, connectionState.BanAge);
			} else {
				EXPECT_EQ(3u, connectionState.NumConsecutiveFailures); // should be incremented (initial value 2)
				EXPECT_EQ(124u, connectionState.BanAge); // should be incremented (initial value 123)
			}
		}

		void IncrementNumConsecutiveFailures(ionet::NodeContainer& container, ionet::ServiceIdentifier serviceId, const Key& identityKey) {
			++container.modifier().provisionConnectionState(serviceId, identityKey).NumConsecutiveFailures;
		}
	}

	TEST(TEST_CLASS, ConnectPeersTask_AddCandidatesHaveConnectionsInitiatedAndStatsUpdated_AllSucceed) {
		// Arrange: prepare a container with alternating matching service nodes
		auto serviceId = ionet::ServiceIdentifier(3);
		ionet::NodeContainer container;
		auto nodes = SeedAlternatingServiceNodes(container, 12, ionet::ServiceIdentifier(9), serviceId);

		mocks::MockPacketWriters writers;

		// Act: run selection that returns add candidates
		auto addCandidates = ionet::NodeSet{ nodes[3], nodes[5], nodes[9] };
		RunConnectPeersTask(container, writers, serviceId, [&addCandidates]() {
			auto result = NodeSelectionResult();
			result.AddCandidates = addCandidates;
			return result;
		});

		// Assert: add candidates were added to writers
		EXPECT_TRUE(IsConnectedNode(writers, nodes[3]));
		EXPECT_TRUE(IsConnectedNode(writers, nodes[5]));
		EXPECT_TRUE(IsConnectedNode(writers, nodes[9]));

		// - connection states have been updated appropriately from initial values (10A, 7S, 3F)
		auto view = container.view();
		AssertConnectionState(view, nodes[3].identityKey(), serviceId, 11, 8, 3, true);
		AssertConnectionState(view, nodes[5].identityKey(), serviceId, 11, 8, 3, true);
		AssertConnectionState(view, nodes[9].identityKey(), serviceId, 11, 8, 3, true);
	}

	TEST(TEST_CLASS, ConnectPeersTask_AddCandidatesHaveConnectionsInitiatedAndStatsUpdated_SomeSucceed) {
		// Arrange: prepare a container with alternating matching service nodes and increment consecutive failures for some nodes
		//          so that those nodes have requisite number of consecutive failures for banning
		auto serviceId = ionet::ServiceIdentifier(3);
		ionet::NodeContainer container;
		auto nodes = SeedAlternatingServiceNodes(container, 12, ionet::ServiceIdentifier(9), serviceId);
		IncrementNumConsecutiveFailures(container, serviceId, nodes[3].identityKey());
		IncrementNumConsecutiveFailures(container, serviceId, nodes[9].identityKey());

		// - trigger some nodes to fail during connection
		mocks::MockPacketWriters writers;
		writers.setConnectCode(nodes[3].identityKey(), net::PeerConnectCode::Socket_Error);
		writers.setConnectCode(nodes[9].identityKey(), net::PeerConnectCode::Socket_Error);

		// Act: run selection that returns add candidates
		auto addCandidates = ionet::NodeSet{ nodes[3], nodes[5], nodes[9] };
		RunConnectPeersTask(container, writers, serviceId, [&addCandidates]() {
			auto result = NodeSelectionResult();
			result.AddCandidates = addCandidates;
			return result;
		});

		// Assert: add candidates were added to writers
		EXPECT_TRUE(IsConnectedNode(writers, nodes[3]));
		EXPECT_TRUE(IsConnectedNode(writers, nodes[5]));
		EXPECT_TRUE(IsConnectedNode(writers, nodes[9]));

		// - connection states have been updated appropriately from initial values (10A, 7S, 3F)
		auto view = container.view();
		AssertConnectionState(view, nodes[3].identityKey(), serviceId, 11, 7, 4, false);
		AssertConnectionState(view, nodes[5].identityKey(), serviceId, 11, 8, 3, true);
		AssertConnectionState(view, nodes[9].identityKey(), serviceId, 11, 7, 4, false);
	}

	TEST(TEST_CLASS, ConnectPeersTask_AddCandidatesHaveConnectionsInitiatedAndStatsUpdated_NoneSucceed) {
		// Arrange: prepare a container with alternating matching service nodes and increment consecutive failures fo all nodes
		//          so that those nodes have requisite number of consecutive failures for banning
		auto serviceId = ionet::ServiceIdentifier(3);
		ionet::NodeContainer container;
		auto nodes = SeedAlternatingServiceNodes(container, 12, ionet::ServiceIdentifier(9), serviceId);
		IncrementNumConsecutiveFailures(container, serviceId, nodes[3].identityKey());
		IncrementNumConsecutiveFailures(container, serviceId, nodes[5].identityKey());
		IncrementNumConsecutiveFailures(container, serviceId, nodes[9].identityKey());

		// - trigger all nodes to fail during connection
		mocks::MockPacketWriters writers;
		writers.setConnectCode(nodes[3].identityKey(), net::PeerConnectCode::Socket_Error);
		writers.setConnectCode(nodes[5].identityKey(), net::PeerConnectCode::Socket_Error);
		writers.setConnectCode(nodes[9].identityKey(), net::PeerConnectCode::Socket_Error);

		// Act: run selection that returns add candidates
		auto addCandidates = ionet::NodeSet{ nodes[3], nodes[5], nodes[9] };
		RunConnectPeersTask(container, writers, serviceId, [&addCandidates]() {
			auto result = NodeSelectionResult();
			result.AddCandidates = addCandidates;
			return result;
		});

		// Assert: add candidates were added to writers
		EXPECT_TRUE(IsConnectedNode(writers, nodes[3]));
		EXPECT_TRUE(IsConnectedNode(writers, nodes[5]));
		EXPECT_TRUE(IsConnectedNode(writers, nodes[9]));

		// - connection states have been updated appropriately from initial values (10A, 7S, 3F)
		auto view = container.view();
		AssertConnectionState(view, nodes[3].identityKey(), serviceId, 11, 7, 4, false);
		AssertConnectionState(view, nodes[5].identityKey(), serviceId, 11, 7, 4, false);
		AssertConnectionState(view, nodes[9].identityKey(), serviceId, 11, 7, 4, false);
	}

	// endregion

	// region CreateRemoveOnlyNodeSelector

	TEST(TEST_CLASS, CanCreateRemoveOnlyNodeSelector) {
		// Arrange:
		ionet::NodeContainer container;
		auto selector = CreateRemoveOnlyNodeSelector(ionet::ServiceIdentifier(1), CreateConfiguration(), container);

		// Act:
		auto removeCandidates = selector();

		// Assert:
		EXPECT_TRUE(removeCandidates.empty());
	}

	// endregion

	// region CreateAgePeersTask

	namespace {
		void RunAgePeersTask(
				ionet::NodeContainer& container,
				net::PacketWriters& packetWriters,
				ionet::ServiceIdentifier serviceId,
				const RemoveOnlyNodeSelector& selector = RemoveOnlyNodeSelector()) {
			// Act:
			auto task = selector
					? CreateAgePeersTask(container, packetWriters, serviceId, CreateConfiguration(), selector)
					: CreateAgePeersTask(container, packetWriters, serviceId, CreateConfiguration());
			auto result = task.Callback().get();

			// Assert:
			EXPECT_EQ("age peers task", task.Name);
			EXPECT_EQ(thread::TaskResult::Continue, result);
		}
	}

	TEST(TEST_CLASS, AgePeersTask_MatchingServiceNodesAreAged) {
		// Assert:
		AssertMatchingServiceNodesAreAged([](auto& container, auto& writers, auto serviceId) {
			RunAgePeersTask(container, writers, serviceId);
		});
	}

	TEST(TEST_CLASS, AgePeersTask_RemoveCandidatesAreClosedInWriters) {
		// Assert:
		AssertRemoveCandidatesAreClosedInWriters([](auto& container, auto& writers, auto serviceId, const auto& removeCandidates) {
			RunAgePeersTask(container, writers, serviceId, [&removeCandidates]() {
				return removeCandidates;
			});
		});
	}

	// endregion
}}
