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
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS AccountRestrictionTypeValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(AccountRestrictionType,)

	namespace {
		void AssertValidationResult(ValidationResult expectedResult, model::AccountRestrictionType restrictionType) {
			// Arrange:
			model::AccountRestrictionTypeNotification notification(restrictionType);
			auto pValidator = CreateAccountRestrictionTypeValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "notification with restriction type " << utils::to_underlying_type(restrictionType);
		}

		void AssertValidTypes(std::initializer_list<model::AccountRestrictionType> restrictionTypes) {
			for (auto restrictionType : restrictionTypes) {
				AssertValidationResult(ValidationResult::Success, restrictionType);
				AssertValidationResult(ValidationResult::Success, restrictionType | model::AccountRestrictionType::Block);
			}
		}
	}

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithKnownAccountRestrictionType) {
		// Assert:
		AssertValidTypes({ model::AccountRestrictionType::Address, model::AccountRestrictionType::MosaicId });
	}

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithUnknownAccountRestrictionType) {
		// Assert:
		AssertValidationResult(Failure_RestrictionAccount_Invalid_Restriction_Type, model::AccountRestrictionType::Sentinel);
		AssertValidationResult(Failure_RestrictionAccount_Invalid_Restriction_Type, static_cast<model::AccountRestrictionType>(0x10));
	}

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithNoFlagsSet) {
		// Assert:
		AssertValidationResult(Failure_RestrictionAccount_Invalid_Restriction_Type, static_cast<model::AccountRestrictionType>(0));
	}

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithMultipleFlagsSet) {
		// Assert:
		AssertValidationResult(Failure_RestrictionAccount_Invalid_Restriction_Type, static_cast<model::AccountRestrictionType>(3));
		AssertValidationResult(Failure_RestrictionAccount_Invalid_Restriction_Type, static_cast<model::AccountRestrictionType>(7));
		AssertValidationResult(Failure_RestrictionAccount_Invalid_Restriction_Type, static_cast<model::AccountRestrictionType>(0xFF));
	}
}}
