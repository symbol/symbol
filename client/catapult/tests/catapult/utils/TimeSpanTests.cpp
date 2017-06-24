#include "catapult/utils/TimeSpan.h"
#include "tests/test/nodeps/Comparison.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

	// region creation

	TEST(TimeSpanTests, CanCreateDefaultTimeSpan) {
		// Assert:
		EXPECT_EQ(0u, TimeSpan().millis());
	}

	TEST(TimeSpanTests, CanCreateTimeSpanFromHours) {
		// Assert:
		EXPECT_EQ(60 * 60 * 1000u, TimeSpan::FromHours(1).millis());
		EXPECT_EQ(2 * 60 * 60 * 1000u, TimeSpan::FromHours(2).millis());
		EXPECT_EQ(10 * 60 * 60 * 1000u, TimeSpan::FromHours(10).millis());
		EXPECT_EQ(123 * 60 * 60 * 1000u, TimeSpan::FromHours(123).millis());
	}

	TEST(TimeSpanTests, CanCreateTimeSpanFromMinutes) {
		// Assert:
		EXPECT_EQ(60 * 1000u, TimeSpan::FromMinutes(1).millis());
		EXPECT_EQ(2 * 60 * 1000u, TimeSpan::FromMinutes(2).millis());
		EXPECT_EQ(10 * 60 * 1000u, TimeSpan::FromMinutes(10).millis());
		EXPECT_EQ(123 * 60 * 1000u, TimeSpan::FromMinutes(123).millis());
	}

	TEST(TimeSpanTests, CanCreateTimeSpanFromSeconds) {
		// Assert:
		EXPECT_EQ(1000u, TimeSpan::FromSeconds(1).millis());
		EXPECT_EQ(2 * 1000u, TimeSpan::FromSeconds(2).millis());
		EXPECT_EQ(10 * 1000u, TimeSpan::FromSeconds(10).millis());
		EXPECT_EQ(123 * 1000u, TimeSpan::FromSeconds(123).millis());
	}

	TEST(TimeSpanTests, CanCreateTimeSpanFromMilliseconds) {
		// Assert:
		EXPECT_EQ(1u, TimeSpan::FromMilliseconds(1).millis());
		EXPECT_EQ(2u, TimeSpan::FromMilliseconds(2).millis());
		EXPECT_EQ(10u, TimeSpan::FromMilliseconds(10).millis());
		EXPECT_EQ(123u, TimeSpan::FromMilliseconds(123).millis());
	}

	TEST(TimeSpanTests, CanCreateTimeSpanFromTimestampDifference) {
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

	TEST(TimeSpanTests, HoursAreTruncatedWhenConverted) {
		// Assert:
		constexpr uint64_t Base_Millis = 10 * 60 * 60 * 1000u;
		EXPECT_EQ(9u, TimeSpan::FromMilliseconds(Base_Millis - 1).hours());
		EXPECT_EQ(10u, TimeSpan::FromMilliseconds(Base_Millis).hours());
		EXPECT_EQ(10u, TimeSpan::FromMilliseconds(Base_Millis + 1).hours());
	}

	TEST(TimeSpanTests, MinutesAreTruncatedWhenConverted) {
		// Assert:
		constexpr uint64_t Base_Millis = 10 * 60 * 1000u;
		EXPECT_EQ(9u, TimeSpan::FromMilliseconds(Base_Millis - 1).minutes());
		EXPECT_EQ(10u, TimeSpan::FromMilliseconds(Base_Millis).minutes());
		EXPECT_EQ(10u, TimeSpan::FromMilliseconds(Base_Millis + 1).minutes());
	}

	TEST(TimeSpanTests, SecondsAreTruncatedWhenConverted) {
		// Assert:
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
				{ "123 h", TimeSpan::FromHours(123) },
			};
		}
	}

	TEST(TimeSpanTests, OperatorEqualReturnsTrueOnlyForEqualValues) {
		// Assert:
		test::AssertOperatorEqualReturnsTrueForEqualObjects("123000 ms", GenerateEqualityInstanceMap(), GetEqualTags());
	}

	TEST(TimeSpanTests, OperatorNotEqualReturnsTrueOnlyForUnequalValues) {
		// Assert:
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

	TEST(TimeSpanTests, CanOutputTimeSpan) {
		// Assert:
		// - zero
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

	TEST(TimeSpanTests, OutputFormattingChangesDoNotLeak) {
		// Arrange:
		std::ostringstream out;
		out.flags(std::ios::hex | std::ios::uppercase);
		out.fill('~');

		// Act:
		out << std::setw(4) << 0xAB << " " << TimeSpan::FromSeconds(123) << " " << std::setw(4) << 0xCD;
		std::string actual = out.str();

		// Assert:
		EXPECT_EQ("~~AB 00:02:03 ~~CD", actual);
	}

	// endregion

	// region other operators

	TEST(TimeSpanTests, CanAddTimeSpanToTimestamp) {
		/// Arrange:
		Timestamp initial(12345678);

		/// Act:
		Timestamp results[] = {
			initial + TimeSpan::FromHours(3),
			initial + TimeSpan::FromMinutes(5),
			initial + TimeSpan::FromSeconds(7),
			initial + TimeSpan::FromMilliseconds(1234)
		};

		/// Assert:
		EXPECT_EQ(initial + Timestamp(3 * 60 * 60 * 1000u), results[0]);
		EXPECT_EQ(initial + Timestamp(5 * 60 * 1000u), results[1]);
		EXPECT_EQ(initial + Timestamp(7 * 1000u), results[2]);
		EXPECT_EQ(initial + Timestamp(1234u), results[3]);
	}

	// endregion
}}
