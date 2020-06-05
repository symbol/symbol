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

#include "catapult/ionet/NodeDataContainer.h"
#include "tests/TestHarness.h"

namespace catapult { namespace ionet {

#define TEST_CLASS NodeDataContainerTests

	namespace {
		constexpr auto Default_Equality_Strategy = model::NodeIdentityEqualityStrategy::Key_And_Host;
		constexpr auto Service_Identifier = ServiceIdentifier(123);

		// region SeedThreeNodes / Prepare

		auto SeedThreeNodes(NodeDataContainer& container, NodeSource secondSource = NodeSource::Static) {
			auto keys = test::GenerateRandomDataVector<Key>(3);
			container.insert(NodeData(Node({ keys[0], "11.22.33.44" }), NodeSource::Dynamic, 16));
			container.insert(NodeData(Node({ keys[1], "22.33.44.55" }), secondSource, 9));
			container.insert(NodeData(Node({ keys[2], "33.44.55.66" }), NodeSource::Dynamic, 4));

			// Sanity:
			EXPECT_EQ(3u, container.size());
			return keys;
		}

		void PrepareInactive(NodeDataContainer& container, const model::NodeIdentity& identity, uint32_t value) {
			auto* pNodeData = container.tryGet(identity);
			pNodeData->NodeInfo.provisionConnectionState(Service_Identifier).BanAge = value;

			// Sanity:
			EXPECT_FALSE(pNodeData->HasIdentityUpdateInProgress);
		}

		void PrepareActive(NodeDataContainer& container, const model::NodeIdentity& identity, uint32_t value) {
			auto* pNodeData = container.tryGet(identity);
			pNodeData->NodeInfo.provisionConnectionState(Service_Identifier).Age = value;
		}

		void PrepareActiveAndUpdatable(NodeDataContainer& container, const model::NodeIdentity& identity, uint32_t value) {
			auto* pNodeData = container.tryGet(identity);
			pNodeData->NodeInfo.provisionConnectionState(Service_Identifier).Age = value;
			pNodeData->HasIdentityUpdateInProgress = true;
		}

		// endregion

		// region BasicNodeData / CollectAll

		struct BasicNodeData {
		public:
			Key IdentityKey;
			std::string Host;
			ionet::NodeSource Source;
			size_t Id;

		public:
			bool operator==(const BasicNodeData& rhs) const {
				return IdentityKey == rhs.IdentityKey && Host == rhs.Host && Source == rhs.Source && Id == rhs.Id;
			}
		};

		struct BasicNodeDataHasher {
			size_t operator()(const BasicNodeData& data) const {
				return utils::ArrayHasher<Key>()(data.IdentityKey);
			}
		};

		using BasicNodeDataContainer = std::unordered_set<BasicNodeData, BasicNodeDataHasher>;

		BasicNodeDataContainer CollectAll(const NodeDataContainer& container) {
			BasicNodeDataContainer basicDataContainer;
			container.forEach([&basicDataContainer](const auto& node, const auto& nodeInfo) {
				const auto& identity = node.identity();
				basicDataContainer.insert({ identity.PublicKey, identity.Host, nodeInfo.source(), 0 });
			});

			return basicDataContainer;
		}

		BasicNodeDataContainer CollectAll(NodeDataContainer& container) {
			BasicNodeDataContainer basicDataContainer;
			container.forEach([&basicDataContainer](const auto& nodeData) {
				const auto& identity = nodeData.Node.identity();
				basicDataContainer.insert({ identity.PublicKey, identity.Host, nodeData.NodeInfo.source(), nodeData.NodeId });
			});

			return basicDataContainer;
		}

		// endregion
	}

	// region constructor

	TEST(TEST_CLASS, ContainerIsInitiallyEmpty) {
		// Act:
		NodeDataContainer container(Default_Equality_Strategy);

		// Assert:
		EXPECT_EQ(0u, container.size());
	}

	// endregion

	// region tryGet

	namespace {
		template<typename TContainer>
		void AssertTryGetReturnsNullptrWhenNodeIsNotInContainer() {
			// Arrange:
			NodeDataContainer container(Default_Equality_Strategy);
			auto keys = SeedThreeNodes(container);

			// Act:
			auto* pNodeData = const_cast<TContainer>(container).tryGet({ keys[1], "11.22.33.44" });

			// Assert:
			EXPECT_FALSE(!!pNodeData);
		}

		template<typename TContainer>
		void AssertTryGetReturnsDataWhenNodeIsInContainer() {
			// Arrange:
			NodeDataContainer container(Default_Equality_Strategy);
			auto keys = SeedThreeNodes(container);

			// Act:
			auto* pNodeData = const_cast<TContainer>(container).tryGet({ keys[1], "22.33.44.55" });

			// Assert:
			ASSERT_TRUE(!!pNodeData);
			EXPECT_EQ(keys[1], pNodeData->Node.identity().PublicKey);
			EXPECT_EQ(NodeSource::Static, pNodeData->NodeInfo.source());
			EXPECT_EQ(9u, pNodeData->NodeId);
			EXPECT_FALSE(pNodeData->HasIdentityUpdateInProgress);
		}
	}

