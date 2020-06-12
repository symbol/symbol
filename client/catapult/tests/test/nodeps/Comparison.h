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
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	/// Asserts that \a op returns expected results for all combinations of values from \a container
	/// with increasing values using a custom \a formatter.
	template<typename TContainer, typename TOperator, typename TFormatter>
	void AssertOperatorBehaviorForIncreasingValues(const TContainer& container, TOperator op, TFormatter formatter) {
		// Act:
		auto containerSize = container.size();
		for (auto i = 0u; i < containerSize; ++i) {
			for (auto j = 0u; j < containerSize; ++j) {
				// Assert:
				EXPECT_EQ(op(i, j), op(container[i], container[j]))
						<< "(" << formatter(container[i]) << ", " << formatter(container[j]) << ") @ "
						<< "(" << i << ", " << j << ")";
			}
		}
	}

	/// Asserts that \a op returns expected results for all combinations of values from \a container
	/// with increasing values using a default formatter.
	template<typename TContainer, typename TOperator>
	void AssertOperatorBehaviorForIncreasingValues(const TContainer& container, TOperator op) {
		AssertOperatorBehaviorForIncreasingValues(container, op, [](const auto& value) { return test::ToString(value); });
	}

	/// Asserts correctness of the less than operator for equal values \a lhs and \a rhs.
	template<typename TValue>
	void AssertLessThanOperatorForEqualValues(const TValue& lhs, const TValue& rhs) {
		EXPECT_FALSE(lhs < rhs);
		EXPECT_FALSE(rhs < lhs);
	}

#define MAKE_COMPARISON_TEST(TEST_CLASS, TEST_NAME, INCREASING_VALUES, OPERATOR) \
	TEST(TEST_CLASS, TEST_NAME) { \
		test::AssertOperatorBehaviorForIncreasingValues(INCREASING_VALUES, [](const auto& lhs, const auto& rhs) { \
			return lhs OPERATOR rhs; \
		}); \
	}

#define MAKE_COMPARISON_CUSTOM_FORMATTER_TEST(TEST_CLASS, TEST_NAME, INCREASING_VALUES, OPERATOR, FORMATTER) \
	TEST(TEST_CLASS, TEST_NAME) { \
		test::AssertOperatorBehaviorForIncreasingValues(INCREASING_VALUES, [](const auto& lhs, const auto& rhs) { \
			return lhs OPERATOR rhs; \
		}, FORMATTER); \
	}

/// Adds all comparison tests to the specified test class (\a TEST_CLASS) given \a INCREASING_VALUES with test name \a PREFIX.
#define DEFINE_COMPARISON_TESTS_WITH_PREFIX(TEST_CLASS, INCREASING_VALUES, PREFIX) \
	MAKE_COMPARISON_TEST(TEST_CLASS, PREFIX##OperatorLessThanReturnsTrueOnlyForSmallerValues, INCREASING_VALUES, <) \
	MAKE_COMPARISON_TEST(TEST_CLASS, PREFIX##OperatorLessThanOrEqualReturnsTrueOnlyForSmallerOrEqualValues, INCREASING_VALUES, <=) \
	MAKE_COMPARISON_TEST(TEST_CLASS, PREFIX##OperatorGreaterThanReturnsTrueOnlyForLargerValues, INCREASING_VALUES, >) \
	MAKE_COMPARISON_TEST(TEST_CLASS, PREFIX##OperatorGreaterThanOrEqualReturnsTrueOnlyForLargerOrEqualValues, INCREASING_VALUES, >=)

/// Adds all comparison tests to the specified test class (\a TEST_CLASS) given \a INCREASING_VALUES.
#define DEFINE_COMPARISON_TESTS(TEST_CLASS, INCREASING_VALUES) DEFINE_COMPARISON_TESTS_WITH_PREFIX(TEST_CLASS, INCREASING_VALUES,)

/// Adds all equality tests to the specified test class (\a TEST_CLASS) given \a INCREASING_VALUES with test name \a PREFIX.
#define DEFINE_EQUALITY_TESTS_WITH_PREFIX(TEST_CLASS, INCREASING_VALUES, PREFIX) \
	MAKE_COMPARISON_TEST(TEST_CLASS, PREFIX##OperatorEqualReturnsTrueOnlyForEqualValues, INCREASING_VALUES, ==) \
	MAKE_COMPARISON_TEST(TEST_CLASS, PREFIX##OperatorNotEqualReturnsTrueOnlyForUnequalValues, INCREASING_VALUES, !=)

/// Adds all equality tests to the specified test class (\a TEST_CLASS) given \a INCREASING_VALUES.
#define DEFINE_EQUALITY_TESTS(TEST_CLASS, INCREASING_VALUES) DEFINE_EQUALITY_TESTS_WITH_PREFIX(TEST_CLASS, INCREASING_VALUES,)

/// Adds all comparison and equality tests to the specified test class (\a TEST_CLASS) given \a INCREASING_VALUES with test name \a PREFIX.
#define DEFINE_EQUALITY_AND_COMPARISON_TESTS_WITH_PREFIX(TEST_CLASS, INCREASING_VALUES, PREFIX) \
	DEFINE_EQUALITY_TESTS_WITH_PREFIX(TEST_CLASS, INCREASING_VALUES, PREFIX) \
	DEFINE_COMPARISON_TESTS_WITH_PREFIX(TEST_CLASS, INCREASING_VALUES, PREFIX)

/// Adds all comparison and equality tests to the specified test class (\a TEST_CLASS) given \a INCREASING_VALUES.
#define DEFINE_EQUALITY_AND_COMPARISON_TESTS(TEST_CLASS, INCREASING_VALUES) \
	DEFINE_EQUALITY_AND_COMPARISON_TESTS_WITH_PREFIX(TEST_CLASS, INCREASING_VALUES,)

/// Adds all equality tests to the specified test class (\a TEST_CLASS) given \a INCREASING_VALUES and \a FORMATTER
/// with test name \a PREFIX.
#define DEFINE_EQUALITY_TESTS_WITH_CUSTOM_FORMATTER_WITH_PREFIX(TEST_CLASS, INCREASING_VALUES, FORMATTER, PREFIX) \
	MAKE_COMPARISON_CUSTOM_FORMATTER_TEST( \
			TEST_CLASS, \
			PREFIX##OperatorEqualReturnsTrueOnlyForEqualValues, \
			INCREASING_VALUES, \
			==, \
			FORMATTER) \
	MAKE_COMPARISON_CUSTOM_FORMATTER_TEST( \
			TEST_CLASS, \
			PREFIX##OperatorNotEqualReturnsTrueOnlyForUnequalValues, \
			INCREASING_VALUES, \
			!=, \
			FORMATTER)

/// Adds all equality tests to the specified test class (\a TEST_CLASS) given \a INCREASING_VALUES and \a FORMATTER.
#define DEFINE_EQUALITY_TESTS_WITH_CUSTOM_FORMATTER(TEST_CLASS, INCREASING_VALUES, FORMATTER) \
	DEFINE_EQUALITY_TESTS_WITH_CUSTOM_FORMATTER_WITH_PREFIX(TEST_CLASS, INCREASING_VALUES, FORMATTER,)
}}
