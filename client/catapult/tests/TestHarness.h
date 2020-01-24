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

	// custom formatter for byte std::array
	template<size_t N>
	void PrintTo(const array<uint8_t, N>& array, std::ostream* pOut) {
		*pOut << catapult::utils::HexFormat(array);
	}

	// custom formatter for byte std::vector
	inline void PrintTo(const vector<uint8_t>& vector, std::ostream* pOut) {
		*pOut << catapult::utils::HexFormat(vector);
	}
}

namespace catapult { namespace utils {

	// custom formatter for utils::ByteArray
	// (gtest does not seem to find utils::ByteArray insertion operator)
	template<typename TTag>
	void PrintTo(const ByteArray<TTag>& byteArray, std::ostream* pOut) {
		*pOut << byteArray;
	}
}}

namespace catapult { namespace test {

	/// Gets the stress iteration count.
	/// \note \c 0 when non-stress tests are running.
	uint32_t GetStressIterationCount();

	/// Gets the default local host port used in tests.
	unsigned short GetLocalHostPort();

	/// Gets the maximum number of times to retry a non-deterministic test.
	uint32_t GetMaxNonDeterministicTestRetries();

	/// Gets the number of default thread pool threads to use in tests.
	uint32_t GetNumDefaultPoolThreads();
}}

// region custom EXPECT macros

/// Conditionally checks the comparison of \a LHS and \a RHS based on \a EXPECTED_COMPARE_RESULT:
/// - EXPECTED_COMPARE_RESULT < 0 : RHS < LHS
/// - EXPECTED_COMPARE_RESULT > 0 : RHS > LHS
/// - EXPECTED_COMPARE_RESULT == 0: RHS == LHS
#define EXPECT_COMPARE(EXPECTED_COMPARE_RESULT, LHS, RHS) \
	do { \
		if (0 == EXPECTED_COMPARE_RESULT) \
			EXPECT_EQ(LHS, RHS); \
		else if (EXPECTED_COMPARE_RESULT < 0) \
			EXPECT_GT(LHS, RHS); \
		else \
			EXPECT_LT(LHS, RHS); \
	} while (false)

/// Asserts that \a SIZE bytes starting at \a PEXPECTED and \a PACTUAL are equal.
#define EXPECT_EQ_MEMORY(PEXPECTED, PACTUAL, SIZE) \
	EXPECT_TRUE(0 == SIZE || 0 == std::memcmp(PEXPECTED, PACTUAL, SIZE)) \
			<< "E: " << utils::HexFormat(reinterpret_cast<const uint8_t*>(PEXPECTED), reinterpret_cast<const uint8_t*>(PEXPECTED) + SIZE) \
			<< std::endl \
			<< "A: " << utils::HexFormat(reinterpret_cast<const uint8_t*>(PACTUAL), reinterpret_cast<const uint8_t*>(PACTUAL) + SIZE) \
			<< " "

/// Asserts that \a VALUE is contained in \a VALUES with \a MESSAGE.
#define EXPECT_CONTAINS_MESSAGE(VALUES, VALUE, MESSAGE) \
	do { \
		using Printer = testing::internal::UniversalPrinter<typename std::remove_reference_t<decltype(VALUES)>::value_type>; \
		if (VALUES.end() == VALUES.find(VALUE)) { \
			std::ostringstream ecm_out; \
			if (!MESSAGE.empty()) \
				ecm_out << MESSAGE << ":" << std::endl; \
			\
			ecm_out << "{ "; \
			for (const auto& ecm_value : VALUES) \
				Printer::Print(ecm_value, &ecm_out); \
				ecm_out << " "; \
			\
			ecm_out << "} does not contain: "; \
			Printer::Print(VALUE, &ecm_out); \
			EXPECT_TRUE(VALUES.end() != VALUES.find(VALUE)) << ecm_out.str(); \
		} \
	} while (false)

/// Asserts that \a VALUE is contained in \a VALUES.
#define EXPECT_CONTAINS(VALUES, VALUE) EXPECT_CONTAINS_MESSAGE(VALUES, VALUE, std::string())

// endregion

/// Concatenates \a TEST_CLASS and \a TEST_NAME with an underscore separator.
#define CONCAT(TEST_CLASS, TEST_NAME) TEST_CLASS##_##TEST_NAME

/// Creates a name for a traits helper function using \a TEST_CLASS and \a TEST_NAME.
#define TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME) CONCAT(TEST_CLASS, TEST_NAME)