	TEST(TEST_CLASS, TryGetReturnsNullptrWhenNodeIsNotInContainer_NonConst) {
		AssertTryGetReturnsNullptrWhenNodeIsNotInContainer<NodeDataContainer&>();
	}

	TEST(TEST_CLASS, TryGetReturnsNullptrWhenNodeIsNotInContainer_Const) {
		AssertTryGetReturnsNullptrWhenNodeIsNotInContainer<const NodeDataContainer&>();
	}

	TEST(TEST_CLASS, TryGetReturnsDataWhenNodeIsInContainer_NonConst) {
		AssertTryGetReturnsDataWhenNodeIsInContainer<NodeDataContainer&>();
	}

	TEST(TEST_CLASS, TryGetReturnsDataWhenNodeIsInContainer_Const) {
		AssertTryGetReturnsDataWhenNodeIsInContainer<const NodeDataContainer&>();
	}

	// endregion

	// region forEach

	TEST(TEST_CLASS, ForEachSucceedsWhenContainerIsEmpty_NonConst) {
		// Arrange:
		NodeDataContainer container(Default_Equality_Strategy);

		// Act:
		auto collectedData = CollectAll(container);

		// Assert:
		auto expectedContents = BasicNodeDataContainer();
		EXPECT_EQ(expectedContents, collectedData);
	}

	TEST(TEST_CLASS, ForEachSucceedsWhenContainerIsEmpty_Const) {
		// Arrange:
		NodeDataContainer container(Default_Equality_Strategy);

		// Act:
		auto collectedData = CollectAll(const_cast<const NodeDataContainer&>(container));

		// Assert:
		auto expectedContents = BasicNodeDataContainer();
		EXPECT_EQ(expectedContents, collectedData);
	}

	TEST(TEST_CLASS, ForEachSucceedsWhenContainerIsNotEmpty_NonConst) {
		// Arrange:
		NodeDataContainer container(Default_Equality_Strategy);
		auto keys = SeedThreeNodes(container);

		// Act:
		auto collectedData = CollectAll(container);

		// Assert:
		auto expectedContents = BasicNodeDataContainer{
			{ keys[0], "11.22.33.44", NodeSource::Dynamic, 16 },
			{ keys[1], "22.33.44.55", NodeSource::Static, 9 },
			{ keys[2], "33.44.55.66", NodeSource::Dynamic, 4 }
		};
		EXPECT_EQ(expectedContents, collectedData);
	}

	TEST(TEST_CLASS, ForEachSucceedsWhenContainerIsNotEmpty_Const) {
		// Arrange:
		NodeDataContainer container(Default_Equality_Strategy);
		auto keys = SeedThreeNodes(container);

		// Act:
		auto collectedData = CollectAll(const_cast<const NodeDataContainer&>(container));

		// Assert:
		auto expectedContents = BasicNodeDataContainer{
			{ keys[0], "11.22.33.44", NodeSource::Dynamic, 0 },
			{ keys[1], "22.33.44.55", NodeSource::Static, 0 },
			{ keys[2], "33.44.55.66", NodeSource::Dynamic, 0 }
		};
		EXPECT_EQ(expectedContents, collectedData);
	}

	// endregion

	// region prepareInsert - traits

	namespace {
		struct KeyTraits {
			static constexpr auto Equality_Strategy = model::NodeIdentityEqualityStrategy::Key;

			static model::NodeIdentity Mutate(const model::NodeIdentity& identity) {
				return { identity.PublicKey, "w.x.y.z" };
			}
		};

		struct HostTraits {
			static constexpr auto Equality_Strategy = model::NodeIdentityEqualityStrategy::Host;

			static model::NodeIdentity Mutate(const model::NodeIdentity& identity) {
				return { test::GenerateRandomByteArray<Key>(), identity.Host };
			}
		};
	}

#define EQUALITY_STRATEGY_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Key) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<KeyTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Host) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<HostTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region prepareInsert - test utils

	namespace {
		void AssertPrepareInsertResult(
				NodeDataContainer::PrepareInsertCode expectedCode,
				const Key& expectedKey,
				const std::string& expectedHost,
				bool expectedHasIdentityUpdateInProgress,
				const std::pair<NodeData*, NodeDataContainer::PrepareInsertCode>& resultPair,
				const consumer<NodeData>& consumer) {
			EXPECT_EQ(expectedCode, resultPair.second);
			ASSERT_TRUE(!!resultPair.first);
			EXPECT_EQ(expectedKey, resultPair.first->Node.identity().PublicKey);
			EXPECT_EQ(expectedHost, resultPair.first->Node.identity().Host);
			EXPECT_EQ(expectedHasIdentityUpdateInProgress, resultPair.first->HasIdentityUpdateInProgress);
			consumer(*resultPair.first);
		}

		void AssertPrepareInsertResult(
				NodeDataContainer::PrepareInsertCode expectedCode,
				const Key& expectedKey,
				const std::string& expectedHost,
				const std::pair<NodeData*, NodeDataContainer::PrepareInsertCode>& resultPair,
				const consumer<NodeData>& consumer) {
			AssertPrepareInsertResult(expectedCode, expectedKey, expectedHost, false, resultPair, consumer);
		}

