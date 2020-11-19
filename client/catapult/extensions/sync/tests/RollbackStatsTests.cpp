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

#include "sync/src/RollbackStats.h"
#include "catapult/utils/NetworkTime.h"
#include "tests/TestHarness.h"

namespace catapult { namespace sync {

#define TEST_CLASS RollbackStatsTests

	namespace {
		void AssertCounters(const RollbackStats& stats, uint64_t all, uint64_t recent, uint64_t longest) {
			EXPECT_EQ(all, stats.total(RollbackCounterType::All));
			EXPECT_EQ(recent, stats.total(RollbackCounterType::Recent));
			EXPECT_EQ(longest, stats.total(RollbackCounterType::Longest));
		}

		void AddRollback(RollbackStats& stats, size_t rollbackSize) {
			stats.add(Timestamp(), rollbackSize);
		}
	}

	TEST(TEST_CLASS, CountersAreInitiallyZero) {
		// Arrange:
		RollbackStats stats;

		// Assert:
		AssertCounters(stats, 0, 0, 0);
	}

	// region common tests

	TEST(TEST_CLASS, CountersReportProperValueAfterAdd) {
		// Arrange:
		RollbackStats stats;

		// Act:
		AddRollback(stats, 3);

		// Assert:
		AssertCounters(stats, 1, 1, 3);
	}

	TEST(TEST_CLASS, UnknownCounterReturnsZero) {
		// Arrange:
		RollbackStats stats;
		AddRollback(stats, 3);

		// Assert:
		EXPECT_EQ(0u, stats.total(static_cast<RollbackCounterType>(12345)));
	}

	TEST(TEST_CLASS, CountersReportProperValueAfterMultipleAdds) {
		// Arrange:
		RollbackStats stats;

		// Act:
		AddRollback(stats, 3);
		AddRollback(stats, 5);
		AddRollback(stats, 4);

		// Assert:
		AssertCounters(stats, 3, 3, 5);
	}

	TEST(TEST_CLASS, AddingZeroDoesNotChangeCounters) {
		// Arrange:
		RollbackStats stats;
		for (auto i = 0u; i < 10; ++i)
			AddRollback(stats, 5 * i + 1);

		// Sanity:
		AssertCounters(stats, 10, 10, 46);

		// Act:
		for (auto i = 0u; i < 10; ++i)
			AddRollback(stats, 0);

		// Assert:
		AssertCounters(stats, 10, 10, 46);
	}

	// endregion

	// region prune tests

	TEST(TEST_CLASS, PruneAltersOnlyRecentCounter) {
		// Arrange:
		RollbackStats stats;
		stats.add(Timestamp(1), 3);

		// Act:
		stats.prune(Timestamp(2));

		// Assert:
		AssertCounters(stats, 1, 0, 3);
	}

	namespace {
		auto CreateStatsWithEntries() {
			RollbackStats stats;
			stats.add(Timestamp(10), 3);
			stats.add(Timestamp(11), 5);
			stats.add(Timestamp(12), 4);
			return stats;
		}
	}

	TEST(TEST_CLASS, PruneDoesNothingWhenAllEntriesAreAboveThreshold) {
		// Arrange:
		auto stats = CreateStatsWithEntries();

		// Act:
		stats.prune(Timestamp(9));

		// Assert:
		AssertCounters(stats, 3, 3, 5);
	}

	TEST(TEST_CLASS, PruneRemovesEntriesAtOrBelowThreshold) {
		// Arrange:
		auto stats = CreateStatsWithEntries();

		// Act:
		stats.prune(Timestamp(11));

		// Assert:
		AssertCounters(stats, 3, 1, 5);
	}

	TEST(TEST_CLASS, PruneRemovesAllRecentEntriesBelowThreshold) {
		// Arrange:
		auto stats = CreateStatsWithEntries();

		// Act:
		stats.prune(Timestamp(13));

		// Assert:
		AssertCounters(stats, 3, 0, 5);
	}

	// endregion
}}
