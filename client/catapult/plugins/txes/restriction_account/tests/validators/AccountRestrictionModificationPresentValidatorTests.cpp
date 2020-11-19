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

#include "src/validators/Validators.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS AccountRestrictionModificationPresentValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(AccountRestrictionModificationPresent,)

	namespace {
		void AssertValidationResult(ValidationResult expectedResult, uint8_t numAdditions, uint8_t numDeletions) {
			// Arrange:
			auto restrictionFlags = model::AccountRestrictionFlags::Address;
			model::AccountRestrictionModificationNotification notification(restrictionFlags, numAdditions, numDeletions);
			auto pValidator = CreateAccountRestrictionModificationPresentValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result)
					<< "notification with " << static_cast<uint16_t>(numAdditions) << " addition modifications"
					<< " and " << static_cast<uint16_t>(numDeletions) << " deletion modifications";
		}
	}

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithNoModifications) {
		AssertValidationResult(Failure_RestrictionAccount_No_Modifications, 0, 0);
	}

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithAdditionModifications) {
		AssertValidationResult(ValidationResult::Success, 1, 0);
		AssertValidationResult(ValidationResult::Success, 5, 0);
	}

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithDeletionModifications) {
		AssertValidationResult(ValidationResult::Success, 0, 1);
		AssertValidationResult(ValidationResult::Success, 0, 5);
	}

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithAdditionAndDeletionModifications) {
		AssertValidationResult(ValidationResult::Success, 1, 1);
		AssertValidationResult(ValidationResult::Success, 2, 5);
		AssertValidationResult(ValidationResult::Success, 5, 1);
	}
}}
