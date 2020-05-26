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

#define TEST_CLASS TransactionFeeValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(TransactionFee,)

	namespace {
		void AssertValidationResult(ValidationResult expectedResult, uint32_t transactionSize, Amount fee, Amount maxFee) {
			// Arrange:
			model::TransactionFeeNotification notification(Address(), transactionSize, fee, maxFee);
			auto pValidator = CreateTransactionFeeValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "size = " << transactionSize << ", fee = " << fee << ", max fee = " << maxFee;
		}

		void AssertValidationResult(ValidationResult expectedResult, uint32_t transactionSize, Amount maxFee) {
			AssertValidationResult(expectedResult, transactionSize, maxFee, maxFee);
		}
	}

	// region fee <= max fee

	TEST(TEST_CLASS, SuccessWhenFeeIsLessThanMaxFee) {
		AssertValidationResult(ValidationResult::Success, 200, Amount(0), Amount(234));
		AssertValidationResult(ValidationResult::Success, 300, Amount(123), Amount(234));
		AssertValidationResult(ValidationResult::Success, 400, Amount(233), Amount(234));
	}

	TEST(TEST_CLASS, SuccessWhenFeeIsEqualToMaxFee) {
		AssertValidationResult(ValidationResult::Success, 200, Amount(234), Amount(234));
	}

	TEST(TEST_CLASS, FailureWhenFeeIsGreaterThanMaxFee) {
		AssertValidationResult(Failure_Core_Invalid_Transaction_Fee, 300, Amount(235), Amount(234));
		AssertValidationResult(Failure_Core_Invalid_Transaction_Fee, 400, Amount(1000), Amount(234));
	}

	// endregion

	// region max fee multiplier can't overflow

	TEST(TEST_CLASS, SuccessWhenMaxFeeMultiplierIsLessThanMax) {
		AssertValidationResult(ValidationResult::Success, 200, Amount(200ull * 0xFFFF'FFFE));
		AssertValidationResult(ValidationResult::Success, 300, Amount(300ull * 0xFFFF'FF00));
		AssertValidationResult(ValidationResult::Success, 400, Amount(400ull * 0xFFFF'FFFE));
	}

	TEST(TEST_CLASS, SuccessWhenMaxFeeMultiplierIsEqualToMax) {
		AssertValidationResult(ValidationResult::Success, 200, Amount(200ull * 0xFFFF'FFFF));
		AssertValidationResult(ValidationResult::Success, 300, Amount(300ull * 0xFFFF'FFFF));
		AssertValidationResult(ValidationResult::Success, 400, Amount(400ull * 0xFFFF'FFFF));
	}

	TEST(TEST_CLASS, FailureWhenMaxFeeMultiplierIsGreaterThanMax) {
		AssertValidationResult(Failure_Core_Invalid_Transaction_Fee, 200, Amount(200ull * 0xFFFF'FFFF + 1));
		AssertValidationResult(Failure_Core_Invalid_Transaction_Fee, 300, Amount(300ull * 0x1'0000'FFFF));
		AssertValidationResult(Failure_Core_Invalid_Transaction_Fee, 400, Amount(400ull * 0x1'0000'0000));
	}

	// endregion
}}
