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

#include "catapult/state/AccountHistory.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS AccountHistoryTests

	// region constructor

	TEST(TEST_CLASS, CanCreateEmpty) {
		// Act:
		AccountHistory history;

		// Assert:
		EXPECT_EQ(0u, history.balances().size());
	}

	// endregion

	// region anyAtLeast

	TEST(TEST_CLASS, AnyAtLeastReturnsFalseWhenEmpty) {
		// Arrange:
		AccountHistory history;

		// Act + Assert:
		EXPECT_FALSE(history.anyAtLeast(Amount(0)));
	}

	TEST(TEST_CLASS, AnyAtLeastReturnsCorrectValueWhenNotEmpty) {
		// Arrange:
		AccountHistory history;
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
		AccountHistory history;
		const auto& balanceHistory = history.balances();

		// Act:
		history.add(Height(11), Amount(22));

		// Assert:
		EXPECT_EQ(1u, balanceHistory.size());
		EXPECT_EQ(std::vector<Height>({ Height(11) }), balanceHistory.heights());
		EXPECT_EQ(Amount(22), balanceHistory.get());

		EXPECT_EQ(Amount(22), balanceHistory.get(Height(11)));
	}

	TEST(TEST_CLASS, CanAddMultipleBalances) {
		// Arrange:
		AccountHistory history;
		const auto& balanceHistory = history.balances();

		// Act:
		history.add(Height(11), Amount(12));
		history.add(Height(22), Amount(98));
		history.add(Height(33), Amount(67));

		// Assert:
		EXPECT_EQ(3u, balanceHistory.size());
		EXPECT_EQ(std::vector<Height>({ Height(11), Height(22), Height(33) }), balanceHistory.heights());
		EXPECT_EQ(Amount(67), balanceHistory.get());

		EXPECT_EQ(Amount(12), balanceHistory.get(Height(11)));
		EXPECT_EQ(Amount(98), balanceHistory.get(Height(22)));
		EXPECT_EQ(Amount(67), balanceHistory.get(Height(33)));
	}

	// endregion

	// region prune

	namespace {
		void RunPruneTest(Height pruneHeight, const consumer<const AccountHistory&>& checkHistory) {
			// Arrange:
			AccountHistory history;
			history.add(Height(11), Amount(12));
			history.add(Height(22), Amount(98));
			history.add(Height(33), Amount(67));

			// Act:
			history.prune(pruneHeight);

			// Assert:
			checkHistory(history);
		}
	}

	TEST(TEST_CLASS, CanPrune) {
		// Act:
		RunPruneTest(Height(22), [](const auto& history) {
			const auto& balanceHistory = history.balances();

			// Assert:
			EXPECT_EQ(2u, balanceHistory.size());
			EXPECT_EQ(std::vector<Height>({ Height(22), Height(33) }), balanceHistory.heights());
			EXPECT_EQ(Amount(67), balanceHistory.get());

			EXPECT_EQ(Amount(0), balanceHistory.get(Height(11)));
			EXPECT_EQ(Amount(0), balanceHistory.get(Height(21)));
			EXPECT_EQ(Amount(98), balanceHistory.get(Height(22)));
			EXPECT_EQ(Amount(98), balanceHistory.get(Height(23)));
			EXPECT_EQ(Amount(67), balanceHistory.get(Height(33)));
		});
	}

	// endregion
}}
