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

#pragma once
#include "catapult/utils/HexFormatter.h"
#include "tests/test/nodeps/Conversions.h"
#include "tests/test/nodeps/Random.h"
#include "tests/test/nodeps/Stress.h" /* note that gtest.h is included by Stress.h */
#include "tests/test/nodeps/Waits.h"

// EXPECT_EQ is not used anywhere to compare different types of null pointers
// so the following hack is used to circumvent clang complaints inside gtest
#undef GTEST_IS_NULL_LITERAL_
#define GTEST_IS_NULL_LITERAL_(x) false

namespace std {

	// custom formatter for byte arrays
	template<size_t N>
	void PrintTo(const array<uint8_t, N>& array, std::ostream* pOut) {
		*pOut << catapult::utils::HexFormat(array);
	}
}

namespace catapult { namespace test {

		/// Default local host port used in tests.
#ifdef STRESS
		constexpr unsigned short Local_Host_Port = 3014;
#else
		constexpr unsigned short Local_Host_Port = 2014;
#endif

		/// Maximum number of times to retry a non-deterministic test.
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
