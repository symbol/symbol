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

#include "catapult/state/BalanceHistory.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS BalanceHistoryTests

	// region constructor

	TEST(TEST_CLASS, CanCreateEmpty) {
		// Act:
		BalanceHistory history;

		// Assert:
		EXPECT_EQ(0u, history.size());
		EXPECT_EQ(std::vector<Height>(), history.heights());
		EXPECT_EQ(Amount(0), history.balance());

		EXPECT_EQ(Amount(0), history.balance(Height(10)));
	}

	// endregion

	// region balance(Height)

	TEST(TEST_CLASS, CanGetHistoricalBalanceAtHeight) {
		// Arrange:
		BalanceHistory history;

		// Act:
		history.add(Height(11), Amount(12));
		history.add(Height(22), Amount(98));
		history.add(Height(33), Amount(67));

		// Assert:
		EXPECT_EQ(3u, history.size());

		EXPECT_EQ(Amount(0), history.balance(Height(10)));
		EXPECT_EQ(Amount(12), history.balance(Height(11)));
		EXPECT_EQ(Amount(12), history.balance(Height(12)));

		EXPECT_EQ(Amount(12), history.balance(Height(21)));
		EXPECT_EQ(Amount(98), history.balance(Height(22)));
		EXPECT_EQ(Amount(98), history.balance(Height(23)));

		EXPECT_EQ(Amount(98), history.balance(Height(32)));
		EXPECT_EQ(Amount(67), history.balance(Height(33)));
		EXPECT_EQ(Amount(67), history.balance(Height(34)));

		EXPECT_EQ(Amount(67), history.balance(Height(99)));
	}

	// endregion

	// region anyAtLeast

	TEST(TEST_CLASS, AnyAtLeastReturnsFalseWhenEmpty) {
		// Arrange:
		BalanceHistory history;

		// Act + Assert:
		EXPECT_FALSE(history.anyAtLeast(Amount(0)));
	}

	TEST(TEST_CLASS, AnyAtLeastReturnsCorrectValueWhenNotEmpty) {
		// Arrange:
		BalanceHistory history;
		history.add(Height(11), Amount(12));
		history.add(Height(22), Amount(98));
		history.add(Height(33), Amount(67));

		// Act + Assert:
		EXPECT_TRUE(history.anyAtLeast(Amount(0)));
		EXPECT_TRUE(history.anyAtLeast(Amount(11)));
		EXPECT_TRUE(history.anyAtLeast(Amount(12)));
		EXPECT_TRUE(history.anyAtLeast(Amount(67)));
		EXPECT_TRUE(history.anyAtLeast(Amount(68)));
		EXPECT_TRUE(history.anyAtLeast(Amount(98)));

		EXPECT_FALSE(history.anyAtLeast(Amount(99)));
		EXPECT_FALSE(history.anyAtLeast(Amount(100)));
		EXPECT_FALSE(history.anyAtLeast(Amount(1000)));
		EXPECT_FALSE(history.anyAtLeast(Amount(10000)));
	}

	// endregion

	// region add

	TEST(TEST_CLASS, CanAddSingleBalance) {
		// Arrange:
		BalanceHistory history;

		// Act:
		history.add(Height(11), Amount(22));

		// Assert:
		EXPECT_EQ(1u, history.size());
		EXPECT_EQ(std::vector<Height>({ Height(11) }), history.heights());
		EXPECT_EQ(Amount(22), history.balance());

		EXPECT_EQ(Amount(22), history.balance(Height(11)));
	}

	TEST(TEST_CLASS, CanAddMultipleBalances) {
		// Arrange:
		BalanceHistory history;

		// Act:
		history.add(Height(11), Amount(12));
		history.add(Height(22), Amount(98));
		history.add(Height(33), Amount(67));

		// Assert:
		EXPECT_EQ(3u, history.size());
		EXPECT_EQ(std::vector<Height>({ Height(11), Height(22), Height(33) }), history.heights());
		EXPECT_EQ(Amount(67), history.balance());

		EXPECT_EQ(Amount(12), history.balance(Height(11)));
		EXPECT_EQ(Amount(98), history.balance(Height(22)));
		EXPECT_EQ(Amount(67), history.balance(Height(33)));
	}

	TEST(TEST_CLASS, CanAddMultipleBalances_OutOfOrder) {
		// Arrange:
		BalanceHistory history;

		// Act:
		history.add(Height(11), Amount(12));
		history.add(Height(33), Amount(67));
		history.add(Height(22), Amount(98));

		// Assert:
		EXPECT_EQ(3u, history.size());
		EXPECT_EQ(std::vector<Height>({ Height(11), Height(22), Height(33) }), history.heights());
		EXPECT_EQ(Amount(67), history.balance());

		EXPECT_EQ(Amount(12), history.balance(Height(11)));
		EXPECT_EQ(Amount(98), history.balance(Height(22)));
		EXPECT_EQ(Amount(67), history.balance(Height(33)));
	}

	TEST(TEST_CLASS, CanOverwriteExistingBalance) {
		// Arrange:
		BalanceHistory history;

		// Act:
		history.add(Height(11), Amount(33));
		history.add(Height(11), Amount(22));

		// Assert:
		EXPECT_EQ(1u, history.size());
		EXPECT_EQ(std::vector<Height>({ Height(11) }), history.heights());
		EXPECT_EQ(Amount(22), history.balance());

		EXPECT_EQ(Amount(22), history.balance(Height(11)));
	}

	TEST(TEST_CLASS, CanWriteSameBalanceAtDifferentHeights) {
		// Arrange:
		BalanceHistory history;

		// Act:
		history.add(Height(11), Amount(12));
		history.add(Height(22), Amount(12));
		history.add(Height(33), Amount(12));

		// Assert:
		EXPECT_EQ(1u, history.size());
		EXPECT_EQ(std::vector<Height>({ Height(11) }), history.heights());
		EXPECT_EQ(Amount(12), history.balance());

		EXPECT_EQ(Amount(12), history.balance(Height(11)));
	}

	TEST(TEST_CLASS, CanWriteSameBalanceAtDifferentHeights_OutOfOrder) {
		// Arrange:
		BalanceHistory history;

		// Act:
		history.add(Height(11), Amount(12));
		history.add(Height(33), Amount(24));
		history.add(Height(22), Amount(12));
		history.add(Height(44), Amount(24));

		// Assert:
		EXPECT_EQ(2u, history.size());
		EXPECT_EQ(std::vector<Height>({ Height(11), Height(33) }), history.heights());
		EXPECT_EQ(Amount(24), history.balance());

		EXPECT_EQ(Amount(12), history.balance(Height(11)));
		EXPECT_EQ(Amount(24), history.balance(Height(33)));
	}

	TEST(TEST_CLASS, CanWriteSameBalanceAtDifferentHeights_OverwriteCoalescing) {
		// Arrange:
		BalanceHistory history;
		history.add(Height(11), Amount(12));
		history.add(Height(22), Amount(12));
		history.add(Height(33), Amount(24));
		history.add(Height(44), Amount(12));
		history.add(Height(55), Amount(12));

		// Sanity:
		EXPECT_EQ(3u, history.size());

		// Act:
		history.add(Height(33), Amount(12));

		// Assert:
		EXPECT_EQ(1u, history.size());
		EXPECT_EQ(std::vector<Height>({ Height(11) }), history.heights());
		EXPECT_EQ(Amount(12), history.balance());

		EXPECT_EQ(Amount(12), history.balance(Height(11)));
	}

	// endregion

	// region prune

	namespace {
		void RunPruneTest(Height pruneHeight, const consumer<const BalanceHistory&>& checkHistory) {
			// Arrange:
			BalanceHistory history;
			history.add(Height(11), Amount(12));
			history.add(Height(22), Amount(98));
			history.add(Height(33), Amount(67));

			// Act:
			history.prune(pruneHeight);

			// Assert:
			checkHistory(history);
		}
	}

	TEST(TEST_CLASS, CanPrune_ExactMatch) {
		// Act:
		RunPruneTest(Height(22), [](const auto& history) {
			// Assert:
			EXPECT_EQ(2u, history.size());
			EXPECT_EQ(std::vector<Height>({ Height(22), Height(33) }), history.heights());
			EXPECT_EQ(Amount(67), history.balance());

			EXPECT_EQ(Amount(0), history.balance(Height(11)));
			EXPECT_EQ(Amount(0), history.balance(Height(21)));
			EXPECT_EQ(Amount(98), history.balance(Height(22)));
			EXPECT_EQ(Amount(98), history.balance(Height(23)));
			EXPECT_EQ(Amount(67), history.balance(Height(33)));
		});
	}

	TEST(TEST_CLASS, CanPrune_InexactMatch_OneLess) {
		// Act:
		RunPruneTest(Height(32), [](const auto& history) {
			// Assert:
			EXPECT_EQ(2u, history.size());
			EXPECT_EQ(std::vector<Height>({ Height(32), Height(33) }), history.heights());
			EXPECT_EQ(Amount(67), history.balance());

			EXPECT_EQ(Amount(0), history.balance(Height(11)));
			EXPECT_EQ(Amount(0), history.balance(Height(22)));
			EXPECT_EQ(Amount(0), history.balance(Height(31)));
			EXPECT_EQ(Amount(98), history.balance(Height(32)));
			EXPECT_EQ(Amount(67), history.balance(Height(33)));
		});
	}

	TEST(TEST_CLASS, CanPrune_InexactMatch_OneGreater) {
		// Act:
		RunPruneTest(Height(23), [](const auto& history) {
			// Assert:
			EXPECT_EQ(2u, history.size());
			EXPECT_EQ(std::vector<Height>({ Height(23), Height(33) }), history.heights());
			EXPECT_EQ(Amount(67), history.balance());

			EXPECT_EQ(Amount(0), history.balance(Height(11)));
			EXPECT_EQ(Amount(0), history.balance(Height(22)));
			EXPECT_EQ(Amount(98), history.balance(Height(23)));
			EXPECT_EQ(Amount(98), history.balance(Height(24)));
			EXPECT_EQ(Amount(67), history.balance(Height(33)));
		});
	}

	TEST(TEST_CLASS, CanPrune_InexactMatch_None) {
		// Act:
		RunPruneTest(Height(7), [](const auto& history) {
			// Assert:
			EXPECT_EQ(3u, history.size());
			EXPECT_EQ(std::vector<Height>({ Height(11), Height(22), Height(33) }), history.heights());
			EXPECT_EQ(Amount(67), history.balance());

			EXPECT_EQ(Amount(12), history.balance(Height(11)));
			EXPECT_EQ(Amount(98), history.balance(Height(22)));
			EXPECT_EQ(Amount(67), history.balance(Height(33)));
		});
	}

	TEST(TEST_CLASS, CanPrune_InexactMatch_All) {
		// Act:
		RunPruneTest(Height(98), [](const auto& history) {
			// Assert:
			EXPECT_EQ(1u, history.size());
			EXPECT_EQ(std::vector<Height>({ Height(98) }), history.heights());
			EXPECT_EQ(Amount(67), history.balance());

			EXPECT_EQ(Amount(0), history.balance(Height(97)));
			EXPECT_EQ(Amount(67), history.balance(Height(98)));
			EXPECT_EQ(Amount(67), history.balance(Height(99)));
		});
	}

	// endregion
}}
