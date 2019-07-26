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

#define TEST_CLASS MosaicSupplyChangeValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MosaicSupplyChange,)

	namespace {
		void AssertValidationResult(ValidationResult expectedResult, model::MosaicSupplyChangeDirection direction, Amount delta) {
			// Arrange:
			model::MosaicSupplyChangeNotification notification(Key(), UnresolvedMosaicId(), direction, delta);
			auto pValidator = CreateMosaicSupplyChangeValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "direction " << utils::to_underlying_type(direction) << ", delta " << delta;
		}
	}

	// region direction

	namespace {
		constexpr auto ToDirection(int32_t direction) {
			return static_cast<model::MosaicSupplyChangeDirection>(direction);
		}
	}

	TEST(TEST_CLASS, SuccessWhenValidatingValidDirection) {
		for (auto direction : { 0x00, 0x01 })
			AssertValidationResult(ValidationResult::Success, ToDirection(direction), Amount(123));
	}

	TEST(TEST_CLASS, FailureWhenValidatingInvalidDirection) {
		for (auto direction : { 0x02, 0xFF })
			AssertValidationResult(Failure_Mosaic_Invalid_Supply_Change_Direction, ToDirection(direction), Amount(123));
	}

	// endregion

	// region amount

	TEST(TEST_CLASS, SuccessWhenDeltaIsNonzero) {
		for (auto direction : { 0x00, 0x01 })
			AssertValidationResult(ValidationResult::Success, ToDirection(direction), Amount(1));
	}

	TEST(TEST_CLASS, FailureWhenDeltaIsZero) {
		for (auto direction : { 0x00, 0x01 })
			AssertValidationResult(Failure_Mosaic_Invalid_Supply_Change_Amount, ToDirection(direction), Amount());
	}

	// endregion
}}
