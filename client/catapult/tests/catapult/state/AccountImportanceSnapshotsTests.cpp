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

#include "catapult/state/AccountImportanceSnapshots.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS AccountImportanceSnapshotsTests

	namespace {
		// region test utils

		void AssertCurrentImportance(const AccountImportanceSnapshots& snapshots, Importance importance, model::ImportanceHeight height) {
			// Assert: accessible via current functions
			EXPECT_EQ(importance, snapshots.current());
			EXPECT_EQ(height, snapshots.height());

			// - accessible via get at height
			EXPECT_EQ(importance, snapshots.get(snapshots.height()));
		}

		void AssertHistoricalValues(
				const AccountImportanceSnapshots& snapshots,
				const std::array<std::pair<uint64_t, uint64_t>, Importance_History_Size>& expectedImportanceHeightPairs) {
			auto index = 0u;
			for (const auto& snapshot : snapshots) {
				const auto message = "height " + std::to_string(snapshot.Height.unwrap()) + ", index " + std::to_string(index);
				const auto& expectedPair = expectedImportanceHeightPairs[index];

				// Assert: iteration
				EXPECT_EQ(model::ImportanceHeight(expectedPair.second), snapshot.Height) << message;
				EXPECT_EQ(Importance(expectedPair.first), snapshot.Importance) << message;

				// - lookup availability
				EXPECT_EQ(snapshot.Importance, snapshots.get(snapshot.Height)) << message;
				++index;
			}

			// - expected number of importances
			EXPECT_EQ(Importance_History_Size, index);
		}

		void AssertEmpty(const AccountImportanceSnapshots& snapshots) {
			EXPECT_TRUE(snapshots.empty());
			EXPECT_FALSE(snapshots.active());
			AssertCurrentImportance(snapshots, Importance(0), model::ImportanceHeight(0));
			AssertHistoricalValues(snapshots, { { std::make_pair(0, 0), std::make_pair(0, 0), std::make_pair(0, 0) } });
		}

		// endregion
	}

	// region ctor

	TEST(TEST_CLASS, CanCreateSnapshots) {
		// Act:
		AccountImportanceSnapshots snapshots;

		// Assert:
		AssertEmpty(snapshots);
	}

	// endregion

	// region set

	TEST(TEST_CLASS, CanSetSnapshot) {
		// Act:
		AccountImportanceSnapshots snapshots;
		snapshots.set(Importance(123), model::ImportanceHeight(234));

		// Assert:
		EXPECT_FALSE(snapshots.empty());
		EXPECT_TRUE(snapshots.active());
		AssertCurrentImportance(snapshots, Importance(123), model::ImportanceHeight(234));
		AssertHistoricalValues(snapshots, { { std::make_pair(123, 234), std::make_pair(0, 0), std::make_pair(0, 0) } });
	}

	TEST(TEST_CLASS, CannotSetSnapshotAtLowerHeight) {
		// Arrange:
		AccountImportanceSnapshots snapshots;
		snapshots.set(Importance(123), model::ImportanceHeight(234));

		// Act + Assert:
		EXPECT_THROW(snapshots.set(Importance(246), model::ImportanceHeight(233)), catapult_runtime_error);
		EXPECT_THROW(snapshots.set(Importance(246), model::ImportanceHeight(100)), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CannotSetSnapshotAtSameHeight) {
		// Arrange:
		AccountImportanceSnapshots snapshots;
		snapshots.set(Importance(123), model::ImportanceHeight(234));

		// Act + Assert:
		EXPECT_THROW(snapshots.set(Importance(246), model::ImportanceHeight(234)), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CanSetSnapshotAtHigherHeight) {
		// Arrange:
		AccountImportanceSnapshots snapshots;
		snapshots.set(Importance(123), model::ImportanceHeight(234));

		// Act:
		snapshots.set(Importance(246), model::ImportanceHeight(235));

		// Assert:
		EXPECT_FALSE(snapshots.empty());
		EXPECT_TRUE(snapshots.active());
		AssertCurrentImportance(snapshots, Importance(246), model::ImportanceHeight(235));
		AssertHistoricalValues(snapshots, { { std::make_pair(246, 235), std::make_pair(123, 234), std::make_pair(0, 0) } });
	}

	TEST(TEST_CLASS, CanSetMaxHistoricalSnapshots) {
		// Act:
		AccountImportanceSnapshots snapshots;
		snapshots.set(Importance(123), model::ImportanceHeight(234));
		snapshots.set(Importance(222), model::ImportanceHeight(444));
		snapshots.set(Importance(111), model::ImportanceHeight(789));
		snapshots.set(Importance(345), model::ImportanceHeight(999));

		// Assert: roll over after max (3) is set
		EXPECT_FALSE(snapshots.empty());
		EXPECT_TRUE(snapshots.active());
		AssertCurrentImportance(snapshots, Importance(345), model::ImportanceHeight(999));
		AssertHistoricalValues(snapshots, { { std::make_pair(345, 999), std::make_pair(111, 789), std::make_pair(222, 444) } });
	}

	TEST(TEST_CLASS, CanSetAfterPush) {
		// Act:
		AccountImportanceSnapshots snapshots;
		snapshots.set(Importance(123), model::ImportanceHeight(234));
		snapshots.push();
		snapshots.set(Importance(111), model::ImportanceHeight(789));

		// Assert:
		EXPECT_FALSE(snapshots.empty());
		EXPECT_TRUE(snapshots.active());
		AssertCurrentImportance(snapshots, Importance(111), model::ImportanceHeight(789));
		AssertHistoricalValues(snapshots, { { std::make_pair(111, 789), std::make_pair(0, 0), std::make_pair(123, 234) } });
	}

	// endregion

	// region push / pop

	namespace {
		void Fill(AccountImportanceSnapshots& snapshots) {
			// Arrange:
			snapshots.set(Importance(123), model::ImportanceHeight(234));
			snapshots.set(Importance(222), model::ImportanceHeight(444));
			snapshots.set(Importance(111), model::ImportanceHeight(789));

			// Sanity:
			EXPECT_EQ(Importance(111), snapshots.get(model::ImportanceHeight(789)));
		}
	}

	TEST(TEST_CLASS, CanPushSnapshotWhenEmpty) {
		// Act:
		AccountImportanceSnapshots snapshots;
		snapshots.push();

		// Assert:
		AssertEmpty(snapshots);
	}

	TEST(TEST_CLASS, CanPushSnapshotWhenNotEmpty) {
		// Arrange:
		AccountImportanceSnapshots snapshots;
		Fill(snapshots);

		// Act:
		snapshots.push();

		// Assert:
		EXPECT_FALSE(snapshots.empty());
		EXPECT_FALSE(snapshots.active());
		AssertCurrentImportance(snapshots, Importance(), model::ImportanceHeight());
		AssertHistoricalValues(snapshots, { { std::make_pair(0, 0), std::make_pair(111, 789), std::make_pair(222, 444) } });
	}

	TEST(TEST_CLASS, CanPushAllSnapshots) {
		// Arrange:
		AccountImportanceSnapshots snapshots;
		Fill(snapshots);

		// Act:
		for (auto i = 0u; i < Importance_History_Size; ++i)
			snapshots.push();

		// Assert:
		AssertEmpty(snapshots);
	}

	TEST(TEST_CLASS, CanPopMostRecentSnapshot) {
		// Arrange:
		AccountImportanceSnapshots snapshots;
		Fill(snapshots);

		// Act:
		snapshots.pop();

		// Assert:
		EXPECT_FALSE(snapshots.empty());
		EXPECT_TRUE(snapshots.active());
		AssertCurrentImportance(snapshots, Importance(222), model::ImportanceHeight(444));
		AssertHistoricalValues(snapshots, { { std::make_pair(222, 444), std::make_pair(123, 234), std::make_pair(0, 0) } });

		EXPECT_EQ(Importance(), snapshots.get(model::ImportanceHeight(789)));
	}

	TEST(TEST_CLASS, CanPopAllSnapshots) {
		// Arrange:
		AccountImportanceSnapshots snapshots;
		Fill(snapshots);

		// Act:
		for (auto i = 0u; i < Importance_History_Size; ++i)
			snapshots.pop();

		// Assert:
		AssertEmpty(snapshots);
	}

	// endregion

	// region get

	TEST(TEST_CLASS, CannotRetrieveUnknownHistoricalSnapshot) {
		// Arrange:
		AccountImportanceSnapshots snapshots;
		snapshots.set(Importance(123), model::ImportanceHeight(234));

		// Sanity:
		AssertCurrentImportance(snapshots, Importance(123), model::ImportanceHeight(234));

		// Act + Assert:
		EXPECT_EQ(Importance(), snapshots.get(model::ImportanceHeight()));
		EXPECT_EQ(Importance(), snapshots.get(model::ImportanceHeight(233)));
		EXPECT_EQ(Importance(), snapshots.get(model::ImportanceHeight(235)));
	}

	// endregion
}}

