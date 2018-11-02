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
#include "tests/test/PropertyCacheTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS TransactionTypePropertyModificationValuesValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(TransactionTypePropertyModificationValues,)

	namespace {
		std::vector<model::PropertyModification<model::EntityType>> CreateModifications(const std::vector<uint16_t>& rawValues) {
			std::vector<model::PropertyModification<model::EntityType>> modifications;
			for (auto rawValue : rawValues)
				modifications.push_back({ model::PropertyModificationType::Add, static_cast<model::EntityType>(rawValue) });

			return modifications;
		}

		void AssertValidationResult(ValidationResult expectedResult, const std::vector<uint16_t>& rawValues) {
			// Arrange:
			auto modifications = CreateModifications(rawValues);
			model::ModifyTransactionTypePropertyNotification notification(
					test::GenerateRandomData<Key_Size>(),
					model::PropertyType::TransactionType,
					static_cast<uint8_t>(modifications.size()),
					modifications.data());
			auto pValidator = CreateTransactionTypePropertyModificationValuesValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	// region failure

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithSingleInvalidPropertyModificationValue) {
		// Assert:
		AssertValidationResult(Failure_Property_Value_Invalid, { 0x8000 });
		AssertValidationResult(Failure_Property_Value_Invalid, { 0x2001 });
		AssertValidationResult(Failure_Property_Value_Invalid, { 0x0049 });
	}

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithValidAndInvalidPropertyModificationValue) {
		// Assert:
		AssertValidationResult(Failure_Property_Value_Invalid, { 0x4000, 0x8000, 0x4149 });
		AssertValidationResult(Failure_Property_Value_Invalid, { 0x2001, 0x4149, 0x4123 });
		AssertValidationResult(Failure_Property_Value_Invalid, { 0x4149, 0x0000, 0x4100 });
	}

	// endregion

	// region success

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithNoPropertyModification) {
		// Assert:
		AssertValidationResult(ValidationResult::Success, {});
	}

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithSingleValidPropertyModificationValue) {
		// Assert:
		AssertValidationResult(ValidationResult::Success, { 0x4000 });
		AssertValidationResult(ValidationResult::Success, { 0x4001 });
		AssertValidationResult(ValidationResult::Success, { 0x4149 });
	}

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithMultipleValidPropertyModificationValues) {
		// Assert:
		AssertValidationResult(ValidationResult::Success, { 0x4000, 0x4001, 0x4149 });
	}

	// endregion
}}
