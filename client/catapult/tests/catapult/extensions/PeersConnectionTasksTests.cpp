#include "catapult/extensions/PeersConnectionTasks.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/test/net/mocks/MockPacketWriters.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

#define TEST_CLASS PeersConnectionTasksTests

	// region CreateNodeSelector

	namespace {
		void Add(ionet::NodeContainer& container, const Key& identityKey, const std::string& nodeName, ionet::NodeRoles roles) {
			container.modifier().add(test::CreateNamedNode(identityKey, nodeName, roles), ionet::NodeSource::Dynamic);
		}
	}

	TEST(TEST_CLASS, CanCreateNodeSelector) {
		// Arrange:
		ionet::NodeContainer container;
		auto selector = CreateNodeSelector(ionet::ServiceIdentifier(1), ionet::NodeRoles::Api, { 5, 8 }, container);

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
		CreateNodeSelector(serviceId, ionet::NodeRoles::Api, { 5, 8 }, container);

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
				connectionState.NumFailures = 3;
				connectionState.NumSuccesses = 7;
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
				const NodeSelector& selector) {
			// Act:
			auto task = CreateConnectPeersTask(container, packetWriters, serviceId, selector);
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
			ionet::NodeContainer container;
			auto nodes = SeedAlternatingServiceNodes(container, 10, ionet::ServiceIdentifier(9), ionet::ServiceIdentifier(3));

			// - indicate 3 / 5 matching service nodes are active
			mocks::MockPacketWriters writers;
			ConnectSyncAll(writers, { nodes[1], nodes[5], nodes[7] });

			// Act: run selection that returns neither add nor remove candidates
			runTask(container, writers, ionet::ServiceIdentifier(3));

			// Assert: nodes associated with service 3 were aged, others were untouched
			auto i = 0u;
			auto view = container.view();
			std::vector<uint32_t> expectedAges = { 3, 0, 7, 9, 0 }; // only for matching service 3
			for (const auto& node : nodes) {
				auto message = "node at " + std::to_string(i);
				const auto& nodeInfo = view.getNodeInfo(node.identityKey());
				const auto* pConnectionState = nodeInfo.getConnectionState(ionet::ServiceIdentifier(3));

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
			RunConnectPeersTask(container, writers, serviceId, []() {
				return NodeSelectionResult();
			});
		});
	}

	// endregion

	// region ConnectPeersTask: remove candidates

	namespace {
		template<typename TRunTask>
		void AssertRemoveCandidatesAreClosedInWriters(TRunTask runTask) {
			// Arrange: prepare a container with alternating matching service nodes
			ionet::NodeContainer container;
			auto nodes = SeedAlternatingServiceNodes(container, 10, ionet::ServiceIdentifier(9), ionet::ServiceIdentifier(3));

			// - indicate 3 / 5 matching service nodes are active
			mocks::MockPacketWriters writers;
			ConnectSyncAll(writers, { nodes[1], nodes[5], nodes[7] });

			// Act: run selection that returns remove candidates
			auto removeCandidates = utils::KeySet{ nodes[1].identityKey(), nodes[9].identityKey() };
			runTask(container, writers, ionet::ServiceIdentifier(3), removeCandidates);

			// Assert: remove candidates were removed from writers
			EXPECT_EQ(removeCandidates, writers.closedNodeIdentities());

			// - removed nodes are still aged if they are active during selection (their ages will be zeroed on next iteration)
			auto view = container.view();
			EXPECT_EQ(3u, view.getNodeInfo(nodes[1].identityKey()).getConnectionState(ionet::ServiceIdentifier(3))->Age);
			EXPECT_EQ(0u, view.getNodeInfo(nodes[9].identityKey()).getConnectionState(ionet::ServiceIdentifier(3))->Age);
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
				uint32_t expectedNumFailures) {
			const auto& nodeInfo = view.getNodeInfo(identityKey);
			const auto& connectionState = *nodeInfo.getConnectionState(serviceId);
			EXPECT_EQ(expectedNumAttempts, connectionState.NumAttempts);
			EXPECT_EQ(expectedNumSuccesses, connectionState.NumSuccesses);
			EXPECT_EQ(expectedNumFailures, connectionState.NumFailures);
		}
	}

	TEST(TEST_CLASS, ConnectPeersTask_AddCandidatesHaveConnectionsInitiatedAndStatsUpdated_AllSucceed) {
		// Arrange: prepare a container with alternating matching service nodes
		ionet::NodeContainer container;
		auto nodes = SeedAlternatingServiceNodes(container, 12, ionet::ServiceIdentifier(9), ionet::ServiceIdentifier(3));

		mocks::MockPacketWriters writers;

		// Act: run selection that returns add candidates
		auto addCandidates = ionet::NodeSet{ nodes[3], nodes[5], nodes[9] };
		RunConnectPeersTask(container, writers, ionet::ServiceIdentifier(3), [&addCandidates]() {
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
		AssertConnectionState(view, nodes[3].identityKey(), ionet::ServiceIdentifier(3), 11, 8, 3);
		AssertConnectionState(view, nodes[5].identityKey(), ionet::ServiceIdentifier(3), 11, 8, 3);
		AssertConnectionState(view, nodes[9].identityKey(), ionet::ServiceIdentifier(3), 11, 8, 3);
	}

	TEST(TEST_CLASS, ConnectPeersTask_AddCandidatesHaveConnectionsInitiatedAndStatsUpdated_SomeSucceed) {
		// Arrange: prepare a container with alternating matching service nodes
		ionet::NodeContainer container;
		auto nodes = SeedAlternatingServiceNodes(container, 12, ionet::ServiceIdentifier(9), ionet::ServiceIdentifier(3));

		// - trigger some nodes to fail during connection
		mocks::MockPacketWriters writers;
		writers.setConnectResult(nodes[3].identityKey(), net::PeerConnectResult::Socket_Error);
		writers.setConnectResult(nodes[9].identityKey(), net::PeerConnectResult::Socket_Error);

		// Act: run selection that returns add candidates
		auto addCandidates = ionet::NodeSet{ nodes[3], nodes[5], nodes[9] };
		RunConnectPeersTask(container, writers, ionet::ServiceIdentifier(3), [&addCandidates]() {
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
		AssertConnectionState(view, nodes[3].identityKey(), ionet::ServiceIdentifier(3), 11, 7, 4);
		AssertConnectionState(view, nodes[5].identityKey(), ionet::ServiceIdentifier(3), 11, 8, 3);
		AssertConnectionState(view, nodes[9].identityKey(), ionet::ServiceIdentifier(3), 11, 7, 4);
	}

	TEST(TEST_CLASS, ConnectPeersTask_AddCandidatesHaveConnectionsInitiatedAndStatsUpdated_NoneSucceed) {
		// Arrange: prepare a container with alternating matching service nodes
		ionet::NodeContainer container;
		auto nodes = SeedAlternatingServiceNodes(container, 12, ionet::ServiceIdentifier(9), ionet::ServiceIdentifier(3));

		// - trigger all nodes to fail during connection
		mocks::MockPacketWriters writers;
		writers.setConnectResult(nodes[3].identityKey(), net::PeerConnectResult::Socket_Error);
		writers.setConnectResult(nodes[5].identityKey(), net::PeerConnectResult::Socket_Error);
		writers.setConnectResult(nodes[9].identityKey(), net::PeerConnectResult::Socket_Error);

		// Act: run selection that returns add candidates
		auto addCandidates = ionet::NodeSet{ nodes[3], nodes[5], nodes[9] };
		RunConnectPeersTask(container, writers, ionet::ServiceIdentifier(3), [&addCandidates]() {
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
		AssertConnectionState(view, nodes[3].identityKey(), ionet::ServiceIdentifier(3), 11, 7, 4);
		AssertConnectionState(view, nodes[5].identityKey(), ionet::ServiceIdentifier(3), 11, 7, 4);
		AssertConnectionState(view, nodes[9].identityKey(), ionet::ServiceIdentifier(3), 11, 7, 4);
	}

	// endregion

	// region CreateRemoveOnlyNodeSelector

	TEST(TEST_CLASS, CanCreateRemoveOnlyNodeSelector) {
		// Arrange:
		ionet::NodeContainer container;
		auto selector = CreateRemoveOnlyNodeSelector(ionet::ServiceIdentifier(1), { 5, 8 }, container);

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
				const RemoveOnlyNodeSelector& selector) {
			// Act:
			auto task = CreateAgePeersTask(container, packetWriters, serviceId, selector);
			auto result = task.Callback().get();

			// Assert:
			EXPECT_EQ("age peers task", task.Name);
			EXPECT_EQ(thread::TaskResult::Continue, result);
		}
	}

	TEST(TEST_CLASS, AgePeersTask_MatchingServiceNodesAreAged) {
		// Assert:
		AssertMatchingServiceNodesAreAged([](auto& container, auto& writers, auto serviceId) {
			RunAgePeersTask(container, writers, serviceId, []() {
				return utils::KeySet();
			});
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
