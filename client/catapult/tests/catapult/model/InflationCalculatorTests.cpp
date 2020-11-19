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

#include "catapult/model/InflationCalculator.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS InflationCalculatorTests

	namespace {
		struct InflationEntry {
			catapult::Height Height;
			catapult::Amount Amount;
		};

		InflationCalculator CreateInflationCalculator(const std::vector<InflationEntry>& inflationEntries) {
			InflationCalculator calculator;
			for (const auto& inflationEntry : inflationEntries)
				calculator.add(inflationEntry.Height, inflationEntry.Amount);

			// Sanity:
			EXPECT_EQ(inflationEntries.size(), calculator.size());

			return calculator;
		}
	}

	// region ctor

	TEST(TEST_CLASS, InflationCalculatorIsInitiallyEmpty) {
		// Act:
		InflationCalculator calculator;

		// Assert:
		EXPECT_EQ(0u, calculator.size());
	}

	// endregion

	// region add

	TEST(TEST_CLASS, CanAddSingleInflationEntry) {
		// Arrange:
		InflationCalculator calculator;

		// Act:
		calculator.add(Height(5), Amount(123));

		// Assert:
		EXPECT_EQ(1u, calculator.size());
		EXPECT_TRUE(calculator.contains(Height(5), Amount(123)));
	}

	TEST(TEST_CLASS, CanAddMultipleInflationEntries) {
		// Arrange:
		InflationCalculator calculator;

		// Act:
		calculator.add(Height(5), Amount(345));
		calculator.add(Height(15), Amount(234));
		calculator.add(Height(25), Amount(123));

		// Assert:
		EXPECT_EQ(3u, calculator.size());
		EXPECT_TRUE(calculator.contains(Height(5), Amount(345)));
		EXPECT_TRUE(calculator.contains(Height(15), Amount(234)));
		EXPECT_TRUE(calculator.contains(Height(25), Amount(123)));
	}

	TEST(TEST_CLASS, CannotAddInflationEntryAtHeightZero) {
		// Arrange:
		InflationCalculator calculator;

		// Act + Assert:
		EXPECT_THROW(calculator.add(Height(0), Amount(123)), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CannotAddInflationEntryWhenHeightIsEqualToLastEntryHeight) {
		// Arrange:
		auto calculator = CreateInflationCalculator({ { Height(5), Amount(345) }, { Height(15), Amount(123) } });

		// Assert:
		EXPECT_THROW(calculator.add(Height(15), Amount(234)), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CannotAddInflationEntryWhenHeightIsLessThanLastEntryHeight) {
		// Arrange:
		auto calculator = CreateInflationCalculator({ { Height(5), Amount(345) }, { Height(15), Amount(123) } });

		// Assert:
		EXPECT_THROW(calculator.add(Height(5), Amount(456)), catapult_invalid_argument);
		EXPECT_THROW(calculator.add(Height(10), Amount(567)), catapult_invalid_argument);
		EXPECT_THROW(calculator.add(Height(14), Amount(678)), catapult_invalid_argument);
	}

	// endregion

	// region getSpotAmount

	namespace {
		constexpr const char* Height_Message = "at height ";
	}

	TEST(TEST_CLASS, GetSpotAmountReturnsZeroWhenMapIsEmpty) {
		// Arrange:
		InflationCalculator calculator;

		// Act + Assert:
		for (auto rawHeight : { 0u, 1u, 10u, 123456u })
			EXPECT_EQ(Amount(), calculator.getSpotAmount(Height(rawHeight))) << Height_Message << rawHeight;
	}

	TEST(TEST_CLASS, GetSpotAmountReturnsExpectedAmount_HeightExistsInMap) {
		// Arrange:
		std::vector<InflationEntry> entries{ { Height(1), Amount(345) }, { Height(15), Amount(234) }, { Height(25), Amount(123) } };
		auto calculator = CreateInflationCalculator(entries);

		// Act + Assert:
		EXPECT_EQ(Amount(345), calculator.getSpotAmount(Height(1)));
		EXPECT_EQ(Amount(234), calculator.getSpotAmount(Height(15)));
		EXPECT_EQ(Amount(123), calculator.getSpotAmount(Height(25)));
	}

	TEST(TEST_CLASS, GetSpotAmountReturnsExpectedAmount_HeightDoesNotExistInMap) {
		// Arrange:
		std::vector<InflationEntry> entries{ { Height(5), Amount(345) }, { Height(15), Amount(234) }, { Height(25), Amount(123) } };
		auto calculator = CreateInflationCalculator(entries);

		// Act + Assert:
		EXPECT_EQ(Amount(), calculator.getSpotAmount(Height(3))); // before first entry
		EXPECT_EQ(Amount(345), calculator.getSpotAmount(Height(6))); // between first and second entry
		EXPECT_EQ(Amount(234), calculator.getSpotAmount(Height(18))); // between second and third entry
		EXPECT_EQ(Amount(123), calculator.getSpotAmount(Height(35))); // after third entry
	}

	// endregion

	// region getCumulativeAmount

	TEST(TEST_CLASS, GetCumulativeAmountReturnsZeroWhenMapIsEmpty) {
		// Arrange:
		InflationCalculator calculator;

		// Act + Assert:
		for (auto rawHeight : { 1u, 5u, 10u, 123456u })
			EXPECT_EQ(Amount(), calculator.getCumulativeAmount(Height(rawHeight))) << Height_Message << rawHeight;
	}

	TEST(TEST_CLASS, GetCumulativeAmountReturnsAmountZeroAtHeightZero) {
		// Arrange:
		std::vector<InflationEntry> entries{ { Height(1), Amount(345) }, { Height(15), Amount(234) }, { Height(25), Amount(123) } };
		auto calculator = CreateInflationCalculator(entries);

		// Act + Assert:
		EXPECT_EQ(Amount(), calculator.getCumulativeAmount(Height(0)));
	}

	TEST(TEST_CLASS, GetCumulativeAmountReturnsExpectedAmount_HeightExistsInMap) {
		// Arrange:
		std::vector<InflationEntry> entries{ { Height(5), Amount(345) }, { Height(15), Amount(86) }, { Height(25), Amount(123) } };
		auto calculator = CreateInflationCalculator(entries);

		// Act + Assert: total inflation does not include the height provided in the call to getTotalAmount
		EXPECT_EQ(Amount(), calculator.getCumulativeAmount(Height(5))); // always zero up to first entry;
		EXPECT_EQ(Amount(10 * 345), calculator.getCumulativeAmount(Height(15))); // 10 heights with inflation of 345
		EXPECT_EQ(Amount(10 * 345 + 10 * 86), calculator.getCumulativeAmount(Height(25))); // additionally 10 heights with inflation of 86
	}

	TEST(TEST_CLASS, GetCumulativeAmountReturnsExpectedAmount_HeightDoesNotExistInMap) {
		// Arrange:
		std::vector<InflationEntry> entries{ { Height(5), Amount(345) }, { Height(15), Amount(234) }, { Height(25), Amount(123) } };
		auto calculator = CreateInflationCalculator(entries);

		// Act + Assert: total inflation does not include the height provided in the call to getTotalAmount
		EXPECT_EQ(Amount(), calculator.getCumulativeAmount(Height(1)));
		EXPECT_EQ(Amount(1 * 345), calculator.getCumulativeAmount(Height(6)));
		EXPECT_EQ(Amount(9 * 345), calculator.getCumulativeAmount(Height(14)));
		EXPECT_EQ(Amount(10 * 345 + 1 * 234), calculator.getCumulativeAmount(Height(16)));
		EXPECT_EQ(Amount(10 * 345 + 9 * 234), calculator.getCumulativeAmount(Height(24)));
		EXPECT_EQ(Amount(10 * 345 + 10 * 234 + 1 * 123), calculator.getCumulativeAmount(Height(26))); // inflation of 123 from height 25 on
		EXPECT_EQ(Amount(10 * 345 + 10 * 234 + 75 * 123), calculator.getCumulativeAmount(Height(100)));
	}

	// endregion

	// region sumAll

	TEST(TEST_CLASS, SumAllReturnsTotalInflationWhenNotEncounteringOverflow) {
		// Arrange:
		auto calculator = CreateInflationCalculator({ { Height(3), Amount(345) }, { Height(9), Amount(234) }, { Height(45), Amount(0) } });

		// Act:
		auto totalInflation = calculator.sumAll();

		// Assert:
		EXPECT_TRUE(totalInflation.second);
		EXPECT_EQ(Amount(6 * 345 + 36 * 234), totalInflation.first);
	}

	TEST(TEST_CLASS, SumAllReturnsFalseWhenInflationSummandCausesOverflow) {
		// Arrange:
		auto numBlocks = std::numeric_limits<uint64_t>::max() / 2 + 2;
		auto calculator = CreateInflationCalculator({ { Height(1), Amount(2) }, { Height(numBlocks), Amount(0) } });

		// Act:
		auto totalInflation = calculator.sumAll();

		// Act + Assert:
		EXPECT_FALSE(totalInflation.second);
		EXPECT_EQ(Amount(), totalInflation.first);
	}

	TEST(TEST_CLASS, SumAllReturnsFalseWhenCurrentTotalInflationPlusSummandCausesOverflow) {
		// Arrange:
		auto numBlocks = std::numeric_limits<uint64_t>::max() / 2;
		auto calculator = CreateInflationCalculator({
			{ Height(1), Amount(2) },
			{ Height(numBlocks), Amount(2) },
			{ Height(numBlocks + 2), Amount(0) }
		});

		// Act:
		auto totalInflation = calculator.sumAll();

		// Act + Assert:
		EXPECT_FALSE(totalInflation.second);
		EXPECT_EQ(Amount(), totalInflation.first);
	}

	TEST(TEST_CLASS, SumAllReturnsFalseWhenLastInflationEntryIsNonzero) {
		// Arrange:
		std::vector<InflationEntry> entries{ { Height(5), Amount(345) }, { Height(15), Amount(234) }, { Height(25), Amount(3) } };
		auto calculator = CreateInflationCalculator(entries);

		// Act:
		auto totalInflation = calculator.sumAll();

		// Assert:
		EXPECT_FALSE(totalInflation.second);
		EXPECT_EQ(Amount(), totalInflation.first);
	}

	// endregion
}}