		void AssertPrepareInsertUpdateAllowed(
				const Key& expectedKey,
				const std::string& expectedHost,
				const std::pair<NodeData*, NodeDataContainer::PrepareInsertCode>& resultPair,
				const consumer<NodeData>& consumer) {
			AssertPrepareInsertResult(NodeDataContainer::PrepareInsertCode::Allowed, expectedKey, expectedHost, resultPair, consumer);
		}

		void AssertPrepareInsertNewAllowed(const std::pair<NodeData*, NodeDataContainer::PrepareInsertCode>& resultPair) {
			EXPECT_EQ(NodeDataContainer::PrepareInsertCode::Allowed, resultPair.second);
			EXPECT_FALSE(!!resultPair.first);
		}

		void AssertPrepareInsertConflict(const std::pair<NodeData*, NodeDataContainer::PrepareInsertCode>& resultPair) {
			EXPECT_EQ(NodeDataContainer::PrepareInsertCode::Conflict, resultPair.second);
			EXPECT_FALSE(!!resultPair.first);
		}
	}

	// endregion

	// region prepareInsert - new identity / update same identity

	EQUALITY_STRATEGY_TRAITS_BASED_TEST(PrepareInsertAllowsNewIdentity) {
		// Arrange:
		NodeDataContainer container(TTraits::Equality_Strategy);
		SeedThreeNodes(container);
		auto key = test::GenerateRandomByteArray<Key>();

		// Act:
		auto resultPair = container.prepareInsert({ key, "99.88.77.66" }, NodeSource::Dynamic);

		// Assert:
		AssertPrepareInsertNewAllowed(resultPair);

		EXPECT_EQ(3u, container.size());
	}

	EQUALITY_STRATEGY_TRAITS_BASED_TEST(PrepareInsertAlwaysAllowsUpdateWhenIdentityIsUnchanged) {
		// Arrange:
		NodeDataContainer container(TTraits::Equality_Strategy);
		auto keys = SeedThreeNodes(container);

		// - set marker indicating active connection
		PrepareActive(container, { keys[1], "22.33.44.55" }, 17);

		// Act: existing key, existing host
		auto resultPair = container.prepareInsert({ keys[1], "22.33.44.55" }, NodeSource::Static);

		// Assert:
		AssertPrepareInsertUpdateAllowed(keys[1], "22.33.44.55", resultPair, [](const auto& nodeData) {
			EXPECT_EQ(17u, nodeData.NodeInfo.getConnectionState(Service_Identifier)->Age);
		});

		EXPECT_EQ(3u, container.size());
	}

	// endregion

	// region prepareInsert - source prioritization

	namespace {
		void AssertPrepareInsertWithSource(
				NodeSource newSource,
				NodeDataContainer::PrepareInsertCode expectedPrepareInsertCode,
				model::NodeIdentityEqualityStrategy equalityStrategy = Default_Equality_Strategy) {
			// Arrange:
			NodeDataContainer container(equalityStrategy);
			auto keys = SeedThreeNodes(container);

			// Act:
			auto resultPair = container.prepareInsert({ keys[1], "22.33.44.55" }, newSource);

			// Assert:
			AssertPrepareInsertResult(expectedPrepareInsertCode, keys[1], "22.33.44.55", resultPair, [](const auto&) {});

			EXPECT_EQ(3u, container.size());
		}
	}

	EQUALITY_STRATEGY_TRAITS_BASED_TEST(PrepareInsertAllowsIdentityUpdateWithSameSource) {
		AssertPrepareInsertWithSource(NodeSource::Static, NodeDataContainer::PrepareInsertCode::Allowed, TTraits::Equality_Strategy);
	}

	EQUALITY_STRATEGY_TRAITS_BASED_TEST(PrepareInsertAllowsIdentityUpdateWithBetterSource) {
		AssertPrepareInsertWithSource(NodeSource::Local, NodeDataContainer::PrepareInsertCode::Allowed, TTraits::Equality_Strategy);
	}

	EQUALITY_STRATEGY_TRAITS_BASED_TEST(PrepareInsertDisallowsIdentityUpdateWithWorseSource) {
		AssertPrepareInsertWithSource(NodeSource::Dynamic, NodeDataContainer::PrepareInsertCode::Redundant, TTraits::Equality_Strategy);
	}

	// endregion

	// region prepareInsert - update different identity

	EQUALITY_STRATEGY_TRAITS_BASED_TEST(PrepareInsertAllowsHostKeyChangeWhenNoActiveConnections) {
		// Arrange:
		NodeDataContainer container(TTraits::Equality_Strategy);
		auto keys = SeedThreeNodes(container);

		// - set marker indicating inactive connection
		PrepareInactive(container, { keys[1], "22.33.44.55" }, 17);

		// Act: new secondary identifier
		auto resultPair = container.prepareInsert(TTraits::Mutate({ keys[1], "22.33.44.55" }), NodeSource::Static);

		// Assert: node data associated with original is returned
		AssertPrepareInsertUpdateAllowed(keys[1], "22.33.44.55", resultPair, [](const auto& nodeData) {
			EXPECT_EQ(17u, nodeData.NodeInfo.getConnectionState(Service_Identifier)->BanAge);
		});

		EXPECT_EQ(3u, container.size());
	}

