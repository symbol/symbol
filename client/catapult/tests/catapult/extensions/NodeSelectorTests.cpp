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
#include "catapult/ionet/NodeInteractionResult.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/test/other/NodeSelectorTestUtils.h"
#include "tests/TestHarness.h"
#include <numeric>
#include <random>

namespace catapult { namespace extensions {

#define TEST_CLASS NodeSelectorTests

	// region test utils

	namespace {
		constexpr auto Default_Service_Id = ionet::ServiceIdentifier(7);

		ImportanceDescriptor UniformImportanceRetriever(const Key&) {
			return { Importance(1), Importance(100) };
		}

		ionet::Node CreateNamedNode(const Key& identityKey, const std::string& name, ionet::NodeRoles roles) {
			auto metadata = ionet::NodeMetadata(model::UniqueNetworkFingerprint(), name);
			metadata.Roles = roles;
			return ionet::Node({ identityKey, "11.22.33.44" }, ionet::NodeEndpoint(), metadata);
		}

		std::vector<ionet::Node> SeedNodes(
				ionet::NodeContainer& container,
				size_t numNodes,
				ionet::NodeSource source = ionet::NodeSource::Dynamic,
				ionet::NodeRoles roles = ionet::NodeRoles::Peer) {
			std::vector<ionet::Node> nodes;
			auto modifier = container.modifier();
			for (auto i = 0u; i < numNodes; ++i) {
				auto identityKey = test::GenerateRandomByteArray<Key>();
				auto node = CreateNamedNode(identityKey, "node " + std::to_string(i), roles);
				modifier.add(node, source);
				modifier.provisionConnectionState(Default_Service_Id, node.identity());
				nodes.push_back(node);
			}

			return nodes;
		}

		void SetAge(ionet::NodeContainer& container, const std::vector<ionet::Node>& nodes, uint32_t age) {
			auto modifier = container.modifier();
			for (const auto& node : nodes)
				modifier.provisionConnectionState(Default_Service_Id, node.identity()).Age = age;
		}

		void SetBanAge(ionet::NodeContainer& container, const std::vector<ionet::Node>& nodes, uint32_t age) {
			auto modifier = container.modifier();
			for (const auto& node : nodes)
				modifier.provisionConnectionState(Default_Service_Id, node.identity()).BanAge = age;
		}

		NodeSelectionConfiguration CreateConfiguration(uint32_t maxConnections, uint32_t maxConnectionAge) {
			return { Default_Service_Id, ionet::NodeRoles::Peer, maxConnections, maxConnectionAge };
		}

		void AssertSubset(const ionet::NodeSet& set, const ionet::NodeSet& subset) {
			for (const auto& node : subset)
				EXPECT_CONTAINS(set, node);
		}

		void AssertSubset(const std::vector<ionet::Node>& set, const ionet::NodeSet& subset) {
			AssertSubset(ionet::NodeSet(set.cbegin(), set.cend()), subset);
		}

		void AssertSubset(const model::NodeIdentitySet& set, const model::NodeIdentitySet& subset) {
			for (const auto& identity : subset)
				EXPECT_CONTAINS(set, identity);
		}

		void AssertSubset(const std::vector<ionet::Node>& set, const model::NodeIdentitySet& subset) {
			AssertSubset(test::ExtractNodeIdentities(set), subset);
		}
	}

	// endregion

	// region CalculateWeight - from interactions

	namespace {
		uint32_t CalculateWeightFromAttempts(uint32_t numSuccesses, uint32_t numFailures) {
			return CalculateWeight(
					ionet::NodeInteractions(numSuccesses, numFailures),
					WeightPolicy::Interactions,
					[]() { return UniformImportanceRetriever(Key()); });
		}
	}

