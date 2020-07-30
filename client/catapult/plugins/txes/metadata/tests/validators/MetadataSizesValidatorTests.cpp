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

#define TEST_CLASS MetadataSizesValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MetadataSizes, 0)

	namespace {
		void AssertValidationResult(ValidationResult expectedResult, int16_t valueSizeDelta, uint16_t valueSize, uint16_t maxValueSize) {
			// Arrange:
			auto notification = model::MetadataSizesNotification(valueSizeDelta, valueSize);
			auto pValidator = CreateMetadataSizesValidator(maxValueSize);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "valueSizeDelta = " << valueSizeDelta << ", valueSize = " << valueSize;
		}
	}

	// region value size too small

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithZeroValueSizeDeltaButNonzeroValueSize) {
		AssertValidationResult(ValidationResult::Success, 0, 1, 1234);
	}

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithNonzeroValueSizeDeltaButZeroValueSize) {
		for (auto valueSizeDelta : std::initializer_list<int16_t>{ -1, 1 })
			AssertValidationResult(Failure_Metadata_Value_Too_Small, valueSizeDelta, 0, 1234);
	}

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithBothZeroValueSizeDeltaAndZeroValueSize) {
		AssertValidationResult(Failure_Metadata_Value_Too_Small, 0, 0, 1234);
	}

	// endregion

	// region value size delta magnitude too large

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithValueSizeDeltaMagnitudeLessThanValueSize) {
		for (auto multiplier : std::initializer_list<int16_t>{ -1, 1 })
			AssertValidationResult(ValidationResult::Success, static_cast<int16_t>(multiplier * 99), 100, 1234);
	}

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithValueSizeDeltaMagnitudeEqualToValueSize) {
		for (auto multiplier : std::initializer_list<int16_t>{ -1, 1 })
			AssertValidationResult(ValidationResult::Success, static_cast<int16_t>(multiplier * 100), 100, 1234);
	}

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithValueSizeDeltaMagnitudeGreaterThanValueSize) {
		for (auto multiplier : std::initializer_list<int16_t>{ -1, 1 })
			AssertValidationResult(Failure_Metadata_Value_Size_Delta_Too_Large, static_cast<int16_t>(multiplier * 101), 100, 1234);

		AssertValidationResult(Failure_Metadata_Value_Size_Delta_Too_Large, std::numeric_limits<int16_t>::min(), 100, 1234);
	}

	// endregion

	// region value size too large

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithValueSizeLessThanMax) {
		AssertValidationResult(ValidationResult::Success, 1, 100, 1234);
	}

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithValueSizeEqualToMax) {
		AssertValidationResult(ValidationResult::Success, 1, 1234, 1234);
	}

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithValueSizeGreaterThanMax) {
		AssertValidationResult(Failure_Metadata_Value_Too_Large, 1, 1235, 1234);
	}

	// endregion
}}
