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

#include "catapult/cache/AccountCounters.h"
#include "catapult/utils/Functional.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS AccountCountersTests

	namespace {
		enum class OperationType { Increment, Decrement };

		void UpdateUseCount(AccountCounters& counters, const Key& key, size_t delta, OperationType operationType) {
			for (auto i = 0u; i < delta; ++i) {
				if (OperationType::Increment == operationType)
					counters.increment(key);
				else
					counters.decrement(key);
			}
		}

		AccountCounters CreateInitialCounters(const std::vector<Key>& keys, size_t initialUseCount) {
			AccountCounters counters;
			for (auto i = 0u; i < keys.size(); ++i) {
				for (auto j = 0u; j < initialUseCount; ++j)
					counters.increment(keys[i]);
			}

			// Sanity:
			EXPECT_EQ(initialUseCount ? keys.size() : 0u, counters.size());
			EXPECT_EQ(keys.size() * initialUseCount, counters.deepSize());
			for (const auto& key : keys)
				EXPECT_EQ(initialUseCount, counters.count(key));

			return counters;
		}

		void AssertCounters(
				const std::vector<Key>& keys,
				size_t initialUseCount,
				const std::vector<size_t>& numOperationsOnKeys,
				OperationType operationType,
				const std::vector<size_t>& expectedCounts) {
			// Sanity:
			ASSERT_EQ(keys.size(), numOperationsOnKeys.size());
			ASSERT_EQ(expectedCounts.size(), numOperationsOnKeys.size());

			// Arrange:
			auto counters = CreateInitialCounters(keys, initialUseCount);

			// Act:
			for (auto i = 0u; i < keys.size(); ++i)
				UpdateUseCount(counters, keys[i], numOperationsOnKeys[i], operationType);

			// Assert:
			auto expectedDeepSize = utils::Sum(expectedCounts, [](auto count) { return count; });
			EXPECT_EQ(keys.size(), counters.size());
			EXPECT_EQ(expectedDeepSize, counters.deepSize());

			for (auto i = 0u; i < keys.size(); ++i)
				EXPECT_EQ(expectedCounts[i], counters.count(keys[i])) << "at index " << i;
		}
	}

	// region basic

	TEST(TEST_CLASS, InitiallyCountersAreEmpty) {
		// Act:
		AccountCounters counters;

		// Assert:
		EXPECT_EQ(0u, counters.size());
		EXPECT_EQ(0u, counters.deepSize());
	}

	TEST(TEST_CLASS, UnknownKeyHasZeroCount) {
		// Act:
		auto keys = test::GenerateRandomDataVector<Key>(3);
		auto counters = CreateInitialCounters(keys, 10);

		// Assert:
		for (auto i = 0u; i < 10; ++i)
			EXPECT_EQ(0u, counters.count(test::GenerateRandomData<Key_Size>()));
	}

	// endregion

	// region increment

	TEST(TEST_CLASS, IncrementIncreasesUseCountForPublicKey_SingleKey_SingleIncrement) {
		// Act:
		AssertCounters(test::GenerateRandomDataVector<Key>(1), 0, { 1 }, OperationType::Increment, { 1 });
	}

	TEST(TEST_CLASS, IncrementIncreasesUseCountForPublicKey_SingleKey_MultipleIncrements) {
		// Act:
		AssertCounters(test::GenerateRandomDataVector<Key>(1), 0, { 8 }, OperationType::Increment, { 8 });
	}

	TEST(TEST_CLASS, IncrementIncreasesUseCountForPublicKey_MultipleKeys_MultipleIncrements) {
		// Act:
		AssertCounters(test::GenerateRandomDataVector<Key>(5), 0, { 1, 4, 3, 7, 2 }, OperationType::Increment, { 1, 4, 3, 7, 2 });
	}

	// endregion

	// region decrement

	TEST(TEST_CLASS, DecrementDecreasesUseCountForPublicKey_SingleKey_SingleDecrement) {
		// Act:
		AssertCounters(test::GenerateRandomDataVector<Key>(1), 10, { 1 }, OperationType::Decrement, { 9 });
	}

	TEST(TEST_CLASS, DecrementDecreasesUseCountForPublicKey_SingleKey_MultipleDecrements) {
		// Act:
		AssertCounters(test::GenerateRandomDataVector<Key>(1), 10, { 8 }, OperationType::Decrement, { 2 });
	}

	TEST(TEST_CLASS, DecrementDecreasesUseCountForPublicKey_MultipleKeys_MultipleDecrements) {
		// Act:
		AssertCounters(test::GenerateRandomDataVector<Key>(5), 10, { 1, 4, 3, 7, 2 }, OperationType::Decrement, { 9, 6, 7, 3, 8 });
	}

	TEST(TEST_CLASS, CannotDecrementUseCountBelowZero) {
		// Arrange:
		auto key = test::GenerateRandomData<Key_Size>();
		AccountCounters counters;
		counters.increment(key);

		// Act + Assert: first decrement is permitted, second should throw
		counters.decrement(key);
		EXPECT_EQ(0u, counters.count(key));
		EXPECT_THROW(counters.decrement(test::GenerateRandomData<Key_Size>()), catapult_runtime_error);
	}

	TEST(TEST_CLASS, DecrementRemovesEntryIfUseCountIsZero) {
		// Arrange:
		auto keys = test::GenerateRandomDataVector<Key>(10);
		auto counters = CreateInitialCounters(keys, 1);

		// Act:
		for (auto index : { 1u, 4u, 5u, 8u, 9u })
			counters.decrement(keys[index]);

		// Assert:
		EXPECT_EQ(5u, counters.size());
		for (auto index : { 0u, 2u, 3u, 6u, 7u })
			EXPECT_EQ(1u, counters.count(keys[index])) << "at index " << index;

		for (auto index : { 1u, 4u, 5u, 8u, 9u })
			EXPECT_EQ(0u, counters.count(keys[index])) << "at index " << index;
	}

	// endregion

	// region mixed increment / decrement

	TEST(TEST_CLASS, IncrementAndDecrementCanBeMixed) {
		// Arrange:
		auto keys = test::GenerateRandomDataVector<Key>(3);
		auto key = keys[0];
		auto counters = CreateInitialCounters(keys, 10);

		// Act: 1 + 1 - 1 + 1 - 1 = 1
		counters.increment(key);
		counters.increment(key);
		counters.decrement(key);
		counters.increment(key);
		counters.decrement(key);

		// Assert:
		EXPECT_EQ(3u, counters.size());
		EXPECT_EQ(31u, counters.deepSize());
		EXPECT_EQ(11u, counters.count(key));
	}

	// endregion

	// region reset

	TEST(TEST_CLASS, CountersCanBeReset) {
		// Arrange:
		auto keys = test::GenerateRandomDataVector<Key>(5);
		auto counters = CreateInitialCounters(keys, 10);

		// Act:
		counters.reset();

		// Assert:
		EXPECT_EQ(0u, counters.size());
		EXPECT_EQ(0u, counters.deepSize());
		EXPECT_EQ(0u, counters.count(keys[0]));
	}

	// endregion
}}