	TEST(TEST_CLASS, NodeInteractionsWithLessThanFourAttemptsAreGivenMedianWeight) {
		for (auto i = 0u; i <= 3; ++i) {
			for (auto j = 0u; j <= 3 - i; ++j)
				EXPECT_EQ(5'000u, CalculateWeightFromAttempts(i, j)) << "NumSuccesses " << i << " NumFailures " << j;
		}
	}

	TEST(TEST_CLASS, NodeInteractionsWithAllSuccessesIsGivenMaxWeight) {
		EXPECT_EQ(10'000u, CalculateWeightFromAttempts(4, 0));
		EXPECT_EQ(10'000u, CalculateWeightFromAttempts(99, 0));
		EXPECT_EQ(10'000u, CalculateWeightFromAttempts(10'000, 0));
	}

	TEST(TEST_CLASS, NodeInteractionsWithAllFailuresIsGivenMinWeight) {
		EXPECT_EQ(500u, CalculateWeightFromAttempts(0, 4));
		EXPECT_EQ(500u, CalculateWeightFromAttempts(0, 99));
		EXPECT_EQ(500u, CalculateWeightFromAttempts(0, 10'000));
	}

	TEST(TEST_CLASS, NodeInteractionsWithMixedSuccessesAndFailuresIsGivenWeightAccordingToFormula) {
		// Act + Assert: weight = max(500, Successes * 10'000 / (Successes + 9 * Failures))
		EXPECT_EQ(1'000u, CalculateWeightFromAttempts(20, 20));
		EXPECT_EQ(1'000u, CalculateWeightFromAttempts(500, 500));
		EXPECT_EQ(9'174u, CalculateWeightFromAttempts(100, 1));
		EXPECT_EQ(5'263u, CalculateWeightFromAttempts(100, 10));
		EXPECT_EQ(1'818u, CalculateWeightFromAttempts(100, 50));
		EXPECT_EQ(526u, CalculateWeightFromAttempts(50, 100));
		EXPECT_EQ(500u, CalculateWeightFromAttempts(10, 100));
		EXPECT_EQ(500u, CalculateWeightFromAttempts(1, 100));
	}

	// endregion

	// region CalculateWeight - from importance

	namespace {
		constexpr Importance Default_Total_Importance(9'000'000'000);

		uint32_t CalculateWeightFromImportance(uint64_t rawImportance) {
			auto retriever = [rawImportance](const auto&) {
				return ImportanceDescriptor{ Importance(rawImportance), Default_Total_Importance };
			};
			return CalculateWeight(
					ionet::NodeInteractions(),
					WeightPolicy::Importance,
					[retriever]() { return retriever(Key()); });
		}
	}

	TEST(TEST_CLASS, NodeWithAtLeastSupernodeImportanceIsGivenMaxWeight) {
		for (auto rawImportance : { 3'000'000ull, 5'000'000ull, 1'000'000'000ull, 9'000'000'000ull })
			EXPECT_EQ(10'000u, CalculateWeightFromImportance(rawImportance));
	}

	TEST(TEST_CLASS, NodeWithLowImportanceIsGivenMinWeight) {
		for (auto rawImportance : { 150'000ull, 100'000ull, 10'000ull, 1'000ull })
			EXPECT_EQ(500u, CalculateWeightFromImportance(rawImportance));
	}

	TEST(TEST_CLASS, NodeIsGivenWeightAccordingToFormula) {
		EXPECT_EQ(750u, CalculateWeightFromImportance(225'000));
		EXPECT_EQ(1000u, CalculateWeightFromImportance(300'000));
		EXPECT_EQ(2000u, CalculateWeightFromImportance(600'000));
		EXPECT_EQ(5000u, CalculateWeightFromImportance(1'500'000));
		EXPECT_EQ(9000u, CalculateWeightFromImportance(2'700'000));
	}

	// endregion

	// region WeightPolicyGenerator

	TEST(TEST_CLASS, WeightPolicyGeneratorGeneratedValuesAreAccordinglyBalancedBetweenInteractionsAndImportance) {
		// Arrange:
		WeightPolicyGenerator policyGenerator;

		// Act + Assert:
		test::RunNonDeterministicTest("CalculateWeight algorithm", [&policyGenerator]() {
			auto numSelectionsViaImportance = 0u;
			for (auto i = 0u; i < 1000; ++i) {
				if (WeightPolicy::Importance == policyGenerator())
					++numSelectionsViaImportance;
			}

			return 230 < numSelectionsViaImportance && 270 > numSelectionsViaImportance;
		});
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
			test::AssertEqualNodes(expectedNodes, nodes);
		}
	}

	TEST(TEST_CLASS, ReturnsAllCandidatesWhenContainerSizeIsLessThanMaxCandidates_SelectCandidatesBasedOnWeight) {
		AssertReturnsAllCandidates(1, 5);
		AssertReturnsAllCandidates(2, 5);
		AssertReturnsAllCandidates(4, 5);
	}

	TEST(TEST_CLASS, ReturnsAllCandidatesWhenContainerSizeIsEqualToMaxCandidates_SelectCandidatesBasedOnWeight) {
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
		auto allIdentities = test::ExtractNodeIdentities(allNodes);

		// Assert:
		EXPECT_EQ(3u, nodes.size());
		for (const auto& node : nodes)
			EXPECT_CONTAINS(allIdentities, node.identity());
	}

	// endregion

	// region SelectCandidatesBasedOnWeight + SelectNodes: probability test

	namespace {
		struct SelectCandidatesBasedOnWeightTraits {
		public:
			using KeyStatistics = std::unordered_map<Key, uint32_t, utils::ArrayHasher<Key>>;
			static constexpr auto Description = "select candidates and weight correlation";

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

					++keyStatistics[selectedNodes.cbegin()->identity().PublicKey];
				}

				return keyStatistics;
			}
		};

		struct SelectNodesTraits {
		public:
			using KeyStatistics = std::unordered_map<Key, uint32_t, utils::ArrayHasher<Key>>;
			static constexpr auto Description = "select nodes and weight correlation";

			static KeyStatistics CreateStatistics(
					const std::vector<ionet::Node>& nodes,
					const std::vector<uint64_t>& rawWeights,
					uint64_t numIterations) {
				// both interaction stats and importances are chosen in such a way that they yield the same node weight
				KeyStatistics keyStatistics;
				ionet::NodeContainer container;
				SeedNodeContainer(container, nodes, CreateInteractionSeeds(rawWeights));
				NodeSelectionConfiguration config{ Default_Service_Id, ionet::NodeRoles::None, 1, 1234 };
				for (auto i = 0u; i < numIterations; ++i) {
					auto nodeSelectionResult = SelectNodes(container, config, CreateImportanceRetriever(nodes, rawWeights));
					const auto& selectedNodes = nodeSelectionResult.AddCandidates;
					if (1u != selectedNodes.size())
						CATAPULT_THROW_RUNTIME_ERROR_1("unexpected number of nodes were selected", selectedNodes.size());

					++keyStatistics[selectedNodes.cbegin()->identity().PublicKey];
				}

				return keyStatistics;
			}

		private:
			static void SeedNodeContainer(
					ionet::NodeContainer& container,
					const std::vector<ionet::Node>& nodes,
					const std::vector<ionet::NodeInteractions>& interactionsSeeds) {
				auto modifier = container.modifier();
				for (auto i = 0u; i < nodes.size(); ++i) {
					modifier.add(nodes[i], ionet::NodeSource::Dynamic);
					modifier.provisionConnectionState(Default_Service_Id, nodes[i].identity());
					auto& interactions = interactionsSeeds[i];
					test::AddNodeInteractions(modifier, nodes[i].identity(), interactions.NumSuccesses, interactions.NumFailures);
				}
			}

			static std::vector<ionet::NodeInteractions> CreateInteractionSeeds(const std::vector<uint64_t>& rawWeights) {
				// weights have to be in the range from 1 to 10'000
				// the test needs values for NumSuccesses and NumFailures that result in node weight == scaledRawWeight;
				// after setting node weight and NumSuccesses to scaledRawWeight, NumFailures can be calculated
				auto maxWeight = *std::max_element(rawWeights.cbegin(), rawWeights.cend());
				std::vector<ionet::NodeInteractions> interactionsSeeds;
				for (auto rawWeight : rawWeights) {
					auto scaledRawWeight = static_cast<uint32_t>(rawWeight * 10'000ull / maxWeight);
					ionet::NodeInteractions interactions;
					interactions.NumSuccesses = scaledRawWeight;
					interactions.NumFailures = (10'000 - scaledRawWeight) / 9;
					interactionsSeeds.push_back(interactions);
				}

				return interactionsSeeds;
			}

			static ImportanceRetriever CreateImportanceRetriever(
					const std::vector<ionet::Node>& nodes,
					const std::vector<uint64_t>& rawWeights) {
				// weights have to be in the range from 1 to 10'000
				// the test needs values for importance and total chain balance that result in
				// node weight == rawWeight * 10'000ull / maxWeight;
				// setting totalChainImportance = totalWeight, the importance can be calculated from
				// rawWeight * 10'000 / maxWeight = importance * 30'000'000 / totalChainImportance
				auto maxWeight = *std::max_element(rawWeights.cbegin(), rawWeights.cend());
				auto totalWeight = std::accumulate(rawWeights.cbegin(), rawWeights.cend(), 0ull);
				std::map<Key, ImportanceDescriptor> map;
				for (auto i = 0u; i < nodes.size(); ++i) {
					auto rawImportance = totalWeight * rawWeights[i] / (3'000 * maxWeight);
					map.emplace(nodes[i].identity().PublicKey, ImportanceDescriptor{ Importance(rawImportance), Importance(totalWeight) });
				}

				return [map](const Key& key) {
					return map.find(key)->second;
				};
			}
		};
	}

	DEFINE_NODE_SELECTOR_PROBABILITY_TESTS(SelectCandidatesBasedOnWeight)
	DEFINE_NODE_SELECTOR_PROBABILITY_TESTS(SelectNodes)

	// endregion

	// region SelectNodes: no matching candidates in container

	TEST(TEST_CLASS, NoAddCandidatesWhenContainerIsEmpty) {
		// Arrange:
		ionet::NodeContainer container;

		// Act:
		auto result = SelectNodes(container, CreateConfiguration(5, 8), UniformImportanceRetriever);

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
			auto selectionConfig = NodeSelectionConfiguration{ ionet::ServiceIdentifier(2), ionet::NodeRoles::Peer, 5, maxAge };
			auto result = SelectNodes(container, selectionConfig, UniformImportanceRetriever);

			// Assert:
			EXPECT_TRUE(result.AddCandidates.empty());
			EXPECT_TRUE(result.RemoveCandidates.empty());
		}
	}

	TEST(TEST_CLASS, NoAddCandidatesWhenContainerHasNoMatchingServiceNodes) {
		AssertNoAddOrRemoveCandidatesWhenContainerHasNoMatchingServiceNodes(0, 8);
	}

	TEST(TEST_CLASS, NoRemoveCandidatesWhenContainerHasNoMatchingServiceNodes) {
		AssertNoAddOrRemoveCandidatesWhenContainerHasNoMatchingServiceNodes(10, 8);
	}

	namespace {
		void AssertNoAddOrRemoveCandidatesWhenContainerHasNoMatchingRoleNodes(uint32_t age, uint32_t maxAge) {
			// Arrange: seed the container with nodes that have a different role
			ionet::NodeContainer container;
			SetAge(container, SeedNodes(container, 10), age);

			// Act:
			auto result = SelectNodes(container, { Default_Service_Id, ionet::NodeRoles::Api, 5, maxAge }, UniformImportanceRetriever);

			// Assert:
			EXPECT_TRUE(result.AddCandidates.empty());
			EXPECT_TRUE(result.RemoveCandidates.empty());
		}
	}

	TEST(TEST_CLASS, NoAddCandidatesWhenContainerHasNoMatchingRoleNodes) {
		AssertNoAddOrRemoveCandidatesWhenContainerHasNoMatchingRoleNodes(0, 8);
	}

	TEST(TEST_CLASS, NoRemoveCandidatesWhenContainerHasNoMatchingRoleNodes) {
		AssertNoAddOrRemoveCandidatesWhenContainerHasNoMatchingRoleNodes(10, 8);
	}

	namespace {
		void AssertNoAddOrRemoveCandidatesWhenContainerHasOnlyLocalMatchingServiceNodes(uint32_t age, uint32_t maxAge) {
			// Arrange:
			ionet::NodeContainer container;
			SetAge(container, SeedNodes(container, 10, ionet::NodeSource::Local), age);

			// Act:
			auto result = SelectNodes(container, CreateConfiguration(5, maxAge), UniformImportanceRetriever);

			// Assert:
			EXPECT_TRUE(result.AddCandidates.empty());
			EXPECT_TRUE(result.RemoveCandidates.empty());
		}
	}

	TEST(TEST_CLASS, NoAddCandidatesWhenContainerHasOnlyLocalMatchingServiceNodes) {
		AssertNoAddOrRemoveCandidatesWhenContainerHasOnlyLocalMatchingServiceNodes(0, 8);
	}

	TEST(TEST_CLASS, NoRemoveCandidatesWhenContainerHasOnlyLocalMatchingServiceNodes) {
		// Assert: this is a contrived example because nodes with Local source should never be active
		AssertNoAddOrRemoveCandidatesWhenContainerHasOnlyLocalMatchingServiceNodes(10, 8);
	}

	namespace {
		void AssertNoAddOrRemoveCandidatesWhenContainerHasOnlyBannedDynamicNodes(uint32_t age, uint32_t maxAge) {
			// Arrange: seed the container with banned nodes
			ionet::NodeContainer container;
			auto nodes = SeedNodes(container, 10, ionet::NodeSource::Dynamic);
			SetAge(container, nodes, age);
			SetBanAge(container, nodes, 1);

			// Act:
			auto result = SelectNodes(container, { Default_Service_Id, ionet::NodeRoles::Peer, 5, maxAge }, UniformImportanceRetriever);

			// Assert:
			EXPECT_TRUE(result.AddCandidates.empty());
			EXPECT_TRUE(result.RemoveCandidates.empty());
		}
	}

	TEST(TEST_CLASS, NoAddCandidatesWhenContainerHasOnlyBannedDynamicNodes) {
		AssertNoAddOrRemoveCandidatesWhenContainerHasOnlyBannedDynamicNodes(0, 8);
	}

	TEST(TEST_CLASS, NoRemoveCandidatesWhenContainerHasOnlyBannedDynamicNodes) {
		AssertNoAddOrRemoveCandidatesWhenContainerHasOnlyBannedDynamicNodes(10, 8);
	}

	// endregion

	// region SelectNodes: add only

	namespace {
		void AgeAndErase(ionet::NodeContainer& container, std::vector<ionet::Node>& nodes, size_t index, uint32_t age) {
			container.modifier().provisionConnectionState(Default_Service_Id, nodes[index].identity()).Age = age;
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
			auto result = SelectNodes(container, CreateConfiguration(maxConnections, 8), UniformImportanceRetriever);

			// Assert:
			EXPECT_EQ(numExpectedAddCandidates, result.AddCandidates.size());
			AssertSubset(nodes, result.AddCandidates);
			EXPECT_TRUE(result.RemoveCandidates.empty());
		}
	}

	TEST(TEST_CLASS, ActiveAndAddCandidatesSumToMaxConnectionsWhenNoNodesAreInitiallyActive) {
		AssertAdds(5, 10, 5, [](const auto&, const auto&) {});
	}

	TEST(TEST_CLASS, ActiveAndAddCandidatesSumToMaxConnectionsWhenFewerThanMaxConnectionsNodesAreInitiallyActive) {
		AssertAdds(5, 10, 3, [](auto& container, auto& nodes) {
			// Arrange: activate 2/5 nodes
			AgeAndErase(container, nodes, 3, 1);
			AgeAndErase(container, nodes, 1, 1);
		});
	}

	TEST(TEST_CLASS, ActiveAndAddCandidatesSumToMaxConnectionsWhenOneLessThanMaxConnectionsNodesAreInitiallyActive) {
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
		auto result = SelectNodes(container, CreateConfiguration(5, 8), UniformImportanceRetriever);

		// Assert: nodes were selected even though they support BOTH Api and Peer roles
		EXPECT_EQ(5u, result.AddCandidates.size());
		AssertSubset(nodes, result.AddCandidates);
		EXPECT_TRUE(result.RemoveCandidates.empty());
	}

	// endregion

	// region SelectNodes: banned nodes filtering

	TEST(TEST_CLASS, BannedNodesAreNotSelected) {
		// Arrange:
		ionet::BanSettings banSettings;
		banSettings.DefaultBanDuration = utils::TimeSpan::FromHours(1);
		banSettings.MaxBanDuration = utils::TimeSpan::FromHours(2);
		banSettings.KeepAliveDuration = utils::TimeSpan::FromHours(3);
		banSettings.MaxBannedNodes = 100;
		ionet::NodeContainer container(
				100,
				model::NodeIdentityEqualityStrategy::Key,
				banSettings,
				[]() { return Timestamp(1); },
				[](auto) { return true; });
		auto nodes = SeedNodes(container, 10, ionet::NodeSource::Dynamic);

		{
			auto modifier = container.modifier();
			for (auto i = 1u; i < nodes.size(); ++i)
				modifier.ban(nodes[i].identity(), 0x123);
		}

		// Act:
		auto result = SelectNodes(container, CreateConfiguration(5, 8), UniformImportanceRetriever);

		// Assert: only the unbanned node was selected
		ASSERT_EQ(1u, result.AddCandidates.size());
		EXPECT_EQ(nodes[0].identity().PublicKey, result.AddCandidates.cbegin()->identity().PublicKey);
		EXPECT_TRUE(result.RemoveCandidates.empty());
	}

	// endregion

	// region SelectNodes: relative weighting

	namespace {
		struct NodeInfos {
		public:
			NodeInfos(const ionet::NodeInteractions& interactions1, const ionet::NodeInteractions& interactions2)
					: Interactions1(interactions1)
					, Interactions2(interactions2)
					, Source1(ionet::NodeSource::Dynamic)
					, Source2(ionet::NodeSource::Dynamic)
			{}

		public:
			const ionet::NodeInteractions& Interactions1;
			const ionet::NodeInteractions& Interactions2;
			ionet::NodeSource Source1;
			ionet::NodeSource Source2;
			ionet::ConnectionState ConnectionState1;
			ionet::ConnectionState ConnectionState2;
		};

		std::pair<uint32_t, uint32_t> RunManyPairwiseSelections(const NodeInfos& nodeInfos) {
			// Arrange: seed two inactive nodes
			ionet::NodeContainer container;
			auto node1 = SeedNodes(container, 1, nodeInfos.Source1)[0];
			auto node2 = SeedNodes(container, 1, nodeInfos.Source2)[0];
			{
				auto modifier = container.modifier();
				modifier.provisionConnectionState(Default_Service_Id, node1.identity()) = nodeInfos.ConnectionState1;
				modifier.provisionConnectionState(Default_Service_Id, node2.identity()) = nodeInfos.ConnectionState2;
				auto& interactions1 = nodeInfos.Interactions1;
				test::AddNodeInteractions(modifier, node1.identity(), interactions1.NumSuccesses, interactions1.NumFailures);
				auto& interactions2 = nodeInfos.Interactions2;
				test::AddNodeInteractions(modifier, node2.identity(), interactions2.NumSuccesses, interactions2.NumFailures);
			}

			// Act: run a lot of selections
			std::pair<uint32_t, uint32_t> counts(0, 0);
			for (auto i = 0u; i < 1000; ++i) {
				auto result = SelectNodes(container, CreateConfiguration(1, 8), UniformImportanceRetriever);
				if (1 != result.AddCandidates.size())
					CATAPULT_THROW_RUNTIME_ERROR("unexpected number of candidate nodes returned");

				if (node1.identity().PublicKey == result.AddCandidates.cbegin()->identity().PublicKey)
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
		auto interactions1 = ionet::NodeInteractions();
		auto interactions2 = ionet::NodeInteractions(5, 0);

		// Assert:
		RunNonDeterministicPairwiseSelectionTest(NodeInfos(interactions1, interactions2), [](const auto& counts) {
			return counts.first < counts.second;
		});
	}

	TEST(TEST_CLASS, NewNodeHasHigherPriorityThanNodeWithFailures) {
		// Arrange:
		auto interactions1 = ionet::NodeInteractions();
		auto interactions2 = ionet::NodeInteractions(0, 5);

		// Assert:
		RunNonDeterministicPairwiseSelectionTest(NodeInfos(interactions1, interactions2), [](const auto& counts) {
			return counts.first > counts.second;
		});
	}

	TEST(TEST_CLASS, StaticNodeHasHigherPriorityThanDynamicNode) {
		// Arrange:
		auto interactions1 = ionet::NodeInteractions();
		auto interactions2 = ionet::NodeInteractions();
		NodeInfos nodeInfos(interactions1, interactions2);
		nodeInfos.Source1 = ionet::NodeSource::Static;
		nodeInfos.Source2 = ionet::NodeSource::Dynamic;

		// Assert:
		RunNonDeterministicPairwiseSelectionTest(nodeInfos, [](const auto& counts) {
			return counts.second < counts.first && counts.first < 5 * counts.second;
		});
	}

	TEST(TEST_CLASS, DynamicNodeWithLargeWeightHasHigherPriorityThanStaticNodeWithSmallWeight) {
		// Arrange: weights 4000 / 9500
		auto interactions1 = ionet::NodeInteractions(1, 4);
		auto interactions2 = ionet::NodeInteractions(95, 5);
		NodeInfos nodeInfos(interactions1, interactions2);
		nodeInfos.Source1 = ionet::NodeSource::Static;
		nodeInfos.Source2 = ionet::NodeSource::Dynamic;

		// Assert:
		RunNonDeterministicPairwiseSelectionTest(nodeInfos, [](const auto& counts) {
			return counts.first < counts.second;
		});
	}

	TEST(TEST_CLASS, BannedStaticNodeHasLowerPriorityThanNonBannedStaticNode) {
		// Arrange:
		auto interactions1 = ionet::NodeInteractions();
		auto interactions2 = ionet::NodeInteractions();
		NodeInfos nodeInfos(interactions1, interactions2);
		nodeInfos.Source1 = ionet::NodeSource::Static;
		nodeInfos.Source2 = ionet::NodeSource::Static;
		nodeInfos.ConnectionState2.BanAge = 1;

		// Assert:
		RunNonDeterministicPairwiseSelectionTest(nodeInfos, [](const auto& counts) {
			return counts.second < counts.first && counts.first < 5 * counts.second;
		});
	}

	TEST(TEST_CLASS, BannedDynamicNodeHasLowerPriorityThanNonBannedDynamicNode) {
		// Arrange:
		auto interactions1 = ionet::NodeInteractions();
		auto interactions2 = ionet::NodeInteractions();
		NodeInfos nodeInfos(interactions1, interactions2);
		nodeInfos.Source1 = ionet::NodeSource::Dynamic;
		nodeInfos.Source2 = ionet::NodeSource::Dynamic;
		nodeInfos.ConnectionState2.BanAge = 1;

		// Assert:
		RunNonDeterministicPairwiseSelectionTest(nodeInfos, [](const auto& counts) {
			return 0 == counts.second;
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
			auto result = SelectNodes(container, CreateConfiguration(maxConnections, 8), UniformImportanceRetriever);

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
				container.modifier().provisionConnectionState(Default_Service_Id, node.identity()).Age = 8 + i++;

			// Act:
			auto result = SelectNodes(container, CreateConfiguration(maxConnections, 8), UniformImportanceRetriever);

			// Assert:
			EXPECT_TRUE(result.AddCandidates.empty());
			EXPECT_EQ(numExpectedRemoveCandidates, result.RemoveCandidates.size());
			AssertSubset(nodes, result.RemoveCandidates);
		}
	}

	TEST(TEST_CLASS, NoRemoveCandidatesWhenSingleConnectionIsActiveAndNodesHaveMaxAge) {
		// Assert: last connection should never be removed
		AssertRemovalsButNoAdds(5, 1, 0);
	}

	TEST(TEST_CLASS, SingleRemoveCandidateWhenLessThanMaxConnectionsAreActiveAndNodesHaveMaxAge) {
		// Assert: single aged connection should be closed
		AssertRemovalsButNoAdds(5, 4, 1);
	}

	TEST(TEST_CLASS, SingleRemoveCandidateWhenMaxConnectionsAreActiveAndNodesHaveMaxAgeAndNoConnectionsAreInactive) {
		// Assert: single aged connection should be closed
		AssertRemovalsButNoAdds(5, 5, 1);
	}

	TEST(TEST_CLASS, MultipleRemoveCandidatesWhenGreaterThanMaxConnectionsAreActiveAndNoConnectionsAreInactive) {
		// Assert: one more than additional aged connections over max should be closed
		AssertRemovalsButNoAdds(5, 7, 3);
	}

	namespace {
		void AssertRemovalsAndSingleAdd(uint32_t maxConnections, uint32_t numActiveNodes, uint32_t numExpectedRemoveCandidates) {
			// Arrange: seed active that all have at least max age and inactive nodes
			ionet::NodeContainer container;
			auto inactiveNodes = SeedNodes(container, 2);
			auto activeNodes = SeedNodes(container, numActiveNodes);
			auto i = 0u;
			for (const auto& node : activeNodes)
				container.modifier().provisionConnectionState(Default_Service_Id, node.identity()).Age = 8 + i++;

			// Act:
			auto result = SelectNodes(container, CreateConfiguration(maxConnections, 8), UniformImportanceRetriever);

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

	// region SelectNodesForRemoval: no matching candidates in container

	namespace {
		NodeAgingConfiguration CreateAgingConfiguration(uint32_t maxConnections, uint32_t maxConnectionAge) {
			return { Default_Service_Id, maxConnections, maxConnectionAge };
		}
	}

	TEST(TEST_CLASS, ForRemoval_NoCandidatesWhenContainerIsEmpty) {
		// Arrange:
		ionet::NodeContainer container;

		// Act:
		auto removeCandidates = SelectNodesForRemoval(container, CreateAgingConfiguration(5, 8), UniformImportanceRetriever);

		// Assert:
		EXPECT_TRUE(removeCandidates.empty());
	}

	TEST(TEST_CLASS, ForRemoval_NoCandidatesWhenContainerHasNoMatchingServiceNodes) {
		// Arrange: seed the container with nodes that support a different service
		ionet::NodeContainer container;
		SetAge(container, SeedNodes(container, 10), 10);

		// Act:
		auto removeCandidates = SelectNodesForRemoval(container, { ionet::ServiceIdentifier(2), 5, 8 }, UniformImportanceRetriever);

		// Assert:
		EXPECT_TRUE(removeCandidates.empty());
	}

	TEST(TEST_CLASS, ForRemoval_NoCandidatesWhenContainerHasOnlyLocalMatchingServiceNodes) {
		// Arrange: this is a contrived example because nodes with Local source should never be active
		ionet::NodeContainer container;
		SetAge(container, SeedNodes(container, 10, ionet::NodeSource::Local), 10);

		// Act:
		auto removeCandidates = SelectNodesForRemoval(container, CreateAgingConfiguration(5, 8), UniformImportanceRetriever);

		// Assert:
		EXPECT_TRUE(removeCandidates.empty());
	}

	TEST(TEST_CLASS, ForRemoval_NoCandidatesWhenContainerHasOnlyBannedDynamicNodes) {
		// Arrange: seed the container with banned nodes
		ionet::NodeContainer container;
		auto nodes = SeedNodes(container, 10, ionet::NodeSource::Dynamic);
		SetAge(container, nodes, 10);
		SetBanAge(container, nodes, 1);

		// Act:
		auto removeCandidates = SelectNodesForRemoval(container, CreateAgingConfiguration(5, 8), UniformImportanceRetriever);

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
			auto agingConfig = CreateAgingConfiguration(maxConnections, 8);
			auto removeCandidates = SelectNodesForRemoval(container, agingConfig, UniformImportanceRetriever);

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

	// endregion

	// region SelectNodesForRemoval: removals

	namespace {
		void AssertRemoveOnlyRemovals(uint32_t maxConnections, uint32_t numActiveNodes, uint32_t numExpectedRemoveCandidates) {
			// Arrange: seed only active nodes that all have at least max age
			ionet::NodeContainer container;
			auto nodes = SeedNodes(container, numActiveNodes);
			auto i = 0u;
			for (const auto& node : nodes)
				container.modifier().provisionConnectionState(Default_Service_Id, node.identity()).Age = 8 + i++;

			// Act:
			auto agingConfig = CreateAgingConfiguration(maxConnections, 8);
			auto removeCandidates = SelectNodesForRemoval(container, agingConfig, UniformImportanceRetriever);

			// Assert:
			EXPECT_EQ(numExpectedRemoveCandidates, removeCandidates.size());
			AssertSubset(nodes, removeCandidates);
		}
	}

	TEST(TEST_CLASS, ForRemoval_NoRemoveCandidatesWhenSingleConnectionIsActiveAndNodesHaveMaxAge) {
		// Assert: last connection should never be removed
		AssertRemoveOnlyRemovals(5, 1, 0);
	}

	TEST(TEST_CLASS, ForRemoval_SingleRemoveCandidatesWhenLessThanMinimumConnectionsAreActive) {
		// Assert: single aged connection should be closed - min connections (5 * 3 / 4 == 3)
		AssertRemoveOnlyRemovals(5, 2, 1);
	}

	TEST(TEST_CLASS, ForRemoval_SingleRemoveCandidatesWhenMinimumConnectionsAreActive) {
		// Assert: single aged connection should be closed - min connections (5 * 3 / 4 == 3)
		AssertRemoveOnlyRemovals(5, 3, 1);
	}

	TEST(TEST_CLASS, ForRemoval_MultipleRemoveCandidatesWhenMaxConnectionsAreActive) {
		// Assert: one more than additional aged connections over max should be closed - min connections (5 * 3 / 4 == 3)
		AssertRemoveOnlyRemovals(5, 5, 3);
	}

	TEST(TEST_CLASS, ForRemoval_MultipleRemoveCandidatesWhenGreaterThanMaxConnectionsAreActive) {
		// Assert: one more than additional aged connections over max should be closed - min connections (5 * 3 / 4 == 3)
		AssertRemoveOnlyRemovals(5, 8, 6);
	}

	TEST(TEST_CLASS, ForRemoval_NoRemoveCandidatesWhenMaxConnectionsIsOne) {
		// Assert: last connection should never be removed
		AssertRemoveOnlyRemovals(1, 1, 0);
	}

	// endregion
}}
