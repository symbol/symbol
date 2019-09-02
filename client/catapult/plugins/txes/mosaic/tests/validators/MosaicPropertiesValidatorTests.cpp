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
#include "catapult/constants.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS MosaicPropertiesValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MosaicProperties, 0, BlockDuration())

	namespace {
		constexpr auto Max_Divisibility = std::numeric_limits<uint8_t>::max();
		constexpr BlockDuration Max_Duration(std::numeric_limits<BlockDuration::ValueType>::max());
	}

	// region flags

	namespace {
		void AssertFlagsResult(ValidationResult expectedResult, model::MosaicFlags flags) {
			// Arrange:
			auto pValidator = CreateMosaicPropertiesValidator(Max_Divisibility, Max_Duration);
			model::MosaicProperties properties(flags, 0, BlockDuration());
			auto notification = model::MosaicPropertiesNotification(properties);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "flags " << static_cast<uint16_t>(flags);
		}
	}

	TEST(TEST_CLASS, SuccessWhenValidatingValidMosaicFlags) {
		for (auto flags : { 0x00, 0x02, 0x05, 0x07 })
			AssertFlagsResult(ValidationResult::Success, static_cast<model::MosaicFlags>(flags));
	}

	TEST(TEST_CLASS, FailureWhenValidatingInvalidMosaicFlags) {
		for (auto flags : { 0x08, 0x09, 0xFF })
			AssertFlagsResult(Failure_Mosaic_Invalid_Flags, static_cast<model::MosaicFlags>(flags));
	}

	// endregion

	// region divisibility

	namespace {
		void AssertDivisibilityValidationResult(ValidationResult expectedResult, uint8_t divisibility, uint8_t maxDivisibility) {
			// Arrange:
			auto pValidator = CreateMosaicPropertiesValidator(maxDivisibility, Max_Duration);
			model::MosaicProperties properties(model::MosaicFlags::None, divisibility, BlockDuration());
			auto notification = model::MosaicPropertiesNotification(properties);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result)
					<< "divisibility " << static_cast<uint32_t>(divisibility)
					<< ", max " << static_cast<uint32_t>(maxDivisibility);
		}
	}

	TEST(TEST_CLASS, SuccessWhenValidatingDivisibilityLessThanMax) {
		AssertDivisibilityValidationResult(ValidationResult::Success, 7, 11);
	}

	TEST(TEST_CLASS, SuccessWhenValidatingDivisibilityEqualToMax) {
		AssertDivisibilityValidationResult(ValidationResult::Success, 11, 11);
	}

	TEST(TEST_CLASS, FailureWhenValidatingDivisibilityGreaterThanMax) {
		AssertDivisibilityValidationResult(Failure_Mosaic_Invalid_Divisibility, 12, 11);
		AssertDivisibilityValidationResult(Failure_Mosaic_Invalid_Divisibility, 111, 11);
	}

	// endregion

	// region duration

	namespace {
		void AssertDurationValidationResult(ValidationResult expectedResult, uint16_t duration, uint16_t maxDuration) {
			// Arrange:
			auto pValidator = CreateMosaicPropertiesValidator(Max_Divisibility, BlockDuration(maxDuration));
			model::MosaicProperties properties(model::MosaicFlags::None, 0, BlockDuration(duration));
			auto notification = model::MosaicPropertiesNotification(properties);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "duration " << duration << ", maxDuration " << maxDuration;
		}
	}

	TEST(TEST_CLASS, SuccessWhenValidatingDurationLessThanMax) {
		AssertDurationValidationResult(ValidationResult::Success, 0, 12345);
		AssertDurationValidationResult(ValidationResult::Success, 12312, 12345);
	}

	TEST(TEST_CLASS, SuccessWhenValidatingDurationEqualToMax) {
		AssertDurationValidationResult(ValidationResult::Success, 12345, 12345);
	}

	TEST(TEST_CLASS, FailureWhenValidatingDurationGreaterThanMax) {
		AssertDurationValidationResult(Failure_Mosaic_Invalid_Duration, 12346, 12345);
		AssertDurationValidationResult(Failure_Mosaic_Invalid_Duration, 65432, 12345);
	}

	// endregion
}}
