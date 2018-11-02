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

#include "src/state/NamespaceLifetime.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS NamespaceLifetimeTests

	// region ctor

	TEST(TEST_CLASS, CanCreateNamespaceLifetimeWithoutGracePeriod) {
		// Act:
		NamespaceLifetime lifetime(Height(123), Height(234));

		// Assert:
		EXPECT_EQ(Height(123), lifetime.Start);
		EXPECT_EQ(Height(234), lifetime.End);
		EXPECT_EQ(Height(234), lifetime.GracePeriodEnd);
	}

	TEST(TEST_CLASS, CanCreateNamespaceLifetimeWithGracePeriod) {
		// Act:
		NamespaceLifetime lifetime(Height(123), Height(234), BlockDuration(50));

		// Assert:
		EXPECT_EQ(Height(123), lifetime.Start);
		EXPECT_EQ(Height(234), lifetime.End);
		EXPECT_EQ(Height(284), lifetime.GracePeriodEnd);
	}

	TEST(TEST_CLASS, CanCreateNamespaceLifetimeWithMinLifetimeWithoutGracePeriod) {
		// Act:
		NamespaceLifetime lifetime(Height(233), Height(234));

		// Assert:
		EXPECT_EQ(Height(233), lifetime.Start);
		EXPECT_EQ(Height(234), lifetime.End);
		EXPECT_EQ(Height(234), lifetime.GracePeriodEnd);
	}

	TEST(TEST_CLASS, CanCreateNamespaceLifetimeWithMinLifetimeWithGracePeriod) {
		// Act:
		NamespaceLifetime lifetime(Height(233), Height(234), BlockDuration(50));

		// Assert:
		EXPECT_EQ(Height(233), lifetime.Start);
		EXPECT_EQ(Height(234), lifetime.End);
		EXPECT_EQ(Height(284), lifetime.GracePeriodEnd);
	}

	TEST(TEST_CLASS, CannotCreateNamespaceLifetimeWithZeroLifetime) {
		// Act + Assert:
		EXPECT_THROW(NamespaceLifetime(Height(123), Height(123)), catapult_invalid_argument);

		EXPECT_THROW(NamespaceLifetime(Height(123), Height(123), BlockDuration(50)), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CannotCreateNamespaceLifetimeWithNegativeLifetime) {
		// Act + Assert:
		EXPECT_THROW(NamespaceLifetime(Height(125), Height(124)), catapult_invalid_argument);
		EXPECT_THROW(NamespaceLifetime(Height(125), Height(63)), catapult_invalid_argument);

		EXPECT_THROW(NamespaceLifetime(Height(125), Height(124), BlockDuration(50)), catapult_invalid_argument);
		EXPECT_THROW(NamespaceLifetime(Height(125), Height(63), BlockDuration(50)), catapult_invalid_argument);
	}

	// endregion

	// region isActiveAndUnlocked / isActiveOrGracePeriod

	TEST(TEST_CLASS, IsActiveAndUnlockedReturnsTrueIfHeightIsWithinLifetime) {
		// Arrange:
		NamespaceLifetime lifetime(Height(123), Height(234), BlockDuration(50));

		// Assert:
		for (auto height : { 123u, 124u, 213u, 232u, 233u })
			EXPECT_TRUE(lifetime.isActiveAndUnlocked(Height(height))) << "at height " << height;
	}

	TEST(TEST_CLASS, IsActiveAndUnlockedReturnsFalseIfHeightIsNotWithinLifetime) {
		// Arrange:
		NamespaceLifetime lifetime(Height(123), Height(234), BlockDuration(50));

		// Assert:
		for (auto height : { 1u, 2u, 121u, 122u, 234u, 235u, 5000u, 100000u })
			EXPECT_FALSE(lifetime.isActiveAndUnlocked(Height(height))) << "at height " << height;
	}

	TEST(TEST_CLASS, IsActiveOrGracePeriodReturnsTrueIfHeightIsWithinLifetimeOrGracePeriod) {
		// Arrange:
		NamespaceLifetime lifetime(Height(123), Height(234), BlockDuration(50));

		// Assert:
		for (auto height : { 123u, 124u, 213u, 232u, 233u, 234u, 235u, 283u })
			EXPECT_TRUE(lifetime.isActiveOrGracePeriod(Height(height))) << "at height " << height;
	}

	TEST(TEST_CLASS, IsActiveOrGracePeriodReturnsFalseIfHeightIsNotWithinLifetimeOrGracePeriod) {
		// Arrange:
		NamespaceLifetime lifetime(Height(123), Height(234), BlockDuration(50));

		// Assert:
		for (auto height : { 1u, 2u, 121u, 122u, 284u, 285u, 5000u, 100000u })
			EXPECT_FALSE(lifetime.isActiveOrGracePeriod(Height(height))) << "at height " << height;
	}

	// endregion

	// region equality

	namespace {
		const char* Default_Key = "default";

		auto GenerateEqualityInstanceMap() {
			std::unordered_map<std::string, NamespaceLifetime> map;
			map.emplace(Default_Key, NamespaceLifetime(Height(123), Height(234)));
			map.emplace("copy", NamespaceLifetime(Height(123), Height(234)));
			map.emplace("diff-start-height", NamespaceLifetime(Height(169), Height(234)));
			map.emplace("diff-end-height", NamespaceLifetime(Height(123), Height(345)));
			map.emplace("diff-grace-period", NamespaceLifetime(Height(123), Height(234), BlockDuration(1)));
			map.emplace("diff-start-and-end-height", NamespaceLifetime(Height(169), Height(345)));
			return map;
		}

		std::unordered_set<std::string> GetEqualTags() {
			return { Default_Key, "copy" };
		}
	}

	TEST(TEST_CLASS, OperatorEqualReturnsTrueForEqualObjects) {
		// Assert:
		test::AssertOperatorEqualReturnsTrueForEqualObjects(Default_Key, GenerateEqualityInstanceMap(), GetEqualTags());
	}

	TEST(TEST_CLASS, OperatorNotEqualReturnsTrueForUnequalObjects) {
		// Assert:
		test::AssertOperatorNotEqualReturnsTrueForUnequalObjects(Default_Key, GenerateEqualityInstanceMap(), GetEqualTags());
	}

	// endregion
}}
