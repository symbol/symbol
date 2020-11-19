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

#include "catapult/state/DynamicFeeMultiplier.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS DynamicFeeMultiplierTests

	namespace {
		std::vector<BlockStatistic> CreateStatistics(std::initializer_list<uint32_t> multipliers) {
			std::vector<BlockStatistic> statistics;
			for (auto multiplier : multipliers)
				statistics.emplace_back(Height(), Timestamp(), Difficulty(), BlockFeeMultiplier(multiplier));

			return statistics;
		}
	}

	// region no candidates

	TEST(TEST_CLASS, DefaultFeeMultiplierIsReturnedWhenCountIsZero) {
		// Arrange:
		auto statistics = CreateStatistics({});

		// Act:
		auto multiplier = CalculateDynamicFeeMultiplier(statistics.cbegin(), statistics.cend(), 0, BlockFeeMultiplier(123));

		// Assert:
		EXPECT_EQ(BlockFeeMultiplier(123), multiplier);
	}

	TEST(TEST_CLASS, DefaultFeeMultiplierIsReturnedWhenRangeIsEmpty) {
		// Arrange:
		auto statistics = CreateStatistics({});

		// Act:
		auto multiplier = CalculateDynamicFeeMultiplier(statistics.cbegin(), statistics.cend(), 5, BlockFeeMultiplier(123));

		// Assert:
		EXPECT_EQ(BlockFeeMultiplier(123), multiplier);
	}

	TEST(TEST_CLASS, DefaultFeeMultiplierIsReturnedWhenRangeContainsNoCandidates) {
		// Arrange:
		auto statistics = CreateStatistics({ 0, 0, 0, 0, 0 });

		// Act:
		auto multiplier = CalculateDynamicFeeMultiplier(statistics.cbegin(), statistics.cend(), 5, BlockFeeMultiplier(123));

		// Assert:
		EXPECT_EQ(BlockFeeMultiplier(123), multiplier);
	}

	// endregion

	// region candidates

	namespace {
		void AssertCanCalculateMultiplierWhenCandidatesLessThanCount(std::initializer_list<uint32_t> multipliers) {
			// Arrange:
			auto statistics = CreateStatistics(multipliers);

			// Act:
			auto multiplier4 = CalculateDynamicFeeMultiplier(statistics.cbegin(), statistics.cend(), 4, BlockFeeMultiplier(123));
			auto multiplier5 = CalculateDynamicFeeMultiplier(statistics.cbegin(), statistics.cend(), 5, BlockFeeMultiplier(123));
			auto multiplier6 = CalculateDynamicFeeMultiplier(statistics.cbegin(), statistics.cend(), 6, BlockFeeMultiplier(123));

			// Assert:
			EXPECT_EQ(BlockFeeMultiplier(500), multiplier4) << "7, 123, <500>, 1000";
			EXPECT_EQ(BlockFeeMultiplier(123), multiplier5) << "7, 123, <123>, 500, 1000";
			EXPECT_EQ(BlockFeeMultiplier(123), multiplier6) << "7, 123, 123, <123>, 500, 1000";
		}
	}

	TEST(TEST_CLASS, CanCalculateMultiplierWhenCandidatesLessThanCount) {
		AssertCanCalculateMultiplierWhenCandidatesLessThanCount({ 500, 7, 1000 });
	}

	TEST(TEST_CLASS, CanCalculateMultiplierWhenCandidatesLessThanCountInterspersedWithNonCandidates) {
		AssertCanCalculateMultiplierWhenCandidatesLessThanCount({ 500, 0, 7, 0, 1000, 0 });
	}

	TEST(TEST_CLASS, CanCalculateMultiplierWhenCandidatesEqualToCount) {
		// Arrange:
		auto statistics5 = CreateStatistics({ 12, 7, 9, 11, 15 });
		auto statistics6 = CreateStatistics({ 12, 7, 9, 11, 15, 17 });

		// Act:
		auto multiplier5 = CalculateDynamicFeeMultiplier(statistics5.cbegin(), statistics5.cend(), 5, BlockFeeMultiplier(123));
		auto multiplier6 = CalculateDynamicFeeMultiplier(statistics6.cbegin(), statistics6.cend(), 6, BlockFeeMultiplier(123));

		// Assert:
		EXPECT_EQ(BlockFeeMultiplier(11), multiplier5) << "7, 9, <11>, 12, 15";
		EXPECT_EQ(BlockFeeMultiplier(12), multiplier6) << "7, 9, 11, <12>, 15, 17";
	}

	TEST(TEST_CLASS, CanCalculateMultiplierWhenCandidatesGreaterThanCount) {
		// Arrange:
		auto statistics = CreateStatistics({ 12, 7, 9, 11, 15, 23, 73 });

		// Act:
		auto multiplier4 = CalculateDynamicFeeMultiplier(statistics.cbegin(), statistics.cend(), 4, BlockFeeMultiplier(123));
		auto multiplier5 = CalculateDynamicFeeMultiplier(statistics.cbegin(), statistics.cend(), 5, BlockFeeMultiplier(123));
		auto multiplier6 = CalculateDynamicFeeMultiplier(statistics.cbegin(), statistics.cend(), 6, BlockFeeMultiplier(123));

		// Assert:
		EXPECT_EQ(BlockFeeMultiplier(11), multiplier4) << "7, 9, <11>, 12";
		EXPECT_EQ(BlockFeeMultiplier(11), multiplier5) << "7, 9, <11>, 12, 15";
		EXPECT_EQ(BlockFeeMultiplier(12), multiplier6) << "7, 9, 11, <12>, 15, 23";
	}

	TEST(TEST_CLASS, CanCalculateMultiplierWhenCandidatesGreaterThanCountInterspersedWithNonCandidates) {
		// Arrange:
		auto statistics = CreateStatistics({ 12, 0, 7, 0, 0, 9, 0, 11, 15, 23, 73 });

		// Act:
		auto multiplier = CalculateDynamicFeeMultiplier(statistics.cbegin(), statistics.cend(), 5, BlockFeeMultiplier(123));

		// Assert:
		EXPECT_EQ(BlockFeeMultiplier(11), multiplier) << "7, 9, <11>, 12, 15";
	}

	// endregion
}}
