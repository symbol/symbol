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

#pragma once
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

	/// Lock duration validator test suite.
	template<typename TTraits>
	struct LockDurationTests {
	public:
		static void AssertFailureWhenDurationIsZero() {
			// Act + Assert:
			AssertDurationValidator(TTraits::Failure_Result, BlockDuration(0));
		}

		static void AssertFailureWhenDurationIsGreaterThanMaxDurationSetting() {
			// Act + Assert:
			for (auto duration : { BlockDuration(1), BlockDuration(100) })
				AssertDurationValidator(TTraits::Failure_Result, MaxDuration() + duration);
		}

		static void AssertSuccessWhenDurationIsLessThanOrEqualToMaxDurationSetting() {
			// Act + Assert:
			for (auto duration : { MaxDuration() - BlockDuration(1), BlockDuration(10), BlockDuration(1), BlockDuration(0) })
				AssertDurationValidator(ValidationResult::Success, MaxDuration() - duration);
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
	MAKE_DURATION_VALIDATOR_TEST(TRAITS_NAME, FailureWhenDurationIsZero) \
	MAKE_DURATION_VALIDATOR_TEST(TRAITS_NAME, FailureWhenDurationIsGreaterThanMaxDurationSetting) \
	MAKE_DURATION_VALIDATOR_TEST(TRAITS_NAME, SuccessWhenDurationIsLessThanOrEqualToMaxDurationSetting)
