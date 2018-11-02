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

#define TEST_CLASS PropertyTypeValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(PropertyType,)

	namespace {
		void AssertValidationResult(ValidationResult expectedResult, model::PropertyType propertyType) {
			// Arrange:
			model::PropertyTypeNotification notification(propertyType);
			auto pValidator = CreatePropertyTypeValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "notification with property type " << utils::to_underlying_type(propertyType);
		}

		void AssertValidTypes(std::initializer_list<model::PropertyType> propertyTypes) {
			for (auto propertyType : propertyTypes) {
				AssertValidationResult(ValidationResult::Success, propertyType);
				AssertValidationResult(ValidationResult::Success, propertyType | model::PropertyType::Block);
			}
		}
	}

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithKnownPropertyType) {
		// Assert:
		AssertValidTypes({ model::PropertyType::Address, model::PropertyType::MosaicId });
	}

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithUnknownPropertyType) {
		// Assert:
		AssertValidationResult(Failure_Property_Invalid_Property_Type, model::PropertyType::Sentinel);
		AssertValidationResult(Failure_Property_Invalid_Property_Type, model::PropertyType(0x10));
	}

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithNoFlagsSet) {
		// Assert:
		AssertValidationResult(Failure_Property_Invalid_Property_Type, model::PropertyType(0));
	}

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithMultipleFlagsSet) {
		// Assert:
		AssertValidationResult(Failure_Property_Invalid_Property_Type, model::PropertyType(3));
		AssertValidationResult(Failure_Property_Invalid_Property_Type, model::PropertyType(7));
		AssertValidationResult(Failure_Property_Invalid_Property_Type, model::PropertyType(0xFF));
	}
}}
