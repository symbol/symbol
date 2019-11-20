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
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/ionet/NodeInteractionResult.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/test/net/mocks/MockPacketWriters.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

#define TEST_CLASS PeersConnectionTasksTests

	// region utils

	namespace {
		model::NodeIdentity ToIdentity(const Key& identityKey) {
			return { identityKey, "11.22.33.44" };
		}

		model::NodeIdentitySet ToIdentitiesSet(const std::vector<Key>& identityKeys) {
			return test::ToIdentitiesSet(identityKeys, "11.22.33.44");
		}

		config::NodeConfiguration::ConnectionsSubConfiguration CreateConfiguration() {
			return { 5, 8, 250, 2 };
		}

		void Add(ionet::NodeContainer& container, const Key& identityKey, const std::string& nodeName, ionet::NodeRoles roles) {
			container.modifier().add(test::CreateNamedNode({ identityKey, "11.22.33.44" }, nodeName, roles), ionet::NodeSource::Dynamic);
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
			modifier.provisionConnectionState(serviceId, ToIdentity(keys[0])).Age = 1;
			modifier.provisionConnectionState(serviceId, ToIdentity(keys[1])).Age = 2;
			modifier.provisionConnectionState(serviceId, ToIdentity(keys[2])).Age = 3;
		}

		// Act:
		auto ager = CreateNodeAger(serviceId, CreateConfiguration(), container);
		ager(ToIdentitiesSet({ keys[0], keys[2] }));

		// Assert: nodes { 0, 2 } are aged, node { 1 } is cleared
		auto view = container.view();
		EXPECT_EQ(2u, view.getNodeInfo(ToIdentity(keys[0])).getConnectionState(serviceId)->Age);
		EXPECT_EQ(0u, view.getNodeInfo(ToIdentity(keys[1])).getConnectionState(serviceId)->Age);
		EXPECT_EQ(4u, view.getNodeInfo(ToIdentity(keys[2])).getConnectionState(serviceId)->Age);
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
			modifier.provisionConnectionState(serviceId, ToIdentity(keys[0])).NumConsecutiveFailures = 3;
			modifier.provisionConnectionState(serviceId, ToIdentity(keys[0])).BanAge = 11;
			modifier.provisionConnectionState(serviceId, ToIdentity(keys[1])).NumConsecutiveFailures = 3;
			modifier.provisionConnectionState(serviceId, ToIdentity(keys[1])).BanAge = 100;
			modifier.provisionConnectionState(serviceId, ToIdentity(keys[2])).NumConsecutiveFailures = 3;
		}

		auto config = CreateConfiguration();
		config.MaxConnectionBanAge = 100;
		config.NumConsecutiveFailuresBeforeBanning = 3;

		// Act:
		auto ager = CreateNodeAger(serviceId, config, container);
		ager(ToIdentitiesSet(std::vector<Key>()));

		// Assert: all banned nodes with matching service are aged irrespective of identities passed to ager
		auto view = container.view();
		EXPECT_EQ(12u, view.getNodeInfo(ToIdentity(keys[0])).getConnectionState(serviceId)->BanAge);
		EXPECT_EQ(0u, view.getNodeInfo(ToIdentity(keys[1])).getConnectionState(serviceId)->BanAge);
		EXPECT_EQ(1u, view.getNodeInfo(ToIdentity(keys[2])).getConnectionState(serviceId)->BanAge);
	}

	// endregion

	// region SelectorSettings

	namespace {
		void AssertImportanceDescriptor(
				const SelectorSettings& settings,
				const Key& key,
				Importance expectedImportance,
				Importance expectedTotalChainImportance,
				const std::string& message) {
			const auto& descriptor = settings.ImportanceRetriever(key);
			EXPECT_EQ(expectedImportance, descriptor.Importance) << message;
			EXPECT_EQ(expectedTotalChainImportance, descriptor.TotalChainImportance) << message;
		}

		template<typename TSettingsFactory>
		void AssertCanCreateSelectorSettings(ionet::NodeRoles expectedRole, TSettingsFactory settingsFactory) {
			// Arrange:
			ionet::NodeContainer container;
			auto unknownKey = test::GenerateRandomByteArray<Key>();
			auto knownKeyWrongHeight = test::GenerateRandomByteArray<Key>();
			auto knownKey = test::GenerateRandomByteArray<Key>();

			// -  create and initialize a cache
			auto blockChainConfig = model::BlockChainConfiguration::Uninitialized();
			blockChainConfig.ImportanceGrouping = 1;
			auto cache = test::CreateEmptyCatapultCache(blockChainConfig);
			{
				auto cacheDelta = cache.createDelta();
				auto& accountStateCacheDelta = cacheDelta.sub<cache::AccountStateCache>();
				accountStateCacheDelta.addAccount(knownKeyWrongHeight, Height(1));
				accountStateCacheDelta.find(knownKeyWrongHeight).get().ImportanceSnapshots.set(
						Importance(222),
						model::ImportanceHeight(100));

				accountStateCacheDelta.addAccount(knownKey, Height(1));
				accountStateCacheDelta.find(knownKey).get().ImportanceSnapshots.set(Importance(111), model::ImportanceHeight(999));
				cache.commit(Height(1000));
			}

			// Act:
			auto settings = settingsFactory(cache, Importance(100), container, ionet::ServiceIdentifier(4), CreateConfiguration());

			// Assert:
			EXPECT_EQ(&container, &settings.Nodes);
			EXPECT_EQ(ionet::ServiceIdentifier(4), settings.ServiceId);
			EXPECT_EQ(expectedRole, settings.RequiredRole);
			EXPECT_EQ(5u, settings.Config.MaxConnections); // only check one config field as proxy

			AssertImportanceDescriptor(settings, unknownKey, Importance(0), Importance(100), "unknownKey");
			AssertImportanceDescriptor(settings, knownKeyWrongHeight, Importance(0), Importance(100), "knownKeyWrongHeight");
			AssertImportanceDescriptor(settings, knownKey, Importance(111), Importance(100), "knownKey");
		}
	}

	TEST(TEST_CLASS, CanCreateSelectorSettingsWithRole) {
		AssertCanCreateSelectorSettings(ionet::NodeRoles::Api, [](
				const auto& cache,
				auto totalChainImportance,
				auto& nodes,
				auto serviceId,
				const auto& config) {
			return SelectorSettings(cache, totalChainImportance, nodes, serviceId, ionet::NodeRoles::Api, config);
		});
	}

	TEST(TEST_CLASS, CanCreateSelectorSettingsWithoutRole) {
		AssertCanCreateSelectorSettings(ionet::NodeRoles::None, [](
				const auto& cache,
				auto totalChainImportance,
				auto& nodes,
				auto serviceId,
				const auto& config) {
			return SelectorSettings(cache, totalChainImportance, nodes, serviceId, config);
		});
	}

	// endregion

	// region CreateNodeSelector

	TEST(TEST_CLASS, CanCreateNodeSelector) {
		// Arrange:
		auto cache = test::CreateEmptyCatapultCache();
		ionet::NodeContainer container;
		auto serviceId = ionet::ServiceIdentifier(1);
		auto settings = SelectorSettings(cache, Importance(100), container, serviceId, ionet::NodeRoles::Api, CreateConfiguration());
		auto selector = CreateNodeSelector(settings);

		// Act:
		auto result = selector();

		// Assert:
		EXPECT_TRUE(result.AddCandidates.empty());
		EXPECT_TRUE(result.RemoveCandidates.empty());
	}

	TEST(TEST_CLASS, CreateNodeSelectorProvisionsConnectionStatesForRoleCompatibleNodes) {
		// Arrange:
		auto cache = test::CreateEmptyCatapultCache();
		ionet::NodeContainer container;
		auto keys = test::GenerateRandomDataVector<Key>(3);
		Add(container, keys[0], "bob", ionet::NodeRoles::Api);
		Add(container, keys[1], "alice", ionet::NodeRoles::Peer);
		Add(container, keys[2], "charlie", ionet::NodeRoles::Api | ionet::NodeRoles::Peer);
		auto serviceId = ionet::ServiceIdentifier(1);
		auto settings = SelectorSettings(cache, Importance(100), container, serviceId, ionet::NodeRoles::Api, CreateConfiguration());

		// Act:
		CreateNodeSelector(settings);

		// Assert:
		const auto& view = container.view();
		EXPECT_TRUE(!!view.getNodeInfo(ToIdentity(keys[0])).getConnectionState(serviceId));
		EXPECT_FALSE(!!view.getNodeInfo(ToIdentity(keys[1])).getConnectionState(serviceId));
		EXPECT_TRUE(!!view.getNodeInfo(ToIdentity(keys[2])).getConnectionState(serviceId));
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
				auto identityKey = test::GenerateRandomByteArray<Key>();
				auto node = test::CreateNamedNode(identityKey, "node " + std::to_string(i));
				modifier.add(node, ionet::NodeSource::Dynamic);
				nodes.push_back(node);
				test::AddNodeInteractions(modifier, node.identity(), 7, 3);

				auto serviceId = 0 == i % 2 ? evenServiceId : oddServiceId;
				auto& connectionState = modifier.provisionConnectionState(serviceId, node.identity());
				connectionState.Age = i + 1;
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
			auto cache = test::CreateEmptyCatapultCache();
			auto settings = SelectorSettings(cache, Importance(100), container, serviceId, ionet::NodeRoles::Peer, CreateConfiguration());
			auto task = selector
					? CreateConnectPeersTask(settings, packetWriters, selector)
					: CreateConnectPeersTask(settings, packetWriters);
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
				const auto& nodeInfo = view.getNodeInfo(node.identity());
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
			auto removeCandidates = test::ToIdentitiesSet({ nodes[1].identity(), nodes[9].identity() });
			runTask(container, writers, serviceId, removeCandidates);

			// Assert: remove candidates were removed from writers
			test::AssertEqualIdentities(removeCandidates, writers.closedNodeIdentities());

			// - ages of all removed nodes are reset even if the nodes are active during selection
			auto view = container.view();
			EXPECT_EQ(0u, view.getNodeInfo(nodes[1].identity()).getConnectionState(serviceId)->Age);
			EXPECT_EQ(0u, view.getNodeInfo(nodes[9].identity()).getConnectionState(serviceId)->Age);
		}
	}

	TEST(TEST_CLASS, ConnectPeersTask_RemoveCandidatesAreClosedInWriters) {
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
				return ionet::NodeEquality()(searchNode, node);
			});
		}

		void AssertInteractions(
				const ionet::NodeContainerView& view,
				const model::NodeIdentity& identity,
				uint32_t expectedNumSuccesses,
				uint32_t expectedNumFailures) {
			auto interactions = view.getNodeInfo(identity).interactions(Timestamp());
			test::AssertNodeInteractions(expectedNumSuccesses, expectedNumFailures, interactions);
		}

		void AssertConnectionState(
				const ionet::NodeContainerView& view,
				const model::NodeIdentity& identity,
				ionet::ServiceIdentifier serviceId,
				bool hasLastSuccess) {
			const auto& nodeInfo = view.getNodeInfo(identity);
			const auto& connectionState = *nodeInfo.getConnectionState(serviceId);

			if (hasLastSuccess) {
				EXPECT_EQ(0u, connectionState.NumConsecutiveFailures);
				EXPECT_EQ(0u, connectionState.BanAge);
			} else {
				EXPECT_EQ(3u, connectionState.NumConsecutiveFailures); // should be incremented (initial value 2)
				EXPECT_EQ(124u, connectionState.BanAge); // should be incremented (initial value 123)
			}
		}

		void IncrementNumConsecutiveFailures(
				ionet::NodeContainer& container,
				ionet::ServiceIdentifier serviceId,
				const model::NodeIdentity& identity) {
			++container.modifier().provisionConnectionState(serviceId, identity).NumConsecutiveFailures;
		}
	}

	TEST(TEST_CLASS, ConnectPeersTask_AddCandidatesHaveConnectionsInitiatedAndInteractionsUpdated_AllSucceed) {
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

		// - interactions have been updated appropriately from initial values (7S, 3F)
		auto view = container.view();
		AssertInteractions(view, nodes[3].identity(), 8, 3);
		AssertInteractions(view, nodes[5].identity(), 8, 3);
		AssertInteractions(view, nodes[9].identity(), 8, 3);

		// - connection states have correct number of consecutive failures and ban age
		AssertConnectionState(view, nodes[3].identity(), serviceId, true);
		AssertConnectionState(view, nodes[5].identity(), serviceId, true);
		AssertConnectionState(view, nodes[9].identity(), serviceId, true);
	}

	TEST(TEST_CLASS, ConnectPeersTask_AddCandidatesHaveConnectionsInitiatedAndInteractionsUpdated_SomeSucceed) {
		// Arrange: prepare a container with alternating matching service nodes and increment consecutive failures for some nodes
		//          so that those nodes have requisite number of consecutive failures for banning
		auto serviceId = ionet::ServiceIdentifier(3);
		ionet::NodeContainer container;
		auto nodes = SeedAlternatingServiceNodes(container, 12, ionet::ServiceIdentifier(9), serviceId);
		IncrementNumConsecutiveFailures(container, serviceId, nodes[3].identity());
		IncrementNumConsecutiveFailures(container, serviceId, nodes[9].identity());

		// - trigger some nodes to fail during connection
		mocks::MockPacketWriters writers;
		writers.setConnectCode(nodes[3].identity(), net::PeerConnectCode::Socket_Error);
		writers.setConnectCode(nodes[9].identity(), net::PeerConnectCode::Socket_Error);

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

		// - interactions have been updated appropriately from initial values (7S, 3F)
		auto view = container.view();
		AssertInteractions(view, nodes[3].identity(), 7, 4);
		AssertInteractions(view, nodes[5].identity(), 8, 3);
		AssertInteractions(view, nodes[9].identity(), 7, 4);

		// - connection states have correct number of consecutive failures and ban age
		AssertConnectionState(view, nodes[3].identity(), serviceId, false);
		AssertConnectionState(view, nodes[5].identity(), serviceId, true);
		AssertConnectionState(view, nodes[9].identity(), serviceId, false);
	}

	TEST(TEST_CLASS, ConnectPeersTask_AddCandidatesHaveConnectionsInitiatedAndInteractionsUpdated_NoneSucceed) {
		// Arrange: prepare a container with alternating matching service nodes and increment consecutive failures fo all nodes
		//          so that those nodes have requisite number of consecutive failures for banning
		auto serviceId = ionet::ServiceIdentifier(3);
		ionet::NodeContainer container;
		auto nodes = SeedAlternatingServiceNodes(container, 12, ionet::ServiceIdentifier(9), serviceId);
		IncrementNumConsecutiveFailures(container, serviceId, nodes[3].identity());
		IncrementNumConsecutiveFailures(container, serviceId, nodes[5].identity());
		IncrementNumConsecutiveFailures(container, serviceId, nodes[9].identity());

		// - trigger all nodes to fail during connection
		mocks::MockPacketWriters writers;
		writers.setConnectCode(nodes[3].identity(), net::PeerConnectCode::Socket_Error);
		writers.setConnectCode(nodes[5].identity(), net::PeerConnectCode::Socket_Error);
		writers.setConnectCode(nodes[9].identity(), net::PeerConnectCode::Socket_Error);

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

		// - interactions have been updated appropriately from initial values (7S, 3F)
		auto view = container.view();
		AssertInteractions(view, nodes[3].identity(), 7, 4);
		AssertInteractions(view, nodes[5].identity(), 7, 4);
		AssertInteractions(view, nodes[9].identity(), 7, 4);

		// - connection states have correct number of consecutive failures and ban age
		AssertConnectionState(view, nodes[3].identity(), serviceId, false);
		AssertConnectionState(view, nodes[5].identity(), serviceId, false);
		AssertConnectionState(view, nodes[9].identity(), serviceId, false);
	}

	// endregion

	// region CreateRemoveOnlyNodeSelector

	TEST(TEST_CLASS, CanCreateRemoveOnlyNodeSelector) {
		// Arrange:
		auto cache = test::CreateEmptyCatapultCache();
		ionet::NodeContainer container;
		auto settings = SelectorSettings(cache, Importance(100), container, ionet::ServiceIdentifier(1), CreateConfiguration());
		auto selector = CreateRemoveOnlyNodeSelector(settings);

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
			auto cache = test::CreateEmptyCatapultCache();
			auto settings = SelectorSettings(cache, Importance(100), container, serviceId, CreateConfiguration());
			auto task = selector
					? CreateAgePeersTask(settings, packetWriters, selector)
					: CreateAgePeersTask(settings, packetWriters);
			auto result = task.Callback().get();

			// Assert:
			EXPECT_EQ("age peers task", task.Name);
			EXPECT_EQ(thread::TaskResult::Continue, result);
		}
	}

	TEST(TEST_CLASS, AgePeersTask_MatchingServiceNodesAreAged) {
		AssertMatchingServiceNodesAreAged([](auto& container, auto& writers, auto serviceId) {
			RunAgePeersTask(container, writers, serviceId);
		});
	}

	TEST(TEST_CLASS, AgePeersTask_RemoveCandidatesAreClosedInWriters) {
		AssertRemoveCandidatesAreClosedInWriters([](auto& container, auto& writers, auto serviceId, const auto& removeCandidates) {
			RunAgePeersTask(container, writers, serviceId, [&removeCandidates]() {
				return removeCandidates;
			});
		});
	}

	// endregion
}}
