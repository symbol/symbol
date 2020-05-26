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

#include "src/validators/Validators.h"
#include "catapult/model/VerifiableEntity.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS DeadlineValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(Deadline, utils::TimeSpan::FromSeconds(14))

	namespace {
		constexpr auto Block_Time = Timestamp(8888);
		constexpr auto TimeSpanFromHours = utils::TimeSpan::FromHours;

		void AssertValidationResult(ValidationResult expectedResult, Timestamp deadline, const utils::TimeSpan& maxCustomLifetime) {
			// Arrange:
			auto cache = test::CreateEmptyCatapultCache();
			auto cacheView = cache.createView();
			auto readOnlyCache = cacheView.toReadOnly();
			auto notificationContext = model::NotificationContext(Height(123), test::CreateResolverContextXor());
			auto validatorContext = ValidatorContext(notificationContext, Block_Time, model::NetworkInfo(), readOnlyCache);
			auto pValidator = CreateDeadlineValidator(TimeSpanFromHours(2));

			model::TransactionDeadlineNotification notification(deadline, maxCustomLifetime);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, validatorContext);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "deadline " << deadline << ", maxCustomLifetime " << maxCustomLifetime;
		}
	}

	// region basic tests

	TEST(TEST_CLASS, FailureWhenTransactionDeadlineIsLessThanBlockTime) {
		for (auto i = 0u; i < 4; ++i)
			AssertValidationResult(Failure_Core_Past_Deadline, Block_Time - Timestamp(1), TimeSpanFromHours(i));
	}

	TEST(TEST_CLASS, SuccessWhenTransactionDeadlineIsEqualToBlockTime) {
		for (auto i = 0u; i < 4; ++i)
			AssertValidationResult(ValidationResult::Success, Block_Time, TimeSpanFromHours(i));
	}

	TEST(TEST_CLASS, SuccessWhenTransactionDeadlineIsValid) {
		for (auto i = 0u; i < 4; ++i)
			AssertValidationResult(ValidationResult::Success, Block_Time + utils::TimeSpan::FromMinutes(30), TimeSpanFromHours(i));
	}

	TEST(TEST_CLASS, SuccessWhenTransactionDeadlineIsEqualToBlockTimePlusLifetime) {
		AssertValidationResult(ValidationResult::Success, Block_Time + TimeSpanFromHours(2), utils::TimeSpan());

		for (auto i = 1u; i < 4; ++i)
			AssertValidationResult(ValidationResult::Success, Block_Time + TimeSpanFromHours(i), TimeSpanFromHours(i));
	}

	TEST(TEST_CLASS, FailureWhenTransactionDeadlineIsGreaterThanBlockTimePlusLifetime) {
		AssertValidationResult(Failure_Core_Future_Deadline, Block_Time + TimeSpanFromHours(3), utils::TimeSpan());

		for (auto i = 1u; i < 4; ++i)
			AssertValidationResult(Failure_Core_Future_Deadline, Block_Time + TimeSpanFromHours(i + 1), TimeSpanFromHours(i));
	}

	// endregion
}}
