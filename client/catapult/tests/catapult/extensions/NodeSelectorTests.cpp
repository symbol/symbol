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

#include "catapult/extensions/NodeSelector.h"
#include "catapult/ionet/NodeContainer.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/test/other/NodeSelectorTestUtils.h"
#include "tests/TestHarness.h"
#include <random>

namespace catapult { namespace extensions {

#define TEST_CLASS NodeSelectorTests

	namespace {
		constexpr auto Default_Service_Id = ionet::ServiceIdentifier(7);

		ionet::Node CreateNamedNode(const Key& identityKey, const std::string& name, ionet::NodeRoles roles) {
			auto metadata = ionet::NodeMetadata(model::NetworkIdentifier::Zero, name);
			metadata.Roles = roles;
			return ionet::Node(identityKey, ionet::NodeEndpoint(), metadata);
		}

		ionet::ConnectionState CreateConnectionStateFromAttempts(uint32_t numSuccesses, uint32_t numFailures) {
			ionet::ConnectionState connectionState;
			connectionState.NumAttempts = numSuccesses + numFailures;
			connectionState.NumSuccesses = numSuccesses;
			connectionState.NumFailures = numFailures;
			return connectionState;
		}

		std::vector<ionet::Node> SeedNodes(
				ionet::NodeContainer& container,
				size_t numNodes,
				ionet::NodeSource source = ionet::NodeSource::Dynamic,
				ionet::NodeRoles roles = ionet::NodeRoles::Peer) {
			std::vector<ionet::Node> nodes;
			auto modifier = container.modifier();
			for (auto i = 0u; i < numNodes; ++i) {
				auto identityKey = test::GenerateRandomData<Key_Size>();
				auto node = CreateNamedNode(identityKey, "node " + std::to_string(i), roles);
				modifier.add(node, source);
				modifier.provisionConnectionState(Default_Service_Id, identityKey);
				nodes.push_back(node);
			}

			return nodes;
		}

		void SetAge(ionet::NodeContainer& container, const std::vector<ionet::Node>& nodes, uint32_t age) {
			auto modifier = container.modifier();
			for (const auto& node : nodes)
				modifier.provisionConnectionState(Default_Service_Id, node.identityKey()).Age = age;
		}

		NodeSelectionConfiguration CreateConfiguration(uint32_t maxConnections, uint32_t maxConnectionAge) {
			return { Default_Service_Id, ionet::NodeRoles::Peer, maxConnections, maxConnectionAge };
		}

		void AssertSubset(const ionet::NodeSet& set, const ionet::NodeSet& subset) {
			for (const auto& node : subset)
				EXPECT_TRUE(set.cend() != set.find(node)) << "set expected to contain " << node;
		}

		void AssertSubset(const std::vector<ionet::Node>& set, const ionet::NodeSet& subset) {
			AssertSubset(ionet::NodeSet(set.cbegin(), set.cend()), subset);
		}

		void AssertSubset(const utils::KeySet& set, const utils::KeySet& subset) {
			for (const auto& key : subset)
				EXPECT_TRUE(set.cend() != set.find(key)) << "set expected to contain " << utils::HexFormat(key);
		}

		void AssertSubset(const std::vector<ionet::Node>& set, const utils::KeySet& subset) {
			AssertSubset(test::ExtractNodeIdentities(set), subset);
		}
	}

	// region CalculateWeight

	namespace {
		uint32_t CalculateWeightFromAttempts(uint32_t numSuccesses, uint32_t numFailures) {
			return CalculateWeight(CreateConnectionStateFromAttempts(numSuccesses, numFailures));
		}
	}

