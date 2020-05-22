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
#include "tests/test/AccountRestrictionCacheTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS AccountOperationRestrictionModificationValuesValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(AccountOperationRestrictionModificationValues,)

	namespace {
		struct AccountOperationRestrictionTraits : public test::BaseAccountOperationRestrictionTraits {
			using NotificationType = model::ModifyAccountOperationRestrictionsNotification;
		};

		void AssertValidationResult(ValidationResult expectedResult, const std::vector<uint16_t>& rawValues) {
			// Arrange:
			std::vector<model::EntityType> entityTypeAdditions;
			std::vector<model::EntityType> entityTypeDeletions;
			for (auto rawValue : rawValues)
				entityTypeAdditions.push_back(static_cast<model::EntityType>(rawValue));

			auto notification = test::CreateAccountRestrictionsNotification<AccountOperationRestrictionTraits>(
					test::GenerateRandomByteArray<Address>(),
					entityTypeAdditions,
					entityTypeDeletions);
			auto pValidator = CreateAccountOperationRestrictionModificationValuesValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	// region failure

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithSingleInvalidAccountRestrictionModificationValue) {
		AssertValidationResult(Failure_RestrictionAccount_Invalid_Value, { 0x8000 });
		AssertValidationResult(Failure_RestrictionAccount_Invalid_Value, { 0x2001 });
		AssertValidationResult(Failure_RestrictionAccount_Invalid_Value, { 0x0049 });
	}

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithValidAndInvalidAccountRestrictionModificationValue) {
		AssertValidationResult(Failure_RestrictionAccount_Invalid_Value, { 0x4000, 0x8000, 0x4149 });
		AssertValidationResult(Failure_RestrictionAccount_Invalid_Value, { 0x2001, 0x4149, 0x4123 });
		AssertValidationResult(Failure_RestrictionAccount_Invalid_Value, { 0x4149, 0x0000, 0x4100 });
	}

	// endregion

	// region success

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithNoAccountRestrictionModification) {
		AssertValidationResult(ValidationResult::Success, {});
	}

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithSingleValidAccountRestrictionModificationValue) {
		AssertValidationResult(ValidationResult::Success, { 0x4000 });
		AssertValidationResult(ValidationResult::Success, { 0x4001 });
		AssertValidationResult(ValidationResult::Success, { 0x4149 });
	}

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithMultipleValidAccountRestrictionModificationValues) {
		AssertValidationResult(ValidationResult::Success, { 0x4000, 0x4001, 0x4149 });
	}

	// endregion
}}
