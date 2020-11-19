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

#include "catapult/model/TransactionChangeTracker.h"
#include "catapult/utils/ArraySet.h"
#include "tests/test/core/TransactionInfoTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS TransactionChangeTrackerTests

	namespace {
		// unlike test::ExtractHashes, this function extracts hashes into a HashSet
		template<typename TTransactionInfos>
		utils::HashSet ExtractHashes(const TTransactionInfos& transactionInfos) {
			utils::HashSet hashes;
			for (const auto& transactionInfo : transactionInfos)
				hashes.emplace(transactionInfo.EntityHash);

			return hashes;
		}
	}

	TEST(TEST_CLASS, TrackerInitiallyHasNoChanges) {
		// Act:
		TransactionChangeTracker tracker;

		// Assert:
		EXPECT_TRUE(tracker.addedTransactionInfos().empty());
		EXPECT_TRUE(tracker.removedTransactionInfos().empty());
	}

	// region add

	namespace {
		void AssertCanAddTransactionInfos(size_t count) {
			// Arrange:
			TransactionChangeTracker tracker;
			auto transactionInfos = test::CreateTransactionInfos(count);

			// Act:
			for (const auto& transactionInfo : transactionInfos)
				tracker.add(transactionInfo);

			// Assert:
			EXPECT_EQ(count, tracker.addedTransactionInfos().size());
			EXPECT_EQ(ExtractHashes(transactionInfos), ExtractHashes(tracker.addedTransactionInfos()));

			EXPECT_TRUE(tracker.removedTransactionInfos().empty());
		}
	}

	TEST(TEST_CLASS, CanAddSingleTransaction) {
		AssertCanAddTransactionInfos(1);
	}

	TEST(TEST_CLASS, CanAddMultipleTransactions) {
		AssertCanAddTransactionInfos(3);
	}

	TEST(TEST_CLASS, RedundantAddsAreCollapsed) {
		// Arrange:
		TransactionChangeTracker tracker;
		auto transactionInfos = test::CreateTransactionInfos(3);

		// Act: add all infos twice
		for (const auto& transactionInfo : transactionInfos)
			tracker.add(transactionInfo);

		for (const auto& transactionInfo : transactionInfos)
			tracker.add(transactionInfo);

		// Assert: each info is only reported once
		EXPECT_EQ(3u, tracker.addedTransactionInfos().size());
		EXPECT_EQ(ExtractHashes(transactionInfos), ExtractHashes(tracker.addedTransactionInfos()));

		EXPECT_TRUE(tracker.removedTransactionInfos().empty());
	}

	// endregion

	// region remove

	namespace {
		void AssertCanRemoveTransactionInfos(size_t count) {
			// Arrange:
			TransactionChangeTracker tracker;
			auto transactionInfos = test::CreateTransactionInfos(count);

			// Act:
			for (const auto& transactionInfo : transactionInfos)
				tracker.remove(transactionInfo);

			// Assert:
			EXPECT_TRUE(tracker.addedTransactionInfos().empty());

			EXPECT_EQ(count, tracker.removedTransactionInfos().size());
			EXPECT_EQ(ExtractHashes(transactionInfos), ExtractHashes(tracker.removedTransactionInfos()));
		}
	}

	TEST(TEST_CLASS, CanRemoveSingleTransaction) {
		AssertCanRemoveTransactionInfos(1);
	}

	TEST(TEST_CLASS, CanRemoveMultipleTransactions) {
		AssertCanRemoveTransactionInfos(3);
	}

	TEST(TEST_CLASS, RedundantRemovesAreCollapsed) {
		// Arrange:
		TransactionChangeTracker tracker;
		auto transactionInfos = test::CreateTransactionInfos(3);

		// Act: remove all infos twice
		for (const auto& transactionInfo : transactionInfos)
			tracker.remove(transactionInfo);

		for (const auto& transactionInfo : transactionInfos)
			tracker.remove(transactionInfo);

		// Assert: each info is only reported once
		EXPECT_TRUE(tracker.addedTransactionInfos().empty());

		EXPECT_EQ(3u, tracker.removedTransactionInfos().size());
		EXPECT_EQ(ExtractHashes(transactionInfos), ExtractHashes(tracker.removedTransactionInfos()));
	}

	// endregion

	// region add + remove

	namespace {
		void SeedTracker(
				TransactionChangeTracker& tracker,
				const std::vector<TransactionInfo>& addedInfos,
				const std::vector<TransactionInfo>& removedInfos) {
			for (const auto& info : addedInfos)
				tracker.add(info);

			for (const auto& info : removedInfos)
				tracker.remove(info);
		}
	}

	TEST(TEST_CLASS, CanAddAndRemoveMultipleTransactions) {
		// Arrange:
		TransactionChangeTracker tracker;
		auto addedInfos = test::CreateTransactionInfos(4);
		auto removedInfos = test::CreateTransactionInfos(3);

		// Act:
		SeedTracker(tracker, addedInfos, removedInfos);

		// Assert:
		EXPECT_EQ(4u, tracker.addedTransactionInfos().size());
		EXPECT_EQ(ExtractHashes(addedInfos), ExtractHashes(tracker.addedTransactionInfos()));

		EXPECT_EQ(3u, tracker.removedTransactionInfos().size());
		EXPECT_EQ(ExtractHashes(removedInfos), ExtractHashes(tracker.removedTransactionInfos()));
	}

	TEST(TEST_CLASS, CanAddRemovedTransaction) {
		// Arrange:
		TransactionChangeTracker tracker;
		auto addedInfos = test::CreateTransactionInfos(4);
		auto removedInfos = test::CreateTransactionInfos(3);
		SeedTracker(tracker, addedInfos, removedInfos);

		// Act: add a transaction that was removed
		tracker.add(removedInfos[1]);

		// Assert:
		EXPECT_EQ(4u, tracker.addedTransactionInfos().size());
		EXPECT_EQ(ExtractHashes(addedInfos), ExtractHashes(tracker.addedTransactionInfos()));

		EXPECT_EQ(2u, tracker.removedTransactionInfos().size());
		removedInfos.erase(removedInfos.begin() + 1); // remove from the original before extracting hashes
		EXPECT_EQ(ExtractHashes(removedInfos), ExtractHashes(tracker.removedTransactionInfos()));
	}

	TEST(TEST_CLASS, CanRemoveAddedTransaction) {
		// Arrange:
		TransactionChangeTracker tracker;
		auto addedInfos = test::CreateTransactionInfos(4);
		auto removedInfos = test::CreateTransactionInfos(3);
		SeedTracker(tracker, addedInfos, removedInfos);

		// Act: remove a transaction that was added
		tracker.remove(addedInfos[2]);

		// Assert:
		EXPECT_EQ(3u, tracker.addedTransactionInfos().size());
		addedInfos.erase(addedInfos.begin() + 2); // remove from the original before extracting hashes
		EXPECT_EQ(ExtractHashes(addedInfos), ExtractHashes(tracker.addedTransactionInfos()));

		EXPECT_EQ(3u, tracker.removedTransactionInfos().size());
		EXPECT_EQ(ExtractHashes(removedInfos), ExtractHashes(tracker.removedTransactionInfos()));
	}

	// endregion

	// region reset

	TEST(TEST_CLASS, ResetClearsTrackedChanges) {
		// Arrange:
		TransactionChangeTracker tracker;
		auto addedInfos = test::CreateTransactionInfos(4);
		auto removedInfos = test::CreateTransactionInfos(3);
		SeedTracker(tracker, addedInfos, removedInfos);

		// Sanity:
		EXPECT_EQ(4u, tracker.addedTransactionInfos().size());
		EXPECT_EQ(3u, tracker.removedTransactionInfos().size());

		// Act:
		tracker.reset();

		// Assert:
		EXPECT_TRUE(tracker.addedTransactionInfos().empty());
		EXPECT_TRUE(tracker.removedTransactionInfos().empty());
	}

	// endregion
}}
