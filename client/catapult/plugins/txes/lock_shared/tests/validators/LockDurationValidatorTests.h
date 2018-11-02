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
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

	/// Lock duration validator test suite.
	template<typename TTraits>
	struct LockDurationTests {
	public:
		static void AssertFailureIfDurationIsGreaterThanMaxDurationSetting() {
			// Act + Assert:
			for (auto duration : { 1u, 100u })
				AssertDurationValidator(TTraits::Failure_Result, MaxDuration() + BlockDuration(duration));
		}

		static void AssertSuccessIfDurationIsLessThanOrEqualToMaxDurationSetting() {
			// Act + Assert:
			for (auto duration : { 10u, 1u, 0u })
				AssertDurationValidator(ValidationResult::Success, MaxDuration() - BlockDuration(duration));
		}

	private:
		static constexpr BlockDuration MaxDuration() {
			return BlockDuration(100);
		}

		static void AssertDurationValidator(ValidationResult expectedResult, BlockDuration notificationDuration) {
			// Arrange:
			typename TTraits::NotificationType notification(notificationDuration);
			auto pValidator = TTraits::CreateValidator(MaxDuration());

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "duration " << notification.Duration;
		}
	};
}}

#define MAKE_DURATION_VALIDATOR_TEST(TRAITS_NAME, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { LockDurationTests<TRAITS_NAME>::Assert##TEST_NAME(); }

#define DEFINE_DURATION_VALIDATOR_TESTS(TRAITS_NAME) \
	MAKE_DURATION_VALIDATOR_TEST(TRAITS_NAME, FailureIfDurationIsGreaterThanMaxDurationSetting) \
	MAKE_DURATION_VALIDATOR_TEST(TRAITS_NAME, SuccessIfDurationIsLessThanOrEqualToMaxDurationSetting)