	EQUALITY_STRATEGY_TRAITS_BASED_TEST(PrepareInsertDisallowsHostKeyChangeWhenLocalSourceKey) {
		// Arrange:
		NodeDataContainer container(TTraits::Equality_Strategy);
		auto keys = SeedThreeNodes(container, NodeSource::Local);

		// - set marker indicating inactive connection
		PrepareInactive(container, { keys[1], "22.33.44.55" }, 17);

		// Act: new secondary identifier
		auto resultPair = container.prepareInsert(TTraits::Mutate({ keys[1], "22.33.44.55" }), NodeSource::Local);

		// Assert: insert is not allowed so no node data is returned
		AssertPrepareInsertConflict(resultPair);

		EXPECT_EQ(3u, container.size());
	}

	namespace {
		template<typename TTraits>
		void AssertPrepareInsertDisallowsHostKeyChangeWhenActiveConnections(NodeSource newSource) {
			// Arrange:
			NodeDataContainer container(TTraits::Equality_Strategy);
			auto keys = SeedThreeNodes(container);

			// - set marker indicating active connection
			PrepareActive(container, { keys[1], "22.33.44.55" }, 17);

			// Act: new secondary identifier
			auto resultPair = container.prepareInsert(TTraits::Mutate({ keys[1], "22.33.44.55" }), newSource);

			// Assert: insert is not allowed so no node data is returned
			AssertPrepareInsertConflict(resultPair);

			EXPECT_EQ(3u, container.size());
		}
	}

	EQUALITY_STRATEGY_TRAITS_BASED_TEST(PrepareInsertDisallowsHostKeyChangeWhenActiveConnectionsAndSameSource) {
		AssertPrepareInsertDisallowsHostKeyChangeWhenActiveConnections<TTraits>(NodeSource::Static);
	}

	EQUALITY_STRATEGY_TRAITS_BASED_TEST(PrepareInsertDisallowsHostKeyChangeWhenActiveConnectionsAndBetterSource) {
		AssertPrepareInsertDisallowsHostKeyChangeWhenActiveConnections<TTraits>(NodeSource::Local);
	}

	EQUALITY_STRATEGY_TRAITS_BASED_TEST(PrepareInsertDisallowsHostKeyChangeWhenActiveConnectionsAndWorseSource) {
		AssertPrepareInsertDisallowsHostKeyChangeWhenActiveConnections<TTraits>(NodeSource::Dynamic);
	}

	// endregion

	// region prepareInsert - update different identity (multiphase)

	EQUALITY_STRATEGY_TRAITS_BASED_TEST(PrepareInsertDisallowsPendingHostKeyChangeWhenWorseSourceForLocalSourceNodePhaseOne) {
		// Arrange: this simulates first notifyIncomingNode or phase one update
		NodeDataContainer container(TTraits::Equality_Strategy);
		auto keys = SeedThreeNodes(container, NodeSource::Local);

		// - set marker indicating inactive connection
		PrepareInactive(container, { keys[1], "22.33.44.55" }, 17);

		// Act: new secondary identifier (worse source)
		auto resultPair = container.prepareInsert(TTraits::Mutate({ keys[1], "22.33.44.55" }), NodeSource::Dynamic_Incoming);

		// Assert: insert is not allowed so no node data is returned
		AssertPrepareInsertConflict(resultPair);

		EXPECT_EQ(3u, container.size());
	}

	EQUALITY_STRATEGY_TRAITS_BASED_TEST(PrepareInsertAllowsPendingHostKeyChangeWhenWorseSourcePhaseOne) {
		// Arrange: this simulates first notifyIncomingNode or phase one update
		NodeDataContainer container(TTraits::Equality_Strategy);
		auto keys = SeedThreeNodes(container);

		// - set marker indicating inactive connection
		PrepareInactive(container, { keys[1], "22.33.44.55" }, 17);

		// Act: new secondary identifier (worse source)
		auto resultPair = container.prepareInsert(TTraits::Mutate({ keys[1], "22.33.44.55" }), NodeSource::Dynamic_Incoming);

		// Assert: node data associated with original is returned; importantly HasIdentityUpdateInProgress is true
		auto expectedCode = NodeDataContainer::PrepareInsertCode::Redundant;
		AssertPrepareInsertResult(expectedCode, keys[1], "22.33.44.55", true, resultPair, [](const auto& nodeData) {
			EXPECT_EQ(17u, nodeData.NodeInfo.getConnectionState(Service_Identifier)->BanAge);
		});

		EXPECT_EQ(3u, container.size());
	}

