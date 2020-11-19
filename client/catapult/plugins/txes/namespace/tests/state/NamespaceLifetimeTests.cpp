/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include <limits>

namespace catapult { namespace state {

#define TEST_CLASS NamespaceLifetimeTests

	// region ctor

	TEST(TEST_CLASS, CanCreateNamespaceLifetimeWithoutGracePeriod) {
		// Act:
		NamespaceLifetime lifetime(Height(123), Height(234));

		// Assert:
		EXPECT_EQ(Height(123), lifetime.Start);
		EXPECT_EQ(Height(234), lifetime.End);
	}

	TEST(TEST_CLASS, CanCreateEternalNamespaceLifetimeWithoutGracePeriod) {
		// Act:
		NamespaceLifetime lifetime(Height(123), Height(0xFFFF'FFFF'FFFF'FFFF));

		// Assert:
		EXPECT_EQ(Height(123), lifetime.Start);
		EXPECT_EQ(Height(0xFFFF'FFFF'FFFF'FFFF), lifetime.End);
	}

	TEST(TEST_CLASS, CanCreateNamespaceLifetimeWithGracePeriod) {
		// Act:
		NamespaceLifetime lifetime(Height(123), Height(234), BlockDuration(50));

		// Assert:
		EXPECT_EQ(Height(123), lifetime.Start);
		EXPECT_EQ(Height(284), lifetime.End);
	}

	TEST(TEST_CLASS, CanCreateEternalNamespaceLifetimeWithGracePeriod) {
		// Act:
		NamespaceLifetime lifetime(Height(123), Height(0xFFFF'FFFF'FFFF'FFFF), BlockDuration(50));

		// Assert:
		EXPECT_EQ(Height(123), lifetime.Start);
		EXPECT_EQ(Height(0xFFFF'FFFF'FFFF'FFFF), lifetime.End);
	}

	TEST(TEST_CLASS, CanCreateNamespaceLifetimeWithMinLifetimeWithoutGracePeriod) {
		// Act:
		NamespaceLifetime lifetime(Height(233), Height(234));

		// Assert:
		EXPECT_EQ(Height(233), lifetime.Start);
		EXPECT_EQ(Height(234), lifetime.End);
	}

	TEST(TEST_CLASS, CanCreateNamespaceLifetimeWithMinLifetimeWithGracePeriod) {
		// Act:
		NamespaceLifetime lifetime(Height(233), Height(234), BlockDuration(50));

		// Assert:
		EXPECT_EQ(Height(233), lifetime.Start);
		EXPECT_EQ(Height(284), lifetime.End);
	}

	TEST(TEST_CLASS, CanCreateNamespaceLifetimeWithMaxGracePeriodEnd) {
		// Act:
		auto maxValue = std::numeric_limits<uint64_t>::max();
		NamespaceLifetime lifetime(Height(123), Height(234), BlockDuration(maxValue - 234));

		// Assert:
		EXPECT_EQ(Height(123), lifetime.Start);
		EXPECT_EQ(Height(maxValue), lifetime.End);
	}

	TEST(TEST_CLASS, CannotCreateNamespaceLifetimeWithZeroLifetime) {
		EXPECT_THROW(NamespaceLifetime(Height(123), Height(123)), catapult_invalid_argument);

		EXPECT_THROW(NamespaceLifetime(Height(123), Height(123), BlockDuration(50)), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CannotCreateNamespaceLifetimeWithNegativeLifetime) {
		EXPECT_THROW(NamespaceLifetime(Height(125), Height(124)), catapult_invalid_argument);
		EXPECT_THROW(NamespaceLifetime(Height(125), Height(63)), catapult_invalid_argument);

		EXPECT_THROW(NamespaceLifetime(Height(125), Height(124), BlockDuration(50)), catapult_invalid_argument);
		EXPECT_THROW(NamespaceLifetime(Height(125), Height(63), BlockDuration(50)), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CannotCreateNamespaceWithGracePeriodEndOverflow) {
		auto maxValue = std::numeric_limits<uint64_t>::max();
		EXPECT_THROW(NamespaceLifetime(Height(123), Height(234), BlockDuration(maxValue - 234 + 1)), catapult_invalid_argument);
		EXPECT_THROW(NamespaceLifetime(Height(123), Height(234), BlockDuration(maxValue - 123 + 1)), catapult_invalid_argument);
		EXPECT_THROW(NamespaceLifetime(Height(123), Height(234), BlockDuration(maxValue)), catapult_invalid_argument);
	}

	// endregion

	// region isActive / isActiveExcludingGracePeriod

	TEST(TEST_CLASS, IsActiveReturnsTrueWhenHeightIsWithinLifetimeOrGracePeriod) {
		// Arrange:
		NamespaceLifetime lifetime(Height(123), Height(234), BlockDuration(50));

		// Assert:
		for (auto height : { 123u, 124u, 213u, 232u, 233u, 234u, 235u, 283u })
			EXPECT_TRUE(lifetime.isActive(Height(height))) << "at height " << height;
	}

	TEST(TEST_CLASS, IsActiveReturnsFalseWhenHeightIsNotWithinLifetimeOrGracePeriod) {
		// Arrange:
		NamespaceLifetime lifetime(Height(123), Height(234), BlockDuration(50));

		// Assert:
		for (auto height : { 1u, 2u, 121u, 122u, 284u, 285u, 5000u, 100000u })
			EXPECT_FALSE(lifetime.isActive(Height(height))) << "at height " << height;
	}

	TEST(TEST_CLASS, IsActiveExcludingGracePeriodReturnsTrueWhenHeightIsWithinLifetime) {
		// Arrange:
		NamespaceLifetime lifetime(Height(123), Height(259), BlockDuration(25));

		// Assert: notice that isActiveExcludingGracePeriod uses grace period passed in (50), not from constructor (25)
		for (auto height : { 123u, 124u, 213u, 232u, 233u })
			EXPECT_TRUE(lifetime.isActiveExcludingGracePeriod(Height(height), BlockDuration(50))) << "at height " << height;
	}

	TEST(TEST_CLASS, IsActiveExcludingGracePeriodReturnsFalseWhenHeightIsNotWithinLifetime) {
		// Arrange:
		NamespaceLifetime lifetime(Height(123), Height(259), BlockDuration(25));

		// Assert: notice that isActiveExcludingGracePeriod uses grace period passed in (50), not from constructor (25)
		for (auto height : { 1u, 2u, 121u, 122u, 234u, 235u, 5000u, 100000u })
			EXPECT_FALSE(lifetime.isActiveExcludingGracePeriod(Height(height), BlockDuration(50))) << "at height " << height;
	}

	TEST(TEST_CLASS, IsActiveExcludingGracePeriodSucceedsWhenGracePeriodIsLessThanLifetime) {
		// Arrange: lifetime is 259 + 25 - 123 = 161
		NamespaceLifetime lifetime(Height(123), Height(259), BlockDuration(25));

		// Assert: notice that isActiveExcludingGracePeriod uses grace period passed in (50), not from constructor (25)
		for (auto gracePeriodDuration : { 0u, 1u, 100u, 159u, 160u }) {
			EXPECT_TRUE(lifetime.isActiveExcludingGracePeriod(Height(123), BlockDuration(gracePeriodDuration)))
					<< "with grace period duration " << gracePeriodDuration;
		}
	}

	TEST(TEST_CLASS, IsActiveExcludingGracePeriodFailsWhenGracePeriodIsGreaterThanOrEqualToLifetime) {
		// Arrange: lifetime is 259 + 25 - 123 = 161
		NamespaceLifetime lifetime(Height(123), Height(259), BlockDuration(25));

		// Assert: notice that isActiveExcludingGracePeriod uses grace period passed in (50), not from constructor (25)
		for (auto gracePeriodDuration : { 161u, 162u, 259u, 260u, 5000u, 100000u }) {
			EXPECT_THROW(lifetime.isActiveExcludingGracePeriod(Height(222), BlockDuration(gracePeriodDuration)), catapult_invalid_argument)
					<< "with grace period duration " << gracePeriodDuration;
		}
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
		test::AssertOperatorEqualReturnsTrueForEqualObjects(Default_Key, GenerateEqualityInstanceMap(), GetEqualTags());
	}

	TEST(TEST_CLASS, OperatorNotEqualReturnsTrueForUnequalObjects) {
		test::AssertOperatorNotEqualReturnsTrueForUnequalObjects(Default_Key, GenerateEqualityInstanceMap(), GetEqualTags());
	}

	// endregion
}}
