#pragma once
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	/// Asserts that \a op returns expected results for all combinations of values from a
	/// \a container with increasing values.
	template<typename TContainer, typename TOperator>
	void AssertOperatorBehaviorForIncreasingValues(const TContainer& container, TOperator op) {
		// Act:
		auto containerSize = container.size();
		for (auto i = 0u; i < containerSize; ++i) {
			for (auto j = 0u; j < containerSize; ++j) {
				// Assert:
				EXPECT_EQ(op(i, j), op(container[i], container[j]))
						<< "(" << container[i] << ", " << container[j] << ") @ "
						<< "(" << i << ", " << j << ")";
			}
		}
	}

#define MAKE_COMPARISON_TEST(TEST_CLASS, TEST_NAME, INCREASING_VALUES, OPERATOR) \
	TEST(TEST_CLASS, TEST_NAME) { \
		test::AssertOperatorBehaviorForIncreasingValues( \
				INCREASING_VALUES, \
				[](const auto& lhs, const auto& rhs) { return lhs OPERATOR rhs; }); \
	}

/// Adds all comparison tests to the specified test class (\a TEST_CLASS) given \a INCREASING_VALUES.
#define DEFINE_COMPARISON_TESTS(TEST_CLASS, INCREASING_VALUES) \
	MAKE_COMPARISON_TEST(TEST_CLASS, OperatorLessThanReturnsTrueOnlyForSmallerValues, INCREASING_VALUES, <) \
	MAKE_COMPARISON_TEST(TEST_CLASS, OperatorLessThanOrEqualReturnsTrueOnlyForSmallerOrEqualValues, INCREASING_VALUES, <=) \
	MAKE_COMPARISON_TEST(TEST_CLASS, OperatorGreaterThanReturnsTrueOnlyForLargerValues, INCREASING_VALUES, >) \
	MAKE_COMPARISON_TEST(TEST_CLASS, OperatorGreaterThanOrEqualReturnsTrueOnlyForLargerOrEqualValues, INCREASING_VALUES, >=)

/// Adds all comparison and equality tests to the specified test class (\a TEST_CLASS) given \a INCREASING_VALUES.
#define DEFINE_EQUALITY_AND_COMPARISON_TESTS(TEST_CLASS, INCREASING_VALUES) \
	MAKE_COMPARISON_TEST(TEST_CLASS, OperatorEqualReturnsTrueOnlyForEqualValues, INCREASING_VALUES, ==) \
	MAKE_COMPARISON_TEST(TEST_CLASS, OperatorNotEqualReturnsTrueOnlyForUnequalValues, INCREASING_VALUES, !=) \
	DEFINE_COMPARISON_TESTS(TEST_CLASS, INCREASING_VALUES)
}}