	EQUALITY_STRATEGY_TRAITS_BASED_TEST(PrepareInsertAllowsPendingHostKeyChangeWhenWorseSourceFollowedBySameSourcePhaseTwo) {
		// Arrange: this simulates second notifyNode or phase two update
		NodeDataContainer container(TTraits::Equality_Strategy);
		auto keys = SeedThreeNodes(container);

		// - set marker indicating active connection
		PrepareActiveAndUpdatable(container, { keys[1], "22.33.44.55" }, 1);

		// Act: new secondary identifier (same source)
		auto resultPair = container.prepareInsert(TTraits::Mutate({ keys[1], "22.33.44.55" }), NodeSource::Static);

		// Assert: node data associated with original is returned
		AssertPrepareInsertUpdateAllowed(keys[1], "22.33.44.55", resultPair, [](const auto& nodeData) {
			EXPECT_EQ(1u, nodeData.NodeInfo.getConnectionState(Service_Identifier)->Age);
		});

		EXPECT_EQ(3u, container.size());
	}

	EQUALITY_STRATEGY_TRAITS_BASED_TEST(PrepareInsertDisallowsPendingHostKeyChangeWhenWorseSourceFollowedByWorseSourcePhaseTwo) {
		// Arrange: this simulates second notifyNode or phase two update
		NodeDataContainer container(TTraits::Equality_Strategy);
		auto keys = SeedThreeNodes(container);

		// - set marker indicating active connection
		PrepareActiveAndUpdatable(container, { keys[1], "22.33.44.55" }, 1);

		// Act: new secondary identifier (worse source)
		auto resultPair = container.prepareInsert(TTraits::Mutate({ keys[1], "22.33.44.55" }), NodeSource::Dynamic);

		// Assert: insert is not allowed so no node data is returned
		AssertPrepareInsertConflict(resultPair);

		// - HasIdentityUpdateInProgress should have been cleared since update was not allowed due to source downgrade
		EXPECT_FALSE(container.tryGet({ keys[1], "22.33.44.55" })->HasIdentityUpdateInProgress);

		EXPECT_EQ(3u, container.size());
	}

	// endregion

	// region prepareInsert - key migration (Host mode only)

	TEST(TEST_CLASS, PrepareInsertAllowsKeyMigrationWhenNoActiveConnections_Host) {
		// Arrange:
		NodeDataContainer container(model::NodeIdentityEqualityStrategy::Host);
		auto keys = SeedThreeNodes(container);

		// - set marker indicating inactive connection
		PrepareInactive(container, { keys[1], "22.33.44.55" }, 17);

		// Act: existing key, new host
		auto resultPair = container.prepareInsert({ keys[1], "99.88.77.66" }, NodeSource::Static);

		// Assert: node data associated with key (not host) is returned
		AssertPrepareInsertUpdateAllowed(keys[1], "22.33.44.55", resultPair, [](const auto& nodeData) {
			EXPECT_EQ(17u, nodeData.NodeInfo.getConnectionState(Service_Identifier)->BanAge);
		});

		EXPECT_EQ(3u, container.size());
	}

	TEST(TEST_CLASS, PrepareInsertDisallowsKeyMigrationWhenLocalSourceKey_Host) {
		// Arrange:
		NodeDataContainer container(model::NodeIdentityEqualityStrategy::Host);
		auto keys = SeedThreeNodes(container, NodeSource::Local);

		// - set marker indicating inactive connection
		PrepareInactive(container, { keys[1], "22.33.44.55" }, 17);

		// Act: existing key, new host
		auto resultPair = container.prepareInsert({ keys[1], "99.88.77.66" }, NodeSource::Local);

		// Assert: insert is not allowed so no node data is returned
		AssertPrepareInsertConflict(resultPair);

		EXPECT_EQ(3u, container.size());
	}

	TEST(TEST_CLASS, PrepareInsertDisallowsKeyMigrationWhenActiveConnections_Host) {
		// Arrange:
		NodeDataContainer container(model::NodeIdentityEqualityStrategy::Host);
		auto keys = SeedThreeNodes(container);

		// - set marker indicating active connection
		PrepareActive(container, { keys[1], "22.33.44.55" }, 17);

		// Act: existing key, new host
		auto resultPair = container.prepareInsert({ keys[1], "99.88.77.66" }, NodeSource::Static);

		// Assert: insert is not allowed so no node data is returned
		AssertPrepareInsertConflict(resultPair);

		EXPECT_EQ(3u, container.size());
	}

	TEST(TEST_CLASS, PrepareInsertAllowsKeyMigrationWhenNoActiveConnectionsAndAmbiguousMatch_Host) {
		// Arrange:
		NodeDataContainer container(model::NodeIdentityEqualityStrategy::Host);
		auto keys = SeedThreeNodes(container);

		// - set markers indicating inactive connections
		PrepareInactive(container, { keys[1], "22.33.44.55" }, 17);
		PrepareInactive(container, { keys[2], "33.44.55.66" }, 18);

		// Act: existing key, existing host
		auto resultPair = container.prepareInsert({ keys[2], "22.33.44.55" }, NodeSource::Static);

		// Assert: node data associated with host (not key) is returned
		AssertPrepareInsertUpdateAllowed(keys[1], "22.33.44.55", resultPair, [](const auto& nodeData) {
			EXPECT_EQ(17u, nodeData.NodeInfo.getConnectionState(Service_Identifier)->BanAge);
		});

		// - data associated with key is purged from container
		EXPECT_EQ(2u, container.size());
		auto expectedContents = BasicNodeDataContainer{
			{ keys[0], "11.22.33.44", NodeSource::Dynamic, 16 },
			{ keys[1], "22.33.44.55", NodeSource::Static, 9 }
		};
		EXPECT_EQ(expectedContents, CollectAll(container));
	}

