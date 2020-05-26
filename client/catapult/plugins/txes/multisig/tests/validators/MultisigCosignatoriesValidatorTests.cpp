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
#include "tests/test/MultisigTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS MultisigCosignatoriesValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MultisigCosignatories,)

	namespace {
		void AssertValidationResult(
				ValidationResult expectedResult,
				const std::vector<UnresolvedAddress>& addressAdditions,
				const std::vector<UnresolvedAddress>& addressDeletions) {
			// Arrange:
			auto multisig = test::GenerateRandomByteArray<Address>();
			auto notification = test::CreateMultisigCosignatoriesNotification(multisig, addressAdditions, addressDeletions);
			auto pValidator = CreateMultisigCosignatoriesValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	TEST(TEST_CLASS, SuccessWhenZeroModificationsArePresent) {
		AssertValidationResult(ValidationResult::Success, {}, {});
	}

	TEST(TEST_CLASS, SuccessWhenSingleAddModificationIsPresent) {
		// Arrange:
		auto address = test::GenerateRandomByteArray<UnresolvedAddress>();

		// Assert:
		AssertValidationResult(ValidationResult::Success, { address }, {});
	}

	TEST(TEST_CLASS, SuccessWhenSingleDelModificationIsPresent) {
		// Arrange:
		auto address = test::GenerateRandomByteArray<UnresolvedAddress>();

		// Assert:
		AssertValidationResult(ValidationResult::Success, {}, { address });
	}

	namespace {
		void AssertResultWhenDifferentAccountsUsed(ValidationResult expectedResult, uint8_t numAdditions, uint8_t numDeletions) {
			AssertValidationResult(
					expectedResult,
					test::GenerateRandomDataVector<UnresolvedAddress>(numAdditions),
					test::GenerateRandomDataVector<UnresolvedAddress>(numDeletions));
		}
	}

	TEST(TEST_CLASS, SuccessWhenMultipleDifferentAccountsAreAdded) {
		AssertResultWhenDifferentAccountsUsed(ValidationResult::Success, 3, 0);
		AssertResultWhenDifferentAccountsUsed(ValidationResult::Success, 2, 1);
	}

	TEST(TEST_CLASS, FailureWhenMultipleDifferentAccountsAreDeleted) {
		constexpr auto expectedResult = Failure_Multisig_Multiple_Deletes;
		AssertResultWhenDifferentAccountsUsed(expectedResult, 1, 2);
		AssertResultWhenDifferentAccountsUsed(expectedResult, 0, 3);
	}

	namespace {
		void AssertResultWhenSameAccountUsed(ValidationResult expectedResult, uint8_t numAdditions, uint8_t numDeletions) {
			// Arrange:
			auto address = test::GenerateRandomByteArray<UnresolvedAddress>();

			// Assert:
			AssertValidationResult(
					expectedResult,
					std::vector<UnresolvedAddress>(numAdditions, address),
					std::vector<UnresolvedAddress>(numDeletions, address));
		}
	}

	TEST(TEST_CLASS, FailureWhenSameAccountIsAddedAndDeleted) {
		constexpr auto expectedResult = Failure_Multisig_Account_In_Both_Sets;
		AssertResultWhenSameAccountUsed(expectedResult, 1, 1);
	}

	TEST(TEST_CLASS, FailureWhenSameAccountIsAddedMultipleTimes) {
		constexpr auto expectedResult = Failure_Multisig_Redundant_Modification;
		AssertResultWhenSameAccountUsed(expectedResult, 2, 0);
		AssertResultWhenSameAccountUsed(expectedResult, 3, 0);
	}

	TEST(TEST_CLASS, FailureWhenSameAccountIsDeletedMultipleTimes) {
		constexpr auto expectedResult = Failure_Multisig_Multiple_Deletes;
		AssertResultWhenSameAccountUsed(expectedResult, 0, 2);
		AssertResultWhenSameAccountUsed(expectedResult, 0, 3);
	}
}}
