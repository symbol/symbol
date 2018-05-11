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
