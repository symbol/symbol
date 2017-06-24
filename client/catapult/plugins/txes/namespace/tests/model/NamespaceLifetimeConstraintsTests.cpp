#include "src/model/NamespaceLifetimeConstraints.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

	// region constructor

	TEST(NamespaceLifetimeConstraintsTests, CanCreateNamespaceLifetimeConstraints) {
		// Act:
		NamespaceLifetimeConstraints constraints(ArtifactDuration(123), ArtifactDuration(234), 345);

		// Assert:
		EXPECT_EQ(ArtifactDuration(234 + 345), constraints.TotalGracePeriodDuration);
		EXPECT_EQ(ArtifactDuration(123 + 234), constraints.MaxNamespaceDuration);
	}

	// endregion

	// region IsWithinLifetimePlusDuration

	TEST(NamespaceLifetimeConstraintsTests, IsWithinLifetimePlusDurationReturnsTrueIfWithinLifetime) {
		// Arrange:
		Height lifetimeEnd(123);
		NamespaceLifetimeConstraints constraints(ArtifactDuration(0), ArtifactDuration(15), 5);

		// Assert:
		for (auto height : { 1u, 19u, 20u, 21u, 57u, 122u })
			EXPECT_TRUE(constraints.IsWithinLifetimePlusDuration(lifetimeEnd, Height(height)));
	}

	TEST(NamespaceLifetimeConstraintsTests, IsWithinLifetimePlusDurationReturnsTrueIfWithinGracePeriod) {
		// Arrange:
		Height lifetimeEnd(123);
		NamespaceLifetimeConstraints constraints(ArtifactDuration(0), ArtifactDuration(15), 5);

		// Assert:
		for (auto height : { 123u, 141u, 142u })
			EXPECT_TRUE(constraints.IsWithinLifetimePlusDuration(lifetimeEnd, Height(height)));
	}

	TEST(NamespaceLifetimeConstraintsTests, IsWithinLifetimePlusDurationReturnsFalseIfOutsideGracePeriod) {
		// Arrange:
		Height lifetimeEnd(123);
		NamespaceLifetimeConstraints constraints(ArtifactDuration(0), ArtifactDuration(15), 5);

		// Assert:
		for (auto height : { 143u, 144u, 10000u })
			EXPECT_FALSE(constraints.IsWithinLifetimePlusDuration(lifetimeEnd, Height(height)));
	}

	TEST(NamespaceLifetimeConstraintsTests, IsWithinLifetimePlusDurationHandlesOverflowCorrectly) {
		// Arrange:
		std::vector<Height::ValueType> testHeights{ 5, 20, 1000, std::numeric_limits<Height::ValueType>::max() };
		Height lifetimeEnd(std::numeric_limits<Height::ValueType>::max() - 10);
		NamespaceLifetimeConstraints constraints(ArtifactDuration(0), ArtifactDuration(15), 5);

		// Assert:
		for (auto height : testHeights)
			EXPECT_TRUE(constraints.IsWithinLifetimePlusDuration(lifetimeEnd, Height(height)));
	}

	// endregion
}}
