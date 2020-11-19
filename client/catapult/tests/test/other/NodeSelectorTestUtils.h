/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include "tests/TestHarness.h"
#include <random>

namespace catapult { namespace test {

	/// Test suite for testing the correlation of selected nodes and their associated weights.
	template<typename TTraits>
	class NodeSelectorProbabilityTests {
	public:
		static void AssertSelectsCandidatesBasedOnWeight() {
			RunNonDeterministicTest(TTraits::Description, []() {
				// Arrange:
				constexpr size_t Max_Percentage_Deviation = 10;
				constexpr uint64_t Num_Iterations = 10'000;
				constexpr uint64_t Cumulative_Weight = 1'500'000;
				std::vector<uint64_t> rawWeights{ 100'000, 200'000, 400'000, 800'000 };
				auto keys = GenerateRandomDataVector<Key>(rawWeights.size());
				std::vector<ionet::Node> nodes;
				for (auto i = 0u; i < keys.size(); ++i)
					nodes.push_back(CreateNamedNode(keys[i], "Node" + std::to_string(i + 1), ionet::NodeRoles::IPv4));

				// Act:
				auto keyStatistics = TTraits::CreateStatistics(nodes, rawWeights, Num_Iterations);
				auto index = 0u;
				for (const auto& key : keys) {
					auto expectedSelectionCount = rawWeights[index] * Num_Iterations / Cumulative_Weight;
					auto selectionCount = keyStatistics[key];
					auto deviation = static_cast<int64_t>(expectedSelectionCount - selectionCount);
					auto absoluteDeviation = static_cast<uint64_t>(std::abs(deviation));
					auto percentageDeviation = absoluteDeviation * 100u / expectedSelectionCount;
					if (Max_Percentage_Deviation < percentageDeviation) {
						CATAPULT_LOG(debug)
								<< "Max_Percentage_Deviation < percentageDeviation ("
								<< Max_Percentage_Deviation << " <= "
								<< percentageDeviation << ") "
								<< "for raw weight " << rawWeights[index];
						return false;
					}

					++index;
				}

				return true;
			});
		}
	};
}}

#define MAKE_NODE_SELECTOR_PROBABILITY_TEST(TRAITS_NAME, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { test::NodeSelectorProbabilityTests<TRAITS_NAME##Traits>::AssertSelectsCandidatesBasedOnWeight(); }

#define DEFINE_NODE_SELECTOR_PROBABILITY_TESTS(TRAITS_NAME) \
	MAKE_NODE_SELECTOR_PROBABILITY_TEST(TRAITS_NAME, SelectsCandidatesBasedOnWeight_##TRAITS_NAME)