	TEST(TEST_CLASS, PrepareInsertAllowsKeyMigrationRequiringSourceUpgrade_Host) {
		// Arrange:
		NodeDataContainer container(model::NodeIdentityEqualityStrategy::Host);
		auto keys = test::GenerateRandomDataVector<Key>(3);
		container.insert(NodeData(Node({ keys[0], "11.22.33.44" }), NodeSource::Dynamic, 16));
		container.insert(NodeData(Node({ keys[1], "22.33.44.55" }), NodeSource::Dynamic_Incoming, 9));
		container.insert(NodeData(Node({ keys[2], "33.44.55.66" }), NodeSource::Dynamic, 4));

		// - set marker indicating inactive connection
		PrepareInactive(container, { keys[1], "22.33.44.55" }, 17);

		// Act: existing key, new host
		auto resultPair = container.prepareInsert({ keys[1], "99.88.77.66" }, NodeSource::Dynamic);

		// Assert: node data associated with key (not host) is returned
		AssertPrepareInsertUpdateAllowed(keys[1], "22.33.44.55", resultPair, [](const auto& nodeData) {
			EXPECT_EQ(17u, nodeData.NodeInfo.getConnectionState(Service_Identifier)->BanAge);
		});

		EXPECT_EQ(3u, container.size());
	}

	// endregion

	// region prepareInsert - key migration (Host mode only) (multiphase)

	TEST(TEST_CLASS, PrepareInsertAllowsKeyMigrationWhenWorseSourcePhaseOne_Host) {
		// Arrange:
		NodeDataContainer container(model::NodeIdentityEqualityStrategy::Host);
		auto keys = SeedThreeNodes(container);

		// - set marker indicating inactive connection
		PrepareInactive(container, { keys[1], "22.33.44.55" }, 17);

		// Act: existing key, new host (worse source)
		auto resultPair = container.prepareInsert({ keys[1], "99.88.77.66" }, NodeSource::Dynamic_Incoming);

		// Assert: node data associated with host (not key) is returned
		AssertPrepareInsertNewAllowed(resultPair);

		EXPECT_EQ(3u, container.size());
	}

	TEST(TEST_CLASS, PrepareInsertAllowsKeyMigrationWhenWorseSourceFollowedBySameSourcePhaseTwo_Host) {
		// Arrange:
		NodeDataContainer container(model::NodeIdentityEqualityStrategy::Host);
		auto keys = SeedThreeNodes(container);

		// - set marker indicating active connection
		PrepareActiveAndUpdatable(container, { keys[1], "22.33.44.55" }, 17);

		// Act: existing key, new host (same source)
		auto resultPair = container.prepareInsert({ keys[1], "99.88.77.66" }, NodeSource::Static);

		// Assert: node data associated with key (not host) is returned
		AssertPrepareInsertUpdateAllowed(keys[1], "22.33.44.55", resultPair, [](const auto& nodeData) {
			EXPECT_EQ(17u, nodeData.NodeInfo.getConnectionState(Service_Identifier)->Age);
		});

		EXPECT_EQ(3u, container.size());
	}

	TEST(TEST_CLASS, PrepareInsertAllowsKeyMigrationWhenWorseSourcePhaseOneAndAmbiguousMatch_Host) {
		// Arrange:
		NodeDataContainer container(model::NodeIdentityEqualityStrategy::Host);
		auto keys = SeedThreeNodes(container);

		// - set markers indicating inactive connections
		PrepareInactive(container, { keys[1], "22.33.44.55" }, 17);
		PrepareInactive(container, { keys[2], "33.44.55.66" }, 18);

		// Act: existing key, existing host (worse source)
		auto resultPair = container.prepareInsert({ keys[2], "22.33.44.55" }, NodeSource::Dynamic_Incoming);

		// Assert: node data associated with host (not key) is returned
		auto expectedCode = NodeDataContainer::PrepareInsertCode::Redundant;
		AssertPrepareInsertResult(expectedCode, keys[1], "22.33.44.55", true, resultPair, [](const auto& nodeData) {
			EXPECT_EQ(17u, nodeData.NodeInfo.getConnectionState(Service_Identifier)->BanAge);
		});

		// - container was not changed because source is worse
		EXPECT_EQ(3u, container.size());
	}

