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

#include "catapult/cache_tx/AccountWeights.h"
#include "catapult/utils/Functional.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS AccountWeightsTests

	namespace {
		// region test utils

		enum class OperationType { Increment, Decrement };

		void UpdateWeight(AccountWeights& weights, const Key& key, uint64_t delta, OperationType operationType) {
			if (OperationType::Increment == operationType)
				weights.increment(key, delta);
			else
				weights.decrement(key, delta);
		}

		AccountWeights CreateInitialWeights(const std::vector<Key>& keys, uint64_t initialWeight) {
			AccountWeights weights;
			for (auto i = 0u; i < keys.size(); ++i)
				weights.increment(keys[i], initialWeight);

			// Sanity:
			EXPECT_EQ(initialWeight ? keys.size() : 0u, weights.size());
			EXPECT_EQ(keys.size() * initialWeight, weights.totalWeight());
			for (const auto& key : keys)
				EXPECT_EQ(initialWeight, weights.weight(key));

			return weights;
		}

		void AssertWeights(
				const std::vector<Key>& keys,
				uint64_t initialWeight,
				const std::vector<uint64_t>& numOperationsOnKeys,
				OperationType operationType,
				const std::vector<uint64_t>& expectedWeights) {
			// Sanity:
			ASSERT_EQ(keys.size(), numOperationsOnKeys.size());
			ASSERT_EQ(expectedWeights.size(), numOperationsOnKeys.size());

			// Arrange:
			auto weights = CreateInitialWeights(keys, initialWeight);

			// Act:
			for (auto i = 0u; i < keys.size(); ++i)
				UpdateWeight(weights, keys[i], numOperationsOnKeys[i], operationType);

			// Assert:
			auto expectedTotalWeight = utils::Sum(expectedWeights, [](auto weight) { return weight; });
			EXPECT_EQ(keys.size(), weights.size());
			EXPECT_EQ(expectedTotalWeight, weights.totalWeight());

			for (auto i = 0u; i < keys.size(); ++i)
				EXPECT_EQ(expectedWeights[i], weights.weight(keys[i])) << "at index " << i;
		}

		// endregion
	}

	// region basic

	TEST(TEST_CLASS, InitiallyWeightsAreEmpty) {
		// Act:
		AccountWeights weights;

		// Assert:
		EXPECT_EQ(0u, weights.size());
		EXPECT_EQ(0u, weights.totalWeight());
	}

	TEST(TEST_CLASS, UnknownKeyHasZeroWeight) {
		// Act:
		auto keys = test::GenerateRandomDataVector<Key>(3);
		auto weights = CreateInitialWeights(keys, 10);

		// Assert:
		for (auto i = 0u; i < 10; ++i)
			EXPECT_EQ(0u, weights.weight(test::GenerateRandomByteArray<Key>()));
	}

	// endregion

	// region increment

	TEST(TEST_CLASS, IncrementIncreasesWeightForPublicKey_SingleKey) {
		AssertWeights(test::GenerateRandomDataVector<Key>(1), 0, { 8 }, OperationType::Increment, { 8 });
	}

	TEST(TEST_CLASS, IncrementIncreasesWeightForPublicKey_MultipleKeys) {
		AssertWeights(test::GenerateRandomDataVector<Key>(5), 0, { 1, 4, 3, 7, 2 }, OperationType::Increment, { 1, 4, 3, 7, 2 });
	}

	// endregion

	// region decrement

	TEST(TEST_CLASS, DecrementDecreasesWeightForPublicKey_SingleKey) {
		AssertWeights(test::GenerateRandomDataVector<Key>(1), 10, { 1 }, OperationType::Decrement, { 9 });
	}

	TEST(TEST_CLASS, DecrementDecreasesWeightForPublicKey_MultipleKeys) {
		AssertWeights(test::GenerateRandomDataVector<Key>(5), 10, { 1, 4, 3, 7, 2 }, OperationType::Decrement, { 9, 6, 7, 3, 8 });
	}

	TEST(TEST_CLASS, CannotDecrementWeightBelowZero) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		AccountWeights weights;
		weights.increment(key, 9);

		// Act + Assert:
		EXPECT_THROW(weights.decrement(key, 10), catapult_runtime_error);
		EXPECT_THROW(weights.decrement(key, 11), catapult_runtime_error);
	}

	TEST(TEST_CLASS, DecrementRemovesEntryWhenWeightIsZero) {
		// Arrange:
		auto keys = test::GenerateRandomDataVector<Key>(10);
		auto weights = CreateInitialWeights(keys, 3);

		// Act:
		for (auto index : { 1u, 4u, 5u, 8u, 9u })
			weights.decrement(keys[index], 3);

		// Assert:
		EXPECT_EQ(5u, weights.size());
		for (auto index : { 0u, 2u, 3u, 6u, 7u })
			EXPECT_EQ(3u, weights.weight(keys[index])) << "at index " << index;

		for (auto index : { 1u, 4u, 5u, 8u, 9u })
			EXPECT_EQ(0u, weights.weight(keys[index])) << "at index " << index;
	}

	// endregion

	// region mixed increment / decrement

	TEST(TEST_CLASS, IncrementsAreCumulative) {
		// Arrange:
		auto keys = test::GenerateRandomDataVector<Key>(3);
		auto key = keys[0];
		auto weights = CreateInitialWeights(keys, 10);

		// Act: 2 + 2 + 1 = 5
		weights.increment(key, 2);
		weights.increment(key, 2);
		weights.increment(key, 1);

		// Assert:
		EXPECT_EQ(3u, weights.size());
		EXPECT_EQ(35u, weights.totalWeight());
		EXPECT_EQ(15u, weights.weight(key));
	}

	TEST(TEST_CLASS, IncrementsAndDecrementsAreCumulative) {
		// Arrange:
		auto keys = test::GenerateRandomDataVector<Key>(3);
		auto key = keys[0];
		auto weights = CreateInitialWeights(keys, 10);

		// Act: 2 + 2 - 3 + 1 - 1 = 1
		weights.increment(key, 2);
		weights.increment(key, 2);
		weights.decrement(key, 3);
		weights.increment(key, 1);
		weights.decrement(key, 1);

		// Assert:
		EXPECT_EQ(3u, weights.size());
		EXPECT_EQ(31u, weights.totalWeight());
		EXPECT_EQ(11u, weights.weight(key));
	}

	// endregion

	// region reset

	TEST(TEST_CLASS, WeightsCanBeReset) {
		// Arrange:
		auto keys = test::GenerateRandomDataVector<Key>(5);
		auto weights = CreateInitialWeights(keys, 10);

		// Act:
		weights.reset();

		// Assert:
		EXPECT_EQ(0u, weights.size());
		EXPECT_EQ(0u, weights.totalWeight());
		EXPECT_EQ(0u, weights.weight(keys[0]));
	}

	// endregion
}}