	TEST(TEST_CLASS, ConnectionStateWithZeroAttemptsIsGivenMedianWeight) {
		// Act + Assert:
		EXPECT_EQ(5'000u, CalculateWeightFromAttempts(0, 0));
	}

	TEST(TEST_CLASS, ConnectionStateWithAllSuccessesIsGivenMaxWeight) {
		// Act + Assert:
		EXPECT_EQ(10'000u, CalculateWeightFromAttempts(1, 0));
		EXPECT_EQ(10'000u, CalculateWeightFromAttempts(99, 0));
		EXPECT_EQ(10'000u, CalculateWeightFromAttempts(10'000, 0));
	}

	TEST(TEST_CLASS, ConnectionStateWithAllFailuresIsGivenDecreasingWeightAsAttemptsIncrease) {
		// Act + Assert:
		EXPECT_EQ(1'000u, CalculateWeightFromAttempts(0, 1));
		EXPECT_EQ(10u, CalculateWeightFromAttempts(0, 99));
		EXPECT_EQ(1u, CalculateWeightFromAttempts(0, 10'000)); // 1 (> 1'000 / 10'000) is the weight lower bound
	}

	TEST(TEST_CLASS, ConnectionStateWithMixedSuccessesAndFailuresIsGivenWeightEqualToSuccessPercentage) {
		// Act + Assert:
		EXPECT_EQ(1'111u, CalculateWeightFromAttempts(11, 88));
		EXPECT_EQ(7'010u, CalculateWeightFromAttempts(701, 299));
		EXPECT_EQ(1'230u, CalculateWeightFromAttempts(123, 877));
		EXPECT_EQ(1u, CalculateWeightFromAttempts(100, 9'999'900)); // 1 (> 100 / 10'000'000 * 10'000) is the weight lower bound
	}

	// endregion

	// region SelectCandidatesBasedOnWeight

	TEST(TEST_CLASS, ReturnsNoCandidatesWhenContainerIsEmpty_SelectCandidatesBasedOnWeight) {
		// Arrange:
		WeightedCandidates candidates;

		// Act:
		auto nodes = SelectCandidatesBasedOnWeight(candidates, 1234, 5);

		// Assert:
		EXPECT_TRUE(nodes.empty());
	}

	namespace {
		ionet::NodeSet CreateNamedNodes(const std::vector<Key>& keys) {
			ionet::NodeSet nodes;
			for (auto i = 0u; i < keys.size(); ++i)
				nodes.emplace(test::CreateNamedNode(keys[i], "Node" + std::to_string(i + 1)));

			return nodes;
		}

		void AssertReturnsAllCandidates(size_t numCandidates, size_t maxCandidates) {
			// Arrange:
			auto keys = test::GenerateRandomDataVector<Key>(numCandidates);
			auto expectedNodes = CreateNamedNodes(keys);
			WeightedCandidates candidates;
			for (const auto& node : expectedNodes)
				candidates.push_back(WeightedCandidate(node, 100));

			// Act:
			auto nodes = SelectCandidatesBasedOnWeight(candidates, numCandidates * 100u, maxCandidates);

			// Assert:
			EXPECT_EQ(numCandidates, nodes.size());
			EXPECT_EQ(expectedNodes, nodes);
		}
	}

	TEST(TEST_CLASS, ReturnsAllCandidatesWhenContainerSizeIsLessThanMaxCandidates_SelectCandidatesBasedOnWeight) {
		// Assert:
		AssertReturnsAllCandidates(1, 5);
		AssertReturnsAllCandidates(2, 5);
		AssertReturnsAllCandidates(4, 5);
	}

	TEST(TEST_CLASS, ReturnsAllCandidatesWhenContainerSizeIsEqualToMaxCandidates_SelectCandidatesBasedOnWeight) {
		// Assert:
		AssertReturnsAllCandidates(1, 1);
		AssertReturnsAllCandidates(5, 5);
		AssertReturnsAllCandidates(10, 10);
	}

	TEST(TEST_CLASS, ReturnsMaxCandidatesWhenContainerSizeIsGreaterThanMaxCandidates_SelectCandidatesBasedOnWeight) {
		// Arrange:
		auto keys = test::GenerateRandomDataVector<Key>(5);
		auto allNodes = CreateNamedNodes(keys);

		WeightedCandidates candidates;
		for (const auto& node : allNodes)
			candidates.push_back(WeightedCandidate(node, 100));

		// Act:
		auto nodes = SelectCandidatesBasedOnWeight(candidates, 500u, 3);
		auto allKeys = test::ExtractNodeIdentities(allNodes);

		// Assert:
		EXPECT_EQ(3u, nodes.size());
		for (const auto& node : nodes)
			EXPECT_TRUE(allKeys.cend() != allKeys.find(node.identityKey()));
	}

	// endregion

	// region probability test

	namespace {
		struct SelectCandidatesBasedOnWeightTraits {
		public:
			using KeyStatistics = std::unordered_map<Key, uint32_t, utils::ArrayHasher<Key>>;
			static constexpr auto Description() { return "select candidates and weight correlation"; }

			static KeyStatistics CreateStatistics(
					const std::vector<ionet::Node>& nodes,
					const std::vector<uint64_t>& rawWeights,
					uint64_t numIterations) {
				KeyStatistics keyStatistics;
				for (auto i = 0u; i < numIterations; ++i) {
					auto index = 0u;
					uint64_t cumulativeWeight = 0u;
					WeightedCandidates candidates;
					for (const auto& node : nodes) {
						cumulativeWeight += rawWeights[index];
						candidates.push_back(WeightedCandidate(node, rawWeights[index++]));
					}

					auto selectedNodes = SelectCandidatesBasedOnWeight(candidates, cumulativeWeight, 1);
					if (1u != selectedNodes.size())
						CATAPULT_THROW_RUNTIME_ERROR_1("unexpected number of nodes were selected", selectedNodes.size());

					++keyStatistics[selectedNodes.cbegin()->identityKey()];
				}

				return keyStatistics;
			}
		};

		struct SelectNodesTraits {
		public:
			using KeyStatistics = std::unordered_map<Key, uint32_t, utils::ArrayHasher<Key>>;
			static constexpr auto Description() { return "select nodes and weight correlation"; }

			static KeyStatistics CreateStatistics(
					const std::vector<ionet::Node>& nodes,
					const std::vector<uint64_t>& rawWeights,
					uint64_t numIterations) {
				KeyStatistics keyStatistics;
				ionet::NodeContainer container;
				SeedNodeContainer(container, nodes, CreateConnectionStates(rawWeights));
				NodeSelectionConfiguration config{ Default_Service_Id, ionet::NodeRoles::None, 1, 1234 };
				for (auto i = 0u; i < numIterations; ++i) {
					auto nodeSelectionResult = SelectNodes(container, config);
					const auto& selectedNodes = nodeSelectionResult.AddCandidates;
					if (1u != selectedNodes.size())
						CATAPULT_THROW_RUNTIME_ERROR_1("unexpected number of nodes were selected", selectedNodes.size());

					++keyStatistics[selectedNodes.cbegin()->identityKey()];
				}

				return keyStatistics;
			}

		private:
			static void SeedNodeContainer(
					ionet::NodeContainer& container,
					const std::vector<ionet::Node>& nodes,
					const std::vector<ionet::ConnectionState>& connectionState) {
				auto modifier = container.modifier();
				for (auto i = 0u; i < nodes.size(); ++i) {
					modifier.add(nodes[i], ionet::NodeSource::Dynamic);
					modifier.provisionConnectionState(Default_Service_Id, nodes[i].identityKey()) = connectionState[i];
				}
			}

			static std::vector<ionet::ConnectionState> CreateConnectionStates(const std::vector<uint64_t>& rawWeights) {
				// weights have to be in the range from 1 to 10'000
				// NumFailures is deliberately set to 1'000 to make weight calculation easy to follow
				auto maxWeight = *std::max_element(rawWeights.cbegin(), rawWeights.cend());
				std::vector<ionet::ConnectionState> connectionStates;
				for (auto rawWeight : rawWeights) {
					auto scaledRawWeight = static_cast<uint32_t>(rawWeight * 10'000ull / maxWeight);
					ionet::ConnectionState connectionState;
					connectionState.Age = 0;
					connectionState.NumAttempts = 10'000;
					connectionState.NumSuccesses = scaledRawWeight;
					connectionState.NumFailures = 1'000;
					connectionStates.push_back(connectionState);
				}

				return connectionStates;
			}
		};
	}

	DEFINE_NODE_SELECTOR_PROBABILITY_TESTS(SelectCandidatesBasedOnWeight)
	DEFINE_NODE_SELECTOR_PROBABILITY_TESTS(SelectNodes)

	// endregion

	// region SelectNodes: no candidates in container

	TEST(TEST_CLASS, NoAddCandidatesWhenContainerIsEmpty) {
		// Arrange:
		ionet::NodeContainer container;

		// Act:
		auto result = SelectNodes(container, CreateConfiguration(5, 8));

		// Assert:
		EXPECT_TRUE(result.AddCandidates.empty());
		EXPECT_TRUE(result.RemoveCandidates.empty());
	}

	namespace {
		void AssertNoAddOrRemoveCandidatesWhenContainerHasNoMatchingServiceNodes(uint32_t age, uint32_t maxAge) {
			// Arrange: seed the container with nodes that support a different service
			ionet::NodeContainer container;
			SetAge(container, SeedNodes(container, 10), age);

			// Act:
			auto result = SelectNodes(container, { ionet::ServiceIdentifier(2), ionet::NodeRoles::Peer, 5, maxAge });

			// Assert:
			EXPECT_TRUE(result.AddCandidates.empty());
			EXPECT_TRUE(result.RemoveCandidates.empty());
		}
	}

	TEST(TEST_CLASS, NoAddCandidatesWhenContainerHasNoMatchingServiceNodes) {
		// Assert:
		AssertNoAddOrRemoveCandidatesWhenContainerHasNoMatchingServiceNodes(0, 8);
	}

	TEST(TEST_CLASS, NoRemoveCandidatesWhenContainerHasNoMatchingServiceNodes) {
		// Assert:
		AssertNoAddOrRemoveCandidatesWhenContainerHasNoMatchingServiceNodes(10, 8);
	}

	namespace {
		void AssertNoAddOrRemoveCandidatesWhenContainerHasNoMatchingRoleNodes(uint32_t age, uint32_t maxAge) {
			// Arrange: seed the container with nodes that have a different role
			ionet::NodeContainer container;
			SetAge(container, SeedNodes(container, 10), age);

			// Act:
			auto result = SelectNodes(container, { Default_Service_Id, ionet::NodeRoles::Api, 5, maxAge });

			// Assert:
			EXPECT_TRUE(result.AddCandidates.empty());
			EXPECT_TRUE(result.RemoveCandidates.empty());
		}
	}

	TEST(TEST_CLASS, NoAddCandidatesWhenContainerHasNoMatchingRoleNodes) {
		// Assert:
		AssertNoAddOrRemoveCandidatesWhenContainerHasNoMatchingRoleNodes(0, 8);
	}

	TEST(TEST_CLASS, NoRemoveCandidatesWhenContainerHasNoMatchingRoleNodes) {
		// Assert:
		AssertNoAddOrRemoveCandidatesWhenContainerHasNoMatchingRoleNodes(10, 8);
	}

	namespace {
		void AssertNoAddOrRemoveCandidatesWhenContainerHasOnlyLocalMatchingServiceNodes(uint32_t age, uint32_t maxAge) {
			// Arrange:
			ionet::NodeContainer container;
			SetAge(container, SeedNodes(container, 10, ionet::NodeSource::Local), age);

			// Act:
			auto result = SelectNodes(container, CreateConfiguration(5, maxAge));

			// Assert:
			EXPECT_TRUE(result.AddCandidates.empty());
			EXPECT_TRUE(result.RemoveCandidates.empty());
		}
	}

	TEST(TEST_CLASS, NoAddCandidatesWhenContainerHasOnlyLocalMatchingServiceNodes) {
		// Assert:
		AssertNoAddOrRemoveCandidatesWhenContainerHasOnlyLocalMatchingServiceNodes(0, 8);
	}

	TEST(TEST_CLASS, NoRemoveCandidatesWhenContainerHasOnlyLocalMatchingServiceNodes) {
		// Assert: this is a contrived example because nodes with Local source should never be active
		AssertNoAddOrRemoveCandidatesWhenContainerHasOnlyLocalMatchingServiceNodes(10, 8);
	}

	// endregion

	// region SelectNodes: add only

	namespace {
		void AgeAndErase(ionet::NodeContainer& container, std::vector<ionet::Node>& nodes, size_t index, uint32_t age) {
			container.modifier().provisionConnectionState(Default_Service_Id, nodes[index].identityKey()).Age = age;
			nodes.erase(nodes.begin() + static_cast<long>(index));
		}

		void AssertAdds(
				uint32_t maxConnections,
				uint32_t numContainerNodes,
				uint32_t numExpectedAddCandidates,
				const consumer<ionet::NodeContainer&, std::vector<ionet::Node>&>& prepare) {
			// Arrange:
			ionet::NodeContainer container;
			auto nodes = SeedNodes(container, numContainerNodes);
			prepare(container, nodes);

			// Act:
			auto result = SelectNodes(container, CreateConfiguration(maxConnections, 8));

			// Assert:
			EXPECT_EQ(numExpectedAddCandidates, result.AddCandidates.size());
			AssertSubset(nodes, result.AddCandidates);
			EXPECT_TRUE(result.RemoveCandidates.empty());
		}
	}

	TEST(TEST_CLASS, ActiveAndAddCandidatesSumToMaxConnectionsWhenNoNodesAreInitiallyActive) {
		// Assert:
		AssertAdds(5, 10, 5, [](const auto&, const auto&) {});
	}

	TEST(TEST_CLASS, ActiveAndAddCandidatesSumToMaxConnectionsWhenFewerThanMaxConnectionsNodesAreInitiallyActive) {
		// Assert:
		AssertAdds(5, 10, 3, [](auto& container, auto& nodes) {
			// Arrange: activate 2/5 nodes
			AgeAndErase(container, nodes, 3, 1);
			AgeAndErase(container, nodes, 1, 1);
		});
	}

	TEST(TEST_CLASS, ActiveAndAddCandidatesSumToMaxConnectionsWhenOneLessThanMaxConnectionsNodesAreInitiallyActive) {
		// Assert:
		AssertAdds(5, 10, 1, [](auto& container, auto& nodes) {
			// Arrange: activate 4/5 nodes
			AgeAndErase(container, nodes, 8, 1);
			AgeAndErase(container, nodes, 7, 1);
			AgeAndErase(container, nodes, 5, 1);
			AgeAndErase(container, nodes, 0, 1);
		});
	}

	TEST(TEST_CLASS, AllInactiveNodesAreAddCandidatesWhenMaxConnectionsIsGreaterThanMatchingServiceNodes) {
		// Assert: since 10 < 500, all inactive nodes are candidates
		AssertAdds(500, 10, 6, [](auto& container, auto& nodes) {
			// Arrange: activate 4/500 nodes
			AgeAndErase(container, nodes, 6, 1);
			AgeAndErase(container, nodes, 5, 1);
			AgeAndErase(container, nodes, 4, 1);
			AgeAndErase(container, nodes, 3, 1);
		});
	}

	// endregion

	// region SelectNodes: roles filtering

	TEST(TEST_CLASS, NodesSupportingSupersetOfDesiredRolesAreSelected) {
		// Arrange:
		ionet::NodeContainer container;
		auto nodes = SeedNodes(container, 10, ionet::NodeSource::Dynamic, ionet::NodeRoles::Api | ionet::NodeRoles::Peer);

		// Act:
		auto result = SelectNodes(container, CreateConfiguration(5, 8));

		// Assert: nodes were selected even though they support BOTH Api and Peer roles
		EXPECT_EQ(5u, result.AddCandidates.size());
		AssertSubset(nodes, result.AddCandidates);
		EXPECT_TRUE(result.RemoveCandidates.empty());
	}

	// endregion

	// region SelectNodes: relative weighting

	namespace {
		struct NodeInfos {
		public:
			NodeInfos(const ionet::ConnectionState& connectionState1, const ionet::ConnectionState& connectionState2)
					: ConnectionState1(connectionState1)
					, ConnectionState2(connectionState2)
					, Source1(ionet::NodeSource::Dynamic)
					, Source2(ionet::NodeSource::Dynamic)
			{}

		public:
			const ionet::ConnectionState& ConnectionState1;
			const ionet::ConnectionState& ConnectionState2;
			ionet::NodeSource Source1;
			ionet::NodeSource Source2;
		};

		std::pair<uint32_t, uint32_t> RunManyPairwiseSelections(const NodeInfos& nodeInfos) {
			// Arrange: seed two inactive nodes
			ionet::NodeContainer container;
			auto node1 = SeedNodes(container, 1, nodeInfos.Source1)[0];
			auto node2 = SeedNodes(container, 1, nodeInfos.Source2)[0];
			{
				auto modifier = container.modifier();
				modifier.provisionConnectionState(Default_Service_Id, node1.identityKey()) = nodeInfos.ConnectionState1;
				modifier.provisionConnectionState(Default_Service_Id, node2.identityKey()) = nodeInfos.ConnectionState2;
			}

			// Act: run a lot of selections
			std::pair<uint32_t, uint32_t> counts(0, 0);
			for (auto i = 0u; i < 1000; ++i) {
				auto result = SelectNodes(container, CreateConfiguration(1, 8));
				if (1 != result.AddCandidates.size())
					CATAPULT_THROW_RUNTIME_ERROR("unexpected number of candidate nodes returned");

				if (node1 == *result.AddCandidates.cbegin())
					++counts.first;
				else
					++counts.second;
			}

			CATAPULT_LOG(debug) << "selections (" << counts.first << ", " << counts.second << ")";
			return counts;
		}

		void RunNonDeterministicPairwiseSelectionTest(
				const NodeInfos& nodeInfos,
				const predicate<std::pair<uint32_t, uint32_t>>& isSuccess) {
			test::RunNonDeterministicTest("pairwise node selection", [&]() {
				// demand that 9 / 10 runs are successful
				auto numSuccessfulTests = 0u;
				for (auto i = 0u; i < 10; ++i) {
					auto counts = RunManyPairwiseSelections(nodeInfos);
					if (isSuccess(counts))
						++numSuccessfulTests;
					else
						CATAPULT_LOG(warning) << "failed pairwise node selection test due to unexpected ratio";
				}

				return 8 < numSuccessfulTests;
			});
		}
	}

	TEST(TEST_CLASS, NewNodeHasLowerPriorityThanNodeWithSuccesses) {
		// Arrange:
		auto connectionState1 = ionet::ConnectionState();
		auto connectionState2 = CreateConnectionStateFromAttempts(5, 0);

		// Assert:
		RunNonDeterministicPairwiseSelectionTest(NodeInfos(connectionState1, connectionState2), [](const auto& counts) {
			return counts.first < counts.second;
		});
	}

	TEST(TEST_CLASS, NewNodeHasHigherPriorityThanNodeWithFailures) {
		// Arrange:
		auto connectionState1 = ionet::ConnectionState();
		auto connectionState2 = CreateConnectionStateFromAttempts(0, 5);

		// Assert:
		RunNonDeterministicPairwiseSelectionTest(NodeInfos(connectionState1, connectionState2), [](const auto& counts) {
			return counts.first > counts.second;
		});
	}

	TEST(TEST_CLASS, StaticNodeHasHigherPriorityThanDynamicNode) {
		// Arrange:
		auto connectionState1 = ionet::ConnectionState();
		auto connectionState2 = ionet::ConnectionState();
		NodeInfos nodeInfos(connectionState1, connectionState2);
		nodeInfos.Source1 = ionet::NodeSource::Static;
		nodeInfos.Source2 = ionet::NodeSource::Dynamic;

		// Assert:
		RunNonDeterministicPairwiseSelectionTest(nodeInfos, [](const auto& counts) {
			return counts.first > counts.second && counts.first < 5 * counts.second;
		});
	}

	TEST(TEST_CLASS, DynamicNodeWithLargeWeightHasHigherPriorityThanStaticNodeWithSmallWeight) {
		// Arrange: weights 4000 / 9500
		auto connectionState1 = CreateConnectionStateFromAttempts(1, 4);
		auto connectionState2 = CreateConnectionStateFromAttempts(95, 5);
		NodeInfos nodeInfos(connectionState1, connectionState2);
		nodeInfos.Source1 = ionet::NodeSource::Static;
		nodeInfos.Source2 = ionet::NodeSource::Dynamic;

		// Assert:
		RunNonDeterministicPairwiseSelectionTest(nodeInfos, [](const auto& counts) {
			return counts.first < counts.second;
		});
	}

	// endregion

	// region SelectNodes: remove

	namespace {
		void AssertNoRemovals(uint32_t maxConnections, uint32_t numActiveNodes, uint32_t numInactiveNodes) {
			// Arrange: seed inactive nodes and active nodes with age 7 (max age is 8)
			ionet::NodeContainer container;
			SeedNodes(container, numInactiveNodes);
			SetAge(container, SeedNodes(container, numActiveNodes), 7);

			// Act:
			auto result = SelectNodes(container, CreateConfiguration(maxConnections, 8));

			// Assert:
			EXPECT_TRUE(result.AddCandidates.empty());
			EXPECT_TRUE(result.RemoveCandidates.empty());
		}
	}

	TEST(TEST_CLASS, NoNodesAreRemovedWhenThereAreExactlyMaxConnectionsButNoneHaveMaxAge) {
		// Assert: maxConnections(5) == numActiveNodes(5)
		AssertNoRemovals(5, 5, 2);
	}

	TEST(TEST_CLASS, NoNodesAreRemovedWhenThereAreGreaterThanMaxConnectionsButNoneHaveMaxAge) {
		// Assert: maxConnections(5) < numActiveNodes(8)
		AssertNoRemovals(5, 8, 2);
	}

	namespace {
		void AssertRemovalsButNoAdds(uint32_t maxConnections, uint32_t numActiveNodes, uint32_t numExpectedRemoveCandidates) {
			// Arrange: seed only active nodes that all have at least max age
			ionet::NodeContainer container;
			auto nodes = SeedNodes(container, numActiveNodes);
			auto i = 0u;
			for (const auto& node : nodes)
				container.modifier().provisionConnectionState(Default_Service_Id, node.identityKey()).Age = 8 + i++;

			// Act:
			auto result = SelectNodes(container, CreateConfiguration(maxConnections, 8));

			// Assert:
			EXPECT_TRUE(result.AddCandidates.empty());
			EXPECT_EQ(numExpectedRemoveCandidates, result.RemoveCandidates.size());
			AssertSubset(nodes, result.RemoveCandidates);
		}
	}

	TEST(TEST_CLASS, NoRemoveCandidatesWhenLessThanMaxConnectionsAreActiveAndNodesHaveMaxAge) {
		// Assert: resulting active nodes (4 - 0) should be less than num connections (5)
		AssertRemovalsButNoAdds(5, 4, 0);
	}

	TEST(TEST_CLASS, NoRemoveCandidatesWhenMaxConnectionsAreActiveAndNodesHaveMaxAgeAndNoConnectionsAreInactive) {
		// Assert: resulting active nodes (5 - 0) should equal num connections (5)
		AssertRemovalsButNoAdds(5, 5, 0);
	}

	TEST(TEST_CLASS, MultipleRemoveCandidatesWhenGreaterThanMaxConnectionsAreActiveAndNoConnectionsAreInactive) {
		// Assert: resulting active nodes (8 - 3) should equal num connections (5)
		AssertRemovalsButNoAdds(5, 8, 3);
	}

	namespace {
		void AssertRemovalsAndSingleAdd(uint32_t maxConnections, uint32_t numActiveNodes, uint32_t numExpectedRemoveCandidates) {
			// Arrange: seed active that all have at least max age and inactive nodes
			ionet::NodeContainer container;
			auto inactiveNodes = SeedNodes(container, 2);
			auto activeNodes = SeedNodes(container, numActiveNodes);
			auto i = 0u;
			for (const auto& node : activeNodes)
				container.modifier().provisionConnectionState(Default_Service_Id, node.identityKey()).Age = 8 + i++;

			// Act:
			auto result = SelectNodes(container, CreateConfiguration(maxConnections, 8));

			// Assert: one candidate is added
			EXPECT_EQ(1u, result.AddCandidates.size());
			AssertSubset(inactiveNodes, result.AddCandidates);
			EXPECT_EQ(numExpectedRemoveCandidates, result.RemoveCandidates.size());
			AssertSubset(activeNodes, result.RemoveCandidates);
		}
	}

	TEST(TEST_CLASS, SingleRemoveCandidateWhenMaxConnectionsAreActiveAndNodesHaveMaxAgeAndSomeConnectionsAreInactive) {
		// Assert: resulting active nodes (5 - 1 + 1) should equal num connections (5)
		AssertRemovalsAndSingleAdd(5, 5, 1);
	}

	TEST(TEST_CLASS, MultipleRemoveCandidatesWhenGreaterThanMaxConnectionsAreActiveAndSomeConnectionsAreInactive) {
		// Assert: resulting active nodes (8 - 3 - 1 + 1) should equal num connections (5)
		AssertRemovalsAndSingleAdd(5, 8, 4);
	}

	// endregion

	// region SelectNodesForRemoval

	namespace {
		NodeAgingConfiguration CreateAgingConfiguration(uint32_t maxConnections, uint32_t maxConnectionAge) {
			return { Default_Service_Id, maxConnections, maxConnectionAge };
		}
	}

	TEST(TEST_CLASS, ForRemoval_NoCandidatesWhenContainerIsEmpty) {
		// Arrange:
		ionet::NodeContainer container;

		// Act:
		auto removeCandidates = SelectNodesForRemoval(container, CreateAgingConfiguration(5, 8));

		// Assert:
		EXPECT_TRUE(removeCandidates.empty());
	}

	TEST(TEST_CLASS, ForRemoval_NoCandidatesWhenContainerHasNoMatchingServiceNodes) {
		// Arrange: seed the container with nodes that support a different service
		ionet::NodeContainer container;
		SetAge(container, SeedNodes(container, 10), 10);

		// Act:
		auto removeCandidates = SelectNodesForRemoval(container, { ionet::ServiceIdentifier(2), 5, 8 });

		// Assert:
		EXPECT_TRUE(removeCandidates.empty());
	}

	TEST(TEST_CLASS, ForRemoval_NoCandidatesWhenContainerHasOnlyLocalMatchingServiceNodes) {
		// Arrange: this is a contrived example because nodes with Local source should never be active
		ionet::NodeContainer container;
		SetAge(container, SeedNodes(container, 10, ionet::NodeSource::Local), 10);

		// Act:
		auto removeCandidates = SelectNodesForRemoval(container, CreateAgingConfiguration(5, 8));

		// Assert:
		EXPECT_TRUE(removeCandidates.empty());
	}

	namespace {
		void AssertRemoveOnlyNoRemovals(uint32_t maxConnections, uint32_t numActiveNodes, uint32_t numInactiveNodes) {
			// Arrange: seed inactive nodes and active nodes with age 7 (max age is 8)
			ionet::NodeContainer container;
			SeedNodes(container, numInactiveNodes);
			SetAge(container, SeedNodes(container, numActiveNodes), 7);

			// Act:
			auto removeCandidates = SelectNodesForRemoval(container, CreateAgingConfiguration(maxConnections, 8));

			// Assert:
			EXPECT_TRUE(removeCandidates.empty());
		}
	}

	TEST(TEST_CLASS, ForRemoval_NoNodesAreRemovedWhenThereAreExactlyMaxConnectionsButNoneHaveMaxAge) {
		// Assert: maxConnections(5) == numActiveNodes(5)
		AssertRemoveOnlyNoRemovals(5, 5, 2);
	}

	TEST(TEST_CLASS, ForRemoval_NoNodesAreRemovedWhenThereAreGreaterThanMaxConnectionsButNoneHaveMaxAge) {
		// Assert: maxConnections(5) < numActiveNodes(8)
		AssertRemoveOnlyNoRemovals(5, 8, 2);
	}

	namespace {
		void AssertRemoveOnlyRemovals(uint32_t maxConnections, uint32_t numActiveNodes, uint32_t numExpectedRemoveCandidates) {
			// Arrange: seed only active nodes that all have at least max age
			ionet::NodeContainer container;
			auto nodes = SeedNodes(container, numActiveNodes);
			auto i = 0u;
			for (const auto& node : nodes)
				container.modifier().provisionConnectionState(Default_Service_Id, node.identityKey()).Age = 8 + i++;

			// Act:
			auto removeCandidates = SelectNodesForRemoval(container, CreateAgingConfiguration(maxConnections, 8));

			// Assert:
			EXPECT_EQ(numExpectedRemoveCandidates, removeCandidates.size());
			AssertSubset(nodes, removeCandidates);
		}
	}

	TEST(TEST_CLASS, ForRemoval_NoRemoveCandidatesWhenLessThanMinimumConnectionsAreActive) {
		// Assert: resulting active nodes (2 - 0) should be less than min connections (5 * 3 / 4 == 3)
		AssertRemoveOnlyRemovals(5, 2, 0);
	}

	TEST(TEST_CLASS, ForRemoval_NoRemoveCandidatesWhenMinimumConnectionsAreActive) {
		// Assert: resulting active nodes (3 - 0) should equal min connections (5 * 3 / 4 == 3)
		AssertRemoveOnlyRemovals(5, 3, 0);
	}

	TEST(TEST_CLASS, ForRemoval_MultipleRemoveCandidatesWhenMaxConnectionsAreActive) {
		// Assert: resulting active nodes (5 - 2) should equal min connections (5 * 3 / 4 == 3)
		AssertRemoveOnlyRemovals(5, 5, 2);
	}

	TEST(TEST_CLASS, ForRemoval_MultipleRemoveCandidatesWhenGreaterThanMaxConnectionsAreActive) {
		// Assert: resulting active nodes (8 - 5) should equal min connections (5 * 3 / 4 == 3)
		AssertRemoveOnlyRemovals(5, 8, 5);
	}

	TEST(TEST_CLASS, ForRemoval_NoRemoveCandidatesWhenMaxConnectionsIsOne) {
		// Assert: resulting active nodes (1 - 0) should equal min connections (1)
		AssertRemoveOnlyRemovals(1, 1, 0);
	}

	// endregion
}}