	TEST(TEST_CLASS, PrepareInsertAllowsKeyMigrationWhenWorseSourceFollowedBySameSourceAndAmbiguousMatchPhaseTwo_Host) {
		// Arrange:
		NodeDataContainer container(model::NodeIdentityEqualityStrategy::Host);
		auto keys = SeedThreeNodes(container);

		// - set markers indicating active connections
		PrepareActiveAndUpdatable(container, { keys[1], "22.33.44.55" }, 17);
		PrepareActiveAndUpdatable(container, { keys[2], "33.44.55.66" }, 18);

		// Act: existing key, existing host (same source)
		auto resultPair = container.prepareInsert({ keys[2], "22.33.44.55" }, NodeSource::Static);

		// Assert: node data associated with host (not key) is returned
		AssertPrepareInsertUpdateAllowed(keys[1], "22.33.44.55", resultPair, [](const auto& nodeData) {
			EXPECT_EQ(17u, nodeData.NodeInfo.getConnectionState(Service_Identifier)->Age);
		});

		// - data associated with key is purged from container
		EXPECT_EQ(2u, container.size());
		auto expectedContents = BasicNodeDataContainer{
			{ keys[0], "11.22.33.44", NodeSource::Dynamic, 16 },
			{ keys[1], "22.33.44.55", NodeSource::Static, 9 }
		};
		EXPECT_EQ(expectedContents, CollectAll(container));
	}

	// endregion

	// region insert / erase

	namespace {
		void AssertCanInsertSingleNodeIntoContainer(bool hasIdentityUpdateInProgress) {
			// Arrange:
			NodeDataContainer container(Default_Equality_Strategy);
			auto key = test::GenerateRandomByteArray<Key>();

			// Act:
			auto originalNodeData = NodeData(Node({ key, "11.22.33.44" }), NodeSource::Dynamic, 16);
			originalNodeData.HasIdentityUpdateInProgress = hasIdentityUpdateInProgress;
			auto* pNodeData = container.insert(originalNodeData);

			// Assert:
			EXPECT_EQ(1u, container.size());

			auto expectedContents = BasicNodeDataContainer{ { key, "11.22.33.44", NodeSource::Dynamic, 16 }, };
			EXPECT_EQ(expectedContents, CollectAll(container));

			// - check returned values
			ASSERT_TRUE(!!pNodeData);
			EXPECT_EQ(key, pNodeData->Node.identity().PublicKey);
			EXPECT_EQ(NodeSource::Dynamic, pNodeData->NodeInfo.source());
			EXPECT_EQ(16u, pNodeData->NodeId);
			EXPECT_FALSE(pNodeData->HasIdentityUpdateInProgress);
		}
	}

	TEST(TEST_CLASS, CanInsertSingleNodeIntoContainer) {
		AssertCanInsertSingleNodeIntoContainer(false);
	}

	TEST(TEST_CLASS, CanInsertSingleNodeIntoContainer_ClearsStatefulFlags) {
		AssertCanInsertSingleNodeIntoContainer(true);
	}

	TEST(TEST_CLASS, CanInsertMultipleNodesIntoContainer) {
		// Arrange:
		NodeDataContainer container(Default_Equality_Strategy);
		auto keys = test::GenerateRandomDataVector<Key>(2);

		// Act:
		auto* pNodeData1 = container.insert(NodeData(Node({ keys[0], "11.22.33.44" }), NodeSource::Dynamic, 16));
		auto* pNodeData2 = container.insert(NodeData(Node({ keys[1], "22.33.44.55" }), NodeSource::Local, 9));

		// Assert:
		EXPECT_EQ(2u, container.size());

		auto expectedContents = BasicNodeDataContainer{
			{ keys[0], "11.22.33.44", NodeSource::Dynamic, 16 },
			{ keys[1], "22.33.44.55", NodeSource::Local, 9 }
		};
		EXPECT_EQ(expectedContents, CollectAll(container));

		// - check returned values
		ASSERT_TRUE(!!pNodeData1);
		EXPECT_EQ(keys[0], pNodeData1->Node.identity().PublicKey);
		EXPECT_EQ(NodeSource::Dynamic, pNodeData1->NodeInfo.source());
		EXPECT_EQ(16u, pNodeData1->NodeId);
		EXPECT_FALSE(pNodeData1->HasIdentityUpdateInProgress);

		ASSERT_TRUE(!!pNodeData2);
		EXPECT_EQ(keys[1], pNodeData2->Node.identity().PublicKey);
		EXPECT_EQ(NodeSource::Local, pNodeData2->NodeInfo.source());
		EXPECT_EQ(9u, pNodeData2->NodeId);
		EXPECT_FALSE(pNodeData2->HasIdentityUpdateInProgress);
	}

	TEST(TEST_CLASS, CanEraseNodeFromContainer) {
		// Arrange:
		NodeDataContainer container(Default_Equality_Strategy);
		auto keys = SeedThreeNodes(container);

		// Act:
		container.erase({ keys[1], "22.33.44.55" });

		// Assert:
		EXPECT_EQ(2u, container.size());

		auto expectedContents = BasicNodeDataContainer{
			{ keys[0], "11.22.33.44", NodeSource::Dynamic, 16 },
			{ keys[2], "33.44.55.66", NodeSource::Dynamic, 4 }
		};
		EXPECT_EQ(expectedContents, CollectAll(container));

		// - should be inaccessible via tryGet too
		EXPECT_FALSE(!!container.tryGet({ keys[1], "22.33.44.55" }));
	}

