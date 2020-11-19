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

#include "catapult/ionet/NodeInteractionsContainer.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace ionet {

#define TEST_CLASS NodeInteractionsContainerTests

	// region constants + constructor

	TEST(TEST_CLASS, ConstantsHaveExpectedValues) {
		EXPECT_EQ(utils::TimeSpan::FromSeconds(24 * 60 * 60), NodeInteractionsContainer::BucketDuration());
		EXPECT_EQ(utils::TimeSpan::FromSeconds(7 * 24 * 60 * 60), NodeInteractionsContainer::InteractionDuration());
	}

	TEST(TEST_CLASS, CanCreateNodeInteractionsContainer) {
		// Act:
		NodeInteractionsContainer container;
		auto interactions = container.interactions(Timestamp());

		// Assert:
		EXPECT_EQ(0u, interactions.NumSuccesses);
		EXPECT_EQ(0u, interactions.NumFailures);
	}

	// endregion

	// region add interaction

	namespace {
		template<typename TInteraction>
		void AssertCanAddInteraction(
				uint32_t expectedNumSuccesses,
				uint32_t expectedNumFailures,
				size_t numInteractions,
				TInteraction interaction) {
			// Arrange:
			NodeInteractionsContainer container;

			// Act:
			for (auto i = 0u; i < numInteractions; ++i)
				interaction(container, Timestamp(100));

			// Assert: interaction lifetime is [0, 100 + NodeInteractionsContainer::InteractionDuration().millis()]
			test::AssertNodeInteractions(expectedNumSuccesses, expectedNumFailures, container.interactions(Timestamp(0)), "0");
			test::AssertNodeInteractions(expectedNumSuccesses, expectedNumFailures, container.interactions(Timestamp(99)), "99");
			test::AssertNodeInteractions(expectedNumSuccesses, expectedNumFailures, container.interactions(Timestamp(100)), "100");

			auto maxAliveTimestamp = Timestamp(100 + NodeInteractionsContainer::InteractionDuration().millis() - 1);
			test::AssertNodeInteractions(expectedNumSuccesses, expectedNumFailures, container.interactions(maxAliveTimestamp), "max");

			test::AssertNodeInteractions(0, 0, container.interactions(maxAliveTimestamp + Timestamp(1)), "max + 1");
			test::AssertNodeInteractions(0, 0, container.interactions(maxAliveTimestamp + Timestamp(10000)), "max + 10000");
		}
	}

	TEST(TEST_CLASS, CanAddSuccessfulInteraction_Single) {
		// Act:
		AssertCanAddInteraction(1, 0, 1, [](auto& container, auto timestamp) { container.incrementSuccesses(timestamp); });
	}

	TEST(TEST_CLASS, CanAddSuccessfulInteraction_Multiple) {
		// Act:
		AssertCanAddInteraction(5, 0, 5, [](auto& container, auto timestamp) { container.incrementSuccesses(timestamp); });
	}

	TEST(TEST_CLASS, CanAddFailedInteraction_Single) {
		// Act:
		AssertCanAddInteraction(0, 1, 1, [](auto& container, auto timestamp) { container.incrementFailures(timestamp); });
	}

	TEST(TEST_CLASS, CanAddFailedInteraction_Multiple) {
		// Act:
		AssertCanAddInteraction(0, 5, 5, [](auto& container, auto timestamp) { container.incrementFailures(timestamp); });
	}

	TEST(TEST_CLASS, CanAddInteraction_Mixed) {
		// Act:
		AssertCanAddInteraction(3, 6, 3, [](auto& container, auto timestamp) {
			container.incrementSuccesses(timestamp);
			container.incrementFailures(timestamp);
			container.incrementFailures(timestamp);
		});
	}

	TEST(TEST_CLASS, AddInteractionDoesNotPerformAnyPruning) {
		// Arrange:
		NodeInteractionsContainer container;

		// Act: interactions in buckets { 1, 2, 4 }
		auto maxLifetimeMillis = NodeInteractionsContainer::InteractionDuration().millis();
		container.incrementSuccesses(Timestamp()); // bucket 1
		container.incrementFailures(Timestamp(23456));
		container.incrementFailures(Timestamp(maxLifetimeMillis)); // bucket 2
		container.incrementFailures(Timestamp(maxLifetimeMillis + 1000));
		container.incrementSuccesses(Timestamp(3 * maxLifetimeMillis + 1)); // bucket 4
		container.incrementSuccesses(Timestamp(3 * maxLifetimeMillis + 1888));

		// Assert:
		auto bucket1MaxTime = Timestamp(maxLifetimeMillis - 1);
		auto bucket2MaxTime = Timestamp(2 * maxLifetimeMillis - 1);
		auto bucket3MaxTime = Timestamp(3 * maxLifetimeMillis - 1);
		auto bucket4MaxTime = Timestamp(4 * maxLifetimeMillis - 1);

		test::AssertNodeInteractions(3, 3, container.interactions(bucket1MaxTime), "bucket 1 max time");
		test::AssertNodeInteractions(2, 2, container.interactions(bucket2MaxTime), "bucket 2 max time");
		test::AssertNodeInteractions(2, 0, container.interactions(bucket3MaxTime), "bucket 3 max time");
		test::AssertNodeInteractions(2, 0, container.interactions(bucket4MaxTime), "bucket 4 max time");
	}

	TEST(TEST_CLASS, AddInteractionAlwaysAppendsToLastBucket) {
		// Arrange:
		NodeInteractionsContainer container;

		// Act: interactions in buckets { 1, 2, 4 } (same as previous test except one untimed interaction is added to each bucket)
		auto maxLifetimeMillis = NodeInteractionsContainer::InteractionDuration().millis();
		container.incrementSuccesses(Timestamp()); // bucket 1
		container.incrementFailures(Timestamp(23456));
		container.incrementSuccesses(Timestamp());
		container.incrementFailures(Timestamp(maxLifetimeMillis)); // bucket 2
		container.incrementFailures(Timestamp(maxLifetimeMillis + 1000));
		container.incrementSuccesses(Timestamp());
		container.incrementSuccesses(Timestamp(3 * maxLifetimeMillis + 1)); // bucket 4
		container.incrementSuccesses(Timestamp(3 * maxLifetimeMillis + 1888));
		container.incrementSuccesses(Timestamp());

		// Assert:
		auto bucket1MaxTime = Timestamp(maxLifetimeMillis - 1);
		auto bucket2MaxTime = Timestamp(2 * maxLifetimeMillis - 1);
		auto bucket3MaxTime = Timestamp(3 * maxLifetimeMillis - 1);
		auto bucket4MaxTime = Timestamp(4 * maxLifetimeMillis - 1);

		test::AssertNodeInteractions(6, 3, container.interactions(bucket1MaxTime), "bucket 1 max time");
		test::AssertNodeInteractions(4, 2, container.interactions(bucket2MaxTime), "bucket 2 max time");
		test::AssertNodeInteractions(3, 0, container.interactions(bucket3MaxTime), "bucket 3 max time");
		test::AssertNodeInteractions(3, 0, container.interactions(bucket4MaxTime), "bucket 4 max time");
	}

	// endregion

	// region pruning

	namespace {
		void AssertPruningBehavior(uint32_t expectedNumSuccesses, uint32_t expectedNumFailures, const std::vector<Timestamp>& timestamps) {
			// Arrange:
			NodeInteractionsContainer container;

			// Act: add 3 interactions
			container.incrementSuccesses(timestamps[0]);
			container.pruneBuckets(timestamps[0]);
			container.incrementFailures(timestamps[1]);
			container.pruneBuckets(timestamps[1]);
			container.incrementFailures(timestamps[2]);
			container.pruneBuckets(timestamps[2]);

			// Assert:
			test::AssertNodeInteractions(expectedNumSuccesses, expectedNumFailures, container.interactions(timestamps[3]), "");
		}
	}

	TEST(TEST_CLASS, NoPruningWhenAllBucketsAreLessThanOneWeekOld) {
		AssertPruningBehavior(1, 2, {
			Timestamp(), // first bucket is added
			Timestamp(23456),
			Timestamp(NodeInteractionsContainer::InteractionDuration().millis() - 1), // second bucket is added
			Timestamp(NodeInteractionsContainer::InteractionDuration().millis() - 1) // interactions retrieved
		});
	}

	TEST(TEST_CLASS, PrunesWhenBucketAreAtLeastOneWeekOld) {
		AssertPruningBehavior(0, 1, {
			Timestamp(), // first bucket is added
			Timestamp(23456),
			Timestamp(NodeInteractionsContainer::InteractionDuration().millis()), // first bucket is removed, another bucket is added
			Timestamp(NodeInteractionsContainer::InteractionDuration().millis()) // interactions retrieved
		});
	}

	// endregion
}}
