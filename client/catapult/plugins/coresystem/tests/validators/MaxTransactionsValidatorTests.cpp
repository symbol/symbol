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
#include "tests/test/core/NotificationTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS MaxTransactionsValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MaxTransactions, 123)

	namespace {
		constexpr uint32_t Max_Transactions = 10;

		void AssertValidationResult(ValidationResult expectedResult, uint32_t numTransactions) {
			// Arrange:
			auto harvester = test::GenerateRandomByteArray<Address>();
			auto notification = test::CreateBlockNotification(harvester);
			notification.NumTransactions = numTransactions;
			auto pValidator = CreateMaxTransactionsValidator(Max_Transactions);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	// region validation

	TEST(TEST_CLASS, SuccessWhenBlockContainsNoTransactions) {
		AssertValidationResult(ValidationResult::Success, 0);
	}

	TEST(TEST_CLASS, SuccessWhenBlockContainsLessThanMaxTransactions) {
		constexpr auto expectedResult = ValidationResult::Success;
		AssertValidationResult(expectedResult, 1);
		AssertValidationResult(expectedResult, 5);
		AssertValidationResult(expectedResult, Max_Transactions - 1);
	}

	TEST(TEST_CLASS, SuccessWhenBlockContainsMaxTransactions) {
		AssertValidationResult(ValidationResult::Success, Max_Transactions);
	}

	TEST(TEST_CLASS, FailureWhenBlockContainsMoreThanMaxTransactions) {
		constexpr auto expectedResult = Failure_Core_Too_Many_Transactions;
		AssertValidationResult(expectedResult, Max_Transactions + 1);
		AssertValidationResult(expectedResult, Max_Transactions + 10);
		AssertValidationResult(expectedResult, Max_Transactions + 100);
	}

	// endregion
}}