	// endregion

	// region tryFindWorst

	TEST(TEST_CLASS, TryFindWorstReturnsOldestCandidateNode) {
		// Arrange:
		NodeDataContainer container(Default_Equality_Strategy);
		auto keys = test::GenerateRandomDataVector<Key>(4);
		container.insert(NodeData(Node({ keys[0], "11.22.33.44" }), NodeSource::Dynamic, 1));
		container.insert(NodeData(Node({ keys[1], "22.33.44.55" }), NodeSource::Dynamic, 2));
		container.insert(NodeData(Node({ keys[2], "33.44.55.66" }), NodeSource::Dynamic, 3));
		container.insert(NodeData(Node({ keys[3], "44.55.66.77" }), NodeSource::Dynamic, 4));

		// Act:
		auto* pWorstNodeData = container.tryFindWorst();

		// Assert:
		ASSERT_TRUE(!!pWorstNodeData);
		EXPECT_EQ(keys[0], pWorstNodeData->Node.identity().PublicKey);
	}

	TEST(TEST_CLASS, TryFindWorstSkipsNonDynamicNodes) {
		// Arrange:
		NodeDataContainer container(Default_Equality_Strategy);
		auto keys = test::GenerateRandomDataVector<Key>(4);
		container.insert(NodeData(Node({ keys[0], "11.22.33.44" }), NodeSource::Static, 1));
		container.insert(NodeData(Node({ keys[1], "22.33.44.55" }), NodeSource::Local, 2));
		container.insert(NodeData(Node({ keys[2], "33.44.55.66" }), NodeSource::Dynamic, 3));
		container.insert(NodeData(Node({ keys[3], "44.55.66.77" }), NodeSource::Dynamic, 4));

		// Act:
		auto* pWorstNodeData = container.tryFindWorst();

		// Assert:
		ASSERT_TRUE(!!pWorstNodeData);
		EXPECT_EQ(keys[2], pWorstNodeData->Node.identity().PublicKey);
	}

	TEST(TEST_CLASS, TryFindWorstPrefersNodesWithWorstSource) {
		// Arrange:
		NodeDataContainer container(Default_Equality_Strategy);
		auto keys = test::GenerateRandomDataVector<Key>(4);
		container.insert(NodeData(Node({ keys[0], "11.22.33.44" }), NodeSource::Static, 1));
		container.insert(NodeData(Node({ keys[1], "22.33.44.55" }), NodeSource::Dynamic_Incoming, 2));
		container.insert(NodeData(Node({ keys[2], "33.44.55.66" }), NodeSource::Dynamic_Incoming, 3));
		container.insert(NodeData(Node({ keys[3], "44.55.66.77" }), NodeSource::Dynamic, 4));

		// Act:
		auto* pWorstNodeData = container.tryFindWorst();

		// Assert:
		ASSERT_TRUE(!!pWorstNodeData);
		EXPECT_EQ(keys[1], pWorstNodeData->Node.identity().PublicKey);
	}

	TEST(TEST_CLASS, TryFindWorstSkipsNodesWithActiveConnections) {
		// Arrange:
		NodeDataContainer container(Default_Equality_Strategy);
		auto keys = test::GenerateRandomDataVector<Key>(4);
		container.insert(NodeData(Node({ keys[0], "11.22.33.44" }), NodeSource::Dynamic, 1));
		PrepareActive(container, { keys[0], "11.22.33.44" }, 1);
		container.insert(NodeData(Node({ keys[1], "22.33.44.55" }), NodeSource::Dynamic, 2));
		container.insert(NodeData(Node({ keys[2], "33.44.55.66" }), NodeSource::Dynamic, 3));
		container.insert(NodeData(Node({ keys[3], "44.55.66.77" }), NodeSource::Dynamic, 4));

		// Act:
		auto* pWorstNodeData = container.tryFindWorst();

		// Assert:
		ASSERT_TRUE(!!pWorstNodeData);
		EXPECT_EQ(keys[1], pWorstNodeData->Node.identity().PublicKey);
	}

	TEST(TEST_CLASS, TryFindWorstReturnsNullptrWhenThereAreNoCandidates) {
		// Arrange:
		NodeDataContainer container(Default_Equality_Strategy);
		auto keys = test::GenerateRandomDataVector<Key>(4);
		container.insert(NodeData(Node({ keys[0], "11.22.33.44" }), NodeSource::Static, 1));
		container.insert(NodeData(Node({ keys[1], "22.33.44.55" }), NodeSource::Static, 2));
		container.insert(NodeData(Node({ keys[2], "33.44.55.66" }), NodeSource::Static, 3));
		container.insert(NodeData(Node({ keys[3], "44.55.66.77" }), NodeSource::Static, 4));

		// Act:
		auto* pWorstNodeData = container.tryFindWorst();

		// Assert:
		EXPECT_FALSE(!!pWorstNodeData);
	}

	// endregion
}}
