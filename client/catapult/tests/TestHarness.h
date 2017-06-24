#pragma once
#include "tests/test/nodeps/Conversions.h"
#include "tests/test/nodeps/Random.h"
#include "tests/test/nodeps/Stress.h" /* note that gtest.h is included by Stress.h */
#include "tests/test/nodeps/Waits.h"

namespace catapult {
	namespace test {
/// The default local host port.
#ifdef STRESS
		constexpr unsigned short Local_Host_Port = 3014;
#else
		constexpr unsigned short Local_Host_Port = 2014;
#endif

		/// The maximum number of times to retry a non-deterministic test.
		constexpr uint32_t Max_Non_Deterministic_Test_Retries = 10;

		/// Gets the number of default thread pool threads to use in tests.
		uint32_t GetNumDefaultPoolThreads();
	}

/// Conditionally checks the comparison of \a LHS and \a RHS based on \a EXPECTED_COMPARE_RESULT:
/// - EXPECTED_COMPARE_RESULT < 0 : RHS < LHS
/// - EXPECTED_COMPARE_RESULT > 0 : RHS > LHS
/// - EXPECTED_COMPARE_RESULT == 0: RHS == LHS
#define EXPECT_COMPARE(EXPECTED_COMPARE_RESULT, LHS, RHS) \
	if (0 == EXPECTED_COMPARE_RESULT) \
		EXPECT_EQ(LHS, RHS); \
	else if (EXPECTED_COMPARE_RESULT < 0) \
		EXPECT_GT(LHS, RHS); \
	else \
		EXPECT_LT(LHS, RHS);

/// Concatenates \a TEST_CLASS and \a TEST_NAME with an underscore separator.
#define CONCAT(TEST_CLASS, TEST_NAME) TEST_CLASS##_##TEST_NAME

/// Creates a name for a traits helper function using \a TEST_CLASS and \a TEST_NAME.
#define TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME) CONCAT(TEST_CLASS, TEST_NAME)
}
