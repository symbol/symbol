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

#include "catapult/state/HeightIndexedHistoryMap.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS HeightIndexedHistoryMapTests

	using HistoryMap = HeightIndexedHistoryMap<Timestamp>;

	// region constructor

	TEST(TEST_CLASS, CanCreateEmpty) {
		// Act:
		HistoryMap history;

		// Assert:
		EXPECT_EQ(0u, history.size());
		EXPECT_EQ(std::vector<Height>(), history.heights());
		EXPECT_EQ(Timestamp(0), history.get());

		EXPECT_EQ(Timestamp(0), history.get(Height(10)));
	}

	// endregion

	// region get(Height)

	TEST(TEST_CLASS, CanGetHistoricalValueAtHeight) {
		// Arrange:
		HistoryMap history;

		// Act:
		history.add(Height(11), Timestamp(12));
		history.add(Height(22), Timestamp(98));
		history.add(Height(33), Timestamp(67));

		// Assert:
		EXPECT_EQ(3u, history.size());

		EXPECT_EQ(Timestamp(0), history.get(Height(10)));
		EXPECT_EQ(Timestamp(12), history.get(Height(11)));
		EXPECT_EQ(Timestamp(12), history.get(Height(12)));

		EXPECT_EQ(Timestamp(12), history.get(Height(21)));
		EXPECT_EQ(Timestamp(98), history.get(Height(22)));
		EXPECT_EQ(Timestamp(98), history.get(Height(23)));

		EXPECT_EQ(Timestamp(98), history.get(Height(32)));
		EXPECT_EQ(Timestamp(67), history.get(Height(33)));
		EXPECT_EQ(Timestamp(67), history.get(Height(34)));

		EXPECT_EQ(Timestamp(67), history.get(Height(99)));
	}

	// endregion

	// region anyOf

	TEST(TEST_CLASS, AnyOfReturnsFalseWhenEmpty) {
		// Arrange:
		HistoryMap history;

		// Act:
		auto result = history.anyOf([](auto) {
			return true;
		});

		// Assert:
		EXPECT_FALSE(result);
	}

	TEST(TEST_CLASS, AnyOfVisitsAllValuesWhenPredicateReturnsFalse) {
		// Arrange:
		HistoryMap history;
		history.add(Height(11), Timestamp(12));
		history.add(Height(22), Timestamp(98));
		history.add(Height(33), Timestamp(67));

		// Act:
		std::vector<Timestamp> values;
		auto result = history.anyOf([&values](auto value) {
			values.push_back(value);
			return false;
		});

		// Assert:
		EXPECT_FALSE(result);
		EXPECT_EQ(std::vector<Timestamp>({ Timestamp(67), Timestamp(98), Timestamp(12) }), values);
	}

	TEST(TEST_CLASS, AnyOfShortCircuitsWhenPredicateReturnsTrue) {
		// Arrange:
		HistoryMap history;
		history.add(Height(11), Timestamp(12));
		history.add(Height(22), Timestamp(98));
		history.add(Height(33), Timestamp(67));

		// Act:
		std::vector<Timestamp> values;
		auto result = history.anyOf([&values](auto value) {
			values.push_back(value);
			return true;
		});

		// Assert:
		EXPECT_TRUE(result);
		EXPECT_EQ(std::vector<Timestamp>({ Timestamp(67) }), values);
	}

	// endregion

	// region add

	TEST(TEST_CLASS, CanAddSingleValue) {
		// Arrange:
		HistoryMap history;

		// Act:
		history.add(Height(11), Timestamp(22));

		// Assert:
		EXPECT_EQ(1u, history.size());
		EXPECT_EQ(std::vector<Height>({ Height(11) }), history.heights());
		EXPECT_EQ(Timestamp(22), history.get());

		EXPECT_EQ(Timestamp(22), history.get(Height(11)));
	}

	TEST(TEST_CLASS, CanAddSingleZeroValue) {
		// Arrange:
		HistoryMap history;

		// Act:
		history.add(Height(11), Timestamp(0));

		// Assert: due to coalescing, no value was actually added
		EXPECT_EQ(0u, history.size());
		EXPECT_EQ(std::vector<Height>(), history.heights());
		EXPECT_EQ(Timestamp(0), history.get());

		EXPECT_EQ(Timestamp(0), history.get(Height(11)));
	}

	TEST(TEST_CLASS, CanAddMultipleValues) {
		// Arrange:
		HistoryMap history;

		// Act:
		history.add(Height(11), Timestamp(12));
		history.add(Height(22), Timestamp(98));
		history.add(Height(33), Timestamp(67));

		// Assert:
		EXPECT_EQ(3u, history.size());
		EXPECT_EQ(std::vector<Height>({ Height(11), Height(22), Height(33) }), history.heights());
		EXPECT_EQ(Timestamp(67), history.get());

		EXPECT_EQ(Timestamp(12), history.get(Height(11)));
		EXPECT_EQ(Timestamp(98), history.get(Height(22)));
		EXPECT_EQ(Timestamp(67), history.get(Height(33)));
	}

	TEST(TEST_CLASS, CanAddMultipleValues_HeightsOutOfOrder) {
		// Arrange:
		HistoryMap history;

		// Act:
		history.add(Height(11), Timestamp(12));
		history.add(Height(33), Timestamp(67));
		history.add(Height(22), Timestamp(98));

		// Assert:
		EXPECT_EQ(3u, history.size());
		EXPECT_EQ(std::vector<Height>({ Height(11), Height(22), Height(33) }), history.heights());
		EXPECT_EQ(Timestamp(67), history.get());

		EXPECT_EQ(Timestamp(12), history.get(Height(11)));
		EXPECT_EQ(Timestamp(98), history.get(Height(22)));
		EXPECT_EQ(Timestamp(67), history.get(Height(33)));
	}

	TEST(TEST_CLASS, CanOverwriteExistingValue) {
		// Arrange:
		HistoryMap history;

		// Act:
		history.add(Height(11), Timestamp(33));
		history.add(Height(11), Timestamp(22));

		// Assert:
		EXPECT_EQ(1u, history.size());
		EXPECT_EQ(std::vector<Height>({ Height(11) }), history.heights());
		EXPECT_EQ(Timestamp(22), history.get());

		EXPECT_EQ(Timestamp(22), history.get(Height(11)));
	}

	TEST(TEST_CLASS, CanWriteSameValueAtDifferentHeights) {
		// Arrange:
		HistoryMap history;

		// Act:
		history.add(Height(11), Timestamp(12));
		history.add(Height(22), Timestamp(12));
		history.add(Height(33), Timestamp(12));

		// Assert:
		EXPECT_EQ(1u, history.size());
		EXPECT_EQ(std::vector<Height>({ Height(11) }), history.heights());
		EXPECT_EQ(Timestamp(12), history.get());

		EXPECT_EQ(Timestamp(12), history.get(Height(11)));
	}

	TEST(TEST_CLASS, CanWriteSameValueAtDifferentHeights_HeightsOutOfOrder) {
		// Arrange:
		HistoryMap history;

		// Act:
		history.add(Height(11), Timestamp(12));
		history.add(Height(33), Timestamp(24));
		history.add(Height(22), Timestamp(12));
		history.add(Height(44), Timestamp(24));

		// Assert:
		EXPECT_EQ(2u, history.size());
		EXPECT_EQ(std::vector<Height>({ Height(11), Height(33) }), history.heights());
		EXPECT_EQ(Timestamp(24), history.get());

		EXPECT_EQ(Timestamp(12), history.get(Height(11)));
		EXPECT_EQ(Timestamp(24), history.get(Height(33)));
	}

	TEST(TEST_CLASS, CanWriteSameValueAtDifferentHeights_OverwriteCoalescing) {
		// Arrange:
		HistoryMap history;
		history.add(Height(11), Timestamp(12));
		history.add(Height(22), Timestamp(12));
		history.add(Height(33), Timestamp(24));
		history.add(Height(44), Timestamp(12));
		history.add(Height(55), Timestamp(12));

		// Sanity:
		EXPECT_EQ(3u, history.size());

		// Act:
		history.add(Height(33), Timestamp(12));

		// Assert:
		EXPECT_EQ(1u, history.size());
		EXPECT_EQ(std::vector<Height>({ Height(11) }), history.heights());
		EXPECT_EQ(Timestamp(12), history.get());

		EXPECT_EQ(Timestamp(12), history.get(Height(11)));
	}

	// endregion

	// region prune

	namespace {
		void RunPruneTest(Height pruneHeight, const consumer<const HistoryMap&>& checkHistory) {
			// Arrange:
			HistoryMap history;
			history.add(Height(11), Timestamp(12));
			history.add(Height(22), Timestamp(98));
			history.add(Height(33), Timestamp(67));

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
			EXPECT_EQ(Timestamp(67), history.get());

			EXPECT_EQ(Timestamp(0), history.get(Height(11)));
			EXPECT_EQ(Timestamp(0), history.get(Height(21)));
			EXPECT_EQ(Timestamp(98), history.get(Height(22)));
			EXPECT_EQ(Timestamp(98), history.get(Height(23)));
			EXPECT_EQ(Timestamp(67), history.get(Height(33)));
		});
	}

	TEST(TEST_CLASS, CanPrune_InexactMatch_OneLess) {
		// Act:
		RunPruneTest(Height(32), [](const auto& history) {
			// Assert:
			EXPECT_EQ(2u, history.size());
			EXPECT_EQ(std::vector<Height>({ Height(32), Height(33) }), history.heights());
			EXPECT_EQ(Timestamp(67), history.get());

			EXPECT_EQ(Timestamp(0), history.get(Height(11)));
			EXPECT_EQ(Timestamp(0), history.get(Height(22)));
			EXPECT_EQ(Timestamp(0), history.get(Height(31)));
			EXPECT_EQ(Timestamp(98), history.get(Height(32)));
			EXPECT_EQ(Timestamp(67), history.get(Height(33)));
		});
	}

	TEST(TEST_CLASS, CanPrune_InexactMatch_OneGreater) {
		// Act:
		RunPruneTest(Height(23), [](const auto& history) {
			// Assert:
			EXPECT_EQ(2u, history.size());
			EXPECT_EQ(std::vector<Height>({ Height(23), Height(33) }), history.heights());
			EXPECT_EQ(Timestamp(67), history.get());

			EXPECT_EQ(Timestamp(0), history.get(Height(11)));
			EXPECT_EQ(Timestamp(0), history.get(Height(22)));
			EXPECT_EQ(Timestamp(98), history.get(Height(23)));
			EXPECT_EQ(Timestamp(98), history.get(Height(24)));
			EXPECT_EQ(Timestamp(67), history.get(Height(33)));
		});
	}

	TEST(TEST_CLASS, CanPrune_InexactMatch_None) {
		// Act:
		RunPruneTest(Height(7), [](const auto& history) {
			// Assert:
			EXPECT_EQ(3u, history.size());
			EXPECT_EQ(std::vector<Height>({ Height(11), Height(22), Height(33) }), history.heights());
			EXPECT_EQ(Timestamp(67), history.get());

			EXPECT_EQ(Timestamp(12), history.get(Height(11)));
			EXPECT_EQ(Timestamp(98), history.get(Height(22)));
			EXPECT_EQ(Timestamp(67), history.get(Height(33)));
		});
	}

	TEST(TEST_CLASS, CanPrune_InexactMatch_All) {
		// Act:
		RunPruneTest(Height(98), [](const auto& history) {
			// Assert:
			EXPECT_EQ(1u, history.size());
			EXPECT_EQ(std::vector<Height>({ Height(98) }), history.heights());
			EXPECT_EQ(Timestamp(67), history.get());

			EXPECT_EQ(Timestamp(0), history.get(Height(97)));
			EXPECT_EQ(Timestamp(67), history.get(Height(98)));
			EXPECT_EQ(Timestamp(67), history.get(Height(99)));
		});
	}

	// endregion
}}
