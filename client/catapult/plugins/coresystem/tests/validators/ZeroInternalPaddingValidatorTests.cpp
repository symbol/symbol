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

#define TEST_CLASS ZeroInternalPaddingValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(ZeroInternalPadding,)

	namespace {
		void AssertValidationResult(ValidationResult expectedResult, uint64_t padding) {
			// Arrange:
			model::InternalPaddingNotification notification(padding);
			auto pValidator = CreateZeroInternalPaddingValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "padding = " << padding;
		}
	}

	TEST(TEST_CLASS, SuccessWhenPaddingIsZero) {
		AssertValidationResult(ValidationResult::Success, 0);
	}

	TEST(TEST_CLASS, FailureWhenPaddingIsNonzero) {
		AssertValidationResult(Failure_Core_Nonzero_Internal_Padding, 1);
		AssertValidationResult(Failure_Core_Nonzero_Internal_Padding, 987);
		AssertValidationResult(Failure_Core_Nonzero_Internal_Padding, static_cast<uint64_t>(1) << 32);
	}
}}
