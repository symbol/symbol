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

#include "catapult/utils/TimeSpan.h"
#include "tests/test/nodeps/Comparison.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS TimeSpanTests

	// region creation

	TEST(TEST_CLASS, CanCreateDefaultTimeSpan) {
		EXPECT_EQ(0u, TimeSpan().millis());
	}

	TEST(TEST_CLASS, CanCreateTimeSpanFromHours) {
		EXPECT_EQ(60 * 60 * 1000u, TimeSpan::FromHours(1).millis());
		EXPECT_EQ(2 * 60 * 60 * 1000u, TimeSpan::FromHours(2).millis());
		EXPECT_EQ(10 * 60 * 60 * 1000u, TimeSpan::FromHours(10).millis());
		EXPECT_EQ(123 * 60 * 60 * 1000u, TimeSpan::FromHours(123).millis());
	}

	TEST(TEST_CLASS, CanCreateTimeSpanFromMinutes) {
		EXPECT_EQ(60 * 1000u, TimeSpan::FromMinutes(1).millis());
		EXPECT_EQ(2 * 60 * 1000u, TimeSpan::FromMinutes(2).millis());
		EXPECT_EQ(10 * 60 * 1000u, TimeSpan::FromMinutes(10).millis());
		EXPECT_EQ(123 * 60 * 1000u, TimeSpan::FromMinutes(123).millis());
	}

	TEST(TEST_CLASS, CanCreateTimeSpanFromSeconds) {
		EXPECT_EQ(1000u, TimeSpan::FromSeconds(1).millis());
		EXPECT_EQ(2 * 1000u, TimeSpan::FromSeconds(2).millis());
		EXPECT_EQ(10 * 1000u, TimeSpan::FromSeconds(10).millis());
		EXPECT_EQ(123 * 1000u, TimeSpan::FromSeconds(123).millis());
	}

	TEST(TEST_CLASS, CanCreateTimeSpanFromMilliseconds) {
		EXPECT_EQ(1u, TimeSpan::FromMilliseconds(1).millis());
		EXPECT_EQ(2u, TimeSpan::FromMilliseconds(2).millis());
		EXPECT_EQ(10u, TimeSpan::FromMilliseconds(10).millis());
		EXPECT_EQ(123u, TimeSpan::FromMilliseconds(123).millis());
	}

	TEST(TEST_CLASS, CanCreateTimeSpanFromTimestampDifference) {
		// Arrange:
		Timestamp start(11111);
		Timestamp end(54321);

		// Assert:
		EXPECT_EQ(43210u, TimeSpan::FromDifference(end, start).millis());
		EXPECT_EQ(0u, TimeSpan::FromDifference(end, end).millis());
		EXPECT_EQ(static_cast<uint64_t>(-43210), TimeSpan::FromDifference(start, end).millis());
	}

	// endregion

	// region accessor conversions

	TEST(TEST_CLASS, HoursAreTruncatedWhenConverted) {
		constexpr uint64_t Base_Millis = 10 * 60 * 60 * 1000u;
		EXPECT_EQ(9u, TimeSpan::FromMilliseconds(Base_Millis - 1).hours());
		EXPECT_EQ(10u, TimeSpan::FromMilliseconds(Base_Millis).hours());
		EXPECT_EQ(10u, TimeSpan::FromMilliseconds(Base_Millis + 1).hours());
	}

	TEST(TEST_CLASS, MinutesAreTruncatedWhenConverted) {
		constexpr uint64_t Base_Millis = 10 * 60 * 1000u;
		EXPECT_EQ(9u, TimeSpan::FromMilliseconds(Base_Millis - 1).minutes());
		EXPECT_EQ(10u, TimeSpan::FromMilliseconds(Base_Millis).minutes());
		EXPECT_EQ(10u, TimeSpan::FromMilliseconds(Base_Millis + 1).minutes());
	}

	TEST(TEST_CLASS, SecondsAreTruncatedWhenConverted) {
		constexpr uint64_t Base_Millis = 10 * 1000u;
		EXPECT_EQ(9u, TimeSpan::FromMilliseconds(Base_Millis - 1).seconds());
		EXPECT_EQ(10u, TimeSpan::FromMilliseconds(Base_Millis).seconds());
		EXPECT_EQ(10u, TimeSpan::FromMilliseconds(Base_Millis + 1).seconds());
	}

	// endregion

	// region equality operators

	namespace {
		std::unordered_set<std::string> GetEqualTags() {
			return { "123000 ms", "123 s", "123000 ms (2)" };
		}

		std::unordered_map<std::string, TimeSpan> GenerateEqualityInstanceMap() {
			return {
				{ "123000 ms", TimeSpan::FromMilliseconds(123000) },
				{ "123 s", TimeSpan::FromSeconds(123) },
				{ "123000 ms (2)", TimeSpan::FromMilliseconds(123000) },

				{ "122999 ms", TimeSpan::FromMilliseconds(122999) },
				{ "123001 ms", TimeSpan::FromMilliseconds(123001) },
				{ "123000 s", TimeSpan::FromSeconds(123000) },
				{ "123 m", TimeSpan::FromMinutes(123) },
				{ "123 h", TimeSpan::FromHours(123) }
			};
		}
	}

	TEST(TEST_CLASS, OperatorEqualReturnsTrueOnlyForEqualValues) {
		test::AssertOperatorEqualReturnsTrueForEqualObjects("123000 ms", GenerateEqualityInstanceMap(), GetEqualTags());
	}

	TEST(TEST_CLASS, OperatorNotEqualReturnsTrueOnlyForUnequalValues) {
		test::AssertOperatorNotEqualReturnsTrueForUnequalObjects("123000 ms", GenerateEqualityInstanceMap(), GetEqualTags());
	}

	// endregion

	// region comparison operators

	namespace {
		std::vector<TimeSpan> GenerateIncreasingValues() {
			return {
				TimeSpan::FromMilliseconds(122999),
				TimeSpan::FromSeconds(123),
				TimeSpan::FromMilliseconds(123001),
				TimeSpan::FromMinutes(123),
				TimeSpan::FromSeconds(123000),
				TimeSpan::FromHours(123)
			};
		}
	}

	DEFINE_COMPARISON_TESTS(TimeSpanTests, GenerateIncreasingValues())

	// endregion

	// region to string

	namespace {
		void AssertStringRepresentation(
				const std::string& expected,
				uint64_t numHours,
				uint64_t numMinutes,
				uint64_t numSeconds,
				uint64_t numMillis) {
			// Arrange:
			auto timeSpan = TimeSpan::FromMilliseconds(((((numHours * 60) + numMinutes) * 60 + numSeconds) * 1000) + numMillis);

			// Act:
			auto str = test::ToString(timeSpan);

			// Assert:
			EXPECT_EQ(expected, str) << numHours << "h " << numMinutes << "m " << numSeconds << "s " << numMillis << "ms";
		}
	}

	TEST(TEST_CLASS, CanOutputTimeSpan) {
		// Assert: zero
		AssertStringRepresentation("00:00:00", 0, 0, 0, 0);

		// - ones
		AssertStringRepresentation("01:00:00", 1, 0, 0, 0);
		AssertStringRepresentation("00:01:00", 0, 1, 0, 0);
		AssertStringRepresentation("00:00:01", 0, 0, 1, 0);
		AssertStringRepresentation("00:00:00.001", 0, 0, 0, 1);

		// - overflows
		AssertStringRepresentation("987:00:00", 987, 0, 0, 0);
		AssertStringRepresentation("62:00:00", 62, 0, 0, 0);
		AssertStringRepresentation("01:02:00", 0, 62, 0, 0);
		AssertStringRepresentation("00:01:02", 0, 0, 62, 0);
		AssertStringRepresentation("00:00:01.020", 0, 0, 0, 1020);

		// - all values
		AssertStringRepresentation("01:01:01.001", 1, 1, 1, 1);
		AssertStringRepresentation("59:59:59.999", 59, 59, 59, 999);
		AssertStringRepresentation("12:52:46.081", 12, 52, 46, 81);
	}

	TEST(TEST_CLASS, OutputFormattingChangesDoNotLeak) {
		// Arrange:
		std::ostringstream out;
		out.flags(std::ios::hex | std::ios::uppercase);
		out.fill('~');

		// Act:
		out << std::setw(4) << 0xAB << " " << TimeSpan::FromSeconds(123) << " " << std::setw(4) << 0xCD;
		auto actual = out.str();

		// Assert:
		EXPECT_EQ("~~AB 00:02:03 ~~CD", actual);
	}

	// endregion

	// region other operators

	TEST(TEST_CLASS, CanAddTimeSpanToTimestamp) {
		// Arrange:
		Timestamp initial(12345678);

		// Act:
		Timestamp results[] = {
			initial + TimeSpan::FromHours(3),
			initial + TimeSpan::FromMinutes(5),
			initial + TimeSpan::FromSeconds(7),
			initial + TimeSpan::FromMilliseconds(1234)
		};

		// Assert:
		EXPECT_EQ(initial + Timestamp(3 * 60 * 60 * 1000), results[0]);
		EXPECT_EQ(initial + Timestamp(5 * 60 * 1000), results[1]);
		EXPECT_EQ(initial + Timestamp(7 * 1000), results[2]);
		EXPECT_EQ(initial + Timestamp(1234), results[3]);
	}

	TEST(TEST_CLASS, CanSubtractTimeSpanFromTimestamp) {
		// Arrange: approximately 3.4 hours
		Timestamp initial(12345678);

		// Act:
		Timestamp results[] = {
			SubtractNonNegative(initial, TimeSpan::FromHours(3)),
			SubtractNonNegative(initial, TimeSpan::FromMinutes(5)),
			SubtractNonNegative(initial, TimeSpan::FromSeconds(7)),
			SubtractNonNegative(initial, TimeSpan::FromMilliseconds(1234)),

			SubtractNonNegative(initial, TimeSpan::FromMilliseconds(12345678)),
			SubtractNonNegative(initial, TimeSpan::FromMilliseconds(12345679)),
			SubtractNonNegative(initial, TimeSpan::FromMilliseconds(99999999))
		};

		// Assert:
		EXPECT_EQ(initial - Timestamp(3 * 60 * 60 * 1000), results[0]);
		EXPECT_EQ(initial - Timestamp(5 * 60 * 1000), results[1]);
		EXPECT_EQ(initial - Timestamp(7 * 1000), results[2]);
		EXPECT_EQ(initial - Timestamp(1234), results[3]);

		EXPECT_EQ(Timestamp(0), results[4]);
		EXPECT_EQ(Timestamp(0), results[5]);
		EXPECT_EQ(Timestamp(0), results[6]);
	}

	// endregion
}}
