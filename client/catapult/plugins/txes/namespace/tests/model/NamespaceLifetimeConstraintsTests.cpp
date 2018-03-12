#include "src/model/NamespaceLifetimeConstraints.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS NamespaceLifetimeConstraintsTests

	// region constructor

	TEST(TEST_CLASS, CanCreateNamespaceLifetimeConstraints) {
		// Act:
		NamespaceLifetimeConstraints constraints(BlockDuration(123), BlockDuration(234), 345);

		// Assert:
		EXPECT_EQ(BlockDuration(234 + 345), constraints.TotalGracePeriodDuration);
		EXPECT_EQ(BlockDuration(123 + 234), constraints.MaxNamespaceDuration);
	}

	// endregion

	// region IsWithinLifetimePlusDuration

	TEST(TEST_CLASS, IsWithinLifetimePlusDurationReturnsTrueIfWithinLifetime) {
		// Arrange:
		Height lifetimeEnd(123);
		NamespaceLifetimeConstraints constraints(BlockDuration(0), BlockDuration(15), 5);

		// Assert:
		for (auto height : { 1u, 19u, 20u, 21u, 57u, 122u })
			EXPECT_TRUE(constraints.IsWithinLifetimePlusDuration(lifetimeEnd, Height(height)));
	}

	TEST(TEST_CLASS, IsWithinLifetimePlusDurationReturnsTrueIfWithinGracePeriod) {
		// Arrange:
		Height lifetimeEnd(123);
		NamespaceLifetimeConstraints constraints(BlockDuration(0), BlockDuration(15), 5);

		// Assert:
		for (auto height : { 123u, 141u, 142u })
			EXPECT_TRUE(constraints.IsWithinLifetimePlusDuration(lifetimeEnd, Height(height)));
	}

	TEST(TEST_CLASS, IsWithinLifetimePlusDurationReturnsFalseIfOutsideGracePeriod) {
		// Arrange:
		Height lifetimeEnd(123);
		NamespaceLifetimeConstraints constraints(BlockDuration(0), BlockDuration(15), 5);

		// Assert:
		for (auto height : { 143u, 144u, 10000u })
			EXPECT_FALSE(constraints.IsWithinLifetimePlusDuration(lifetimeEnd, Height(height)));
	}

	TEST(TEST_CLASS, IsWithinLifetimePlusDurationHandlesOverflowCorrectly) {
		// Arrange:
		std::vector<Height::ValueType> testHeights{ 5, 20, 1000, std::numeric_limits<Height::ValueType>::max() };
		Height lifetimeEnd(std::numeric_limits<Height::ValueType>::max() - 10);
		NamespaceLifetimeConstraints constraints(BlockDuration(0), BlockDuration(15), 5);

		// Assert:
		for (auto height : testHeights)
			EXPECT_TRUE(constraints.IsWithinLifetimePlusDuration(lifetimeEnd, Height(height)));
	}

	// endregion
}}
