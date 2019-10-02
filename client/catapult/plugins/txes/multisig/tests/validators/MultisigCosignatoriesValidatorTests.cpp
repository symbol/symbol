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

#define TEST_CLASS MultisigCosignatoriesValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MultisigCosignatories,)

	namespace {
		constexpr auto Add = model::CosignatoryModificationAction::Add;
		constexpr auto Del = model::CosignatoryModificationAction::Del;

		void AssertValidationResult(ValidationResult expectedResult, const std::vector<model::CosignatoryModification>& modifications) {
			// Arrange:
			auto signer = test::GenerateRandomByteArray<Key>();
			model::MultisigCosignatoriesNotification notification(
					signer,
					static_cast<uint8_t>(modifications.size()), modifications.data());
			auto pValidator = CreateMultisigCosignatoriesValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	TEST(TEST_CLASS, SuccessWhenZeroModificationsArePresent) {
		AssertValidationResult(ValidationResult::Success, {});
	}

	TEST(TEST_CLASS, SuccessWhenSingleAddModificationIsPresent) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();

		// Assert:
		AssertValidationResult(ValidationResult::Success, { { Add, key } });
	}

	TEST(TEST_CLASS, SuccessWhenSingleDelModificationIsPresent) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();

		// Assert:
		AssertValidationResult(ValidationResult::Success, { { Del, key } });
	}

	TEST(TEST_CLASS, FailureWhenSingleUnsupportedModificationIsPresent) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();

		// Assert:
		constexpr auto expectedResult = Failure_Multisig_Invalid_Modification_Action;
		for (auto action : { 2, 3, 0xFF }) {
			CATAPULT_LOG(debug) << "validating modification with action " << action;
			AssertValidationResult(expectedResult, { { static_cast<model::CosignatoryModificationAction>(action), key } });
		}
	}

	namespace {
		using ModificationActions = std::vector<model::CosignatoryModificationAction>;

		void AssertResultWhenDifferentAccountsUsed(ValidationResult expectedResult, const ModificationActions& modificationActions) {
			// Arrange:
			std::vector<model::CosignatoryModification> modifications;

			for (auto modificationAction : modificationActions) {
				auto key = test::GenerateRandomByteArray<Key>();
				modifications.push_back({ modificationAction, key });
			}

			// Assert:
			AssertValidationResult(expectedResult, modifications);
		}
	}

	TEST(TEST_CLASS, SuccessWhenMultipleDifferentAccountsAreAdded) {
		AssertResultWhenDifferentAccountsUsed(ValidationResult::Success, { Add, Add, Add });
		AssertResultWhenDifferentAccountsUsed(ValidationResult::Success, { Add, Add, Del });
		AssertResultWhenDifferentAccountsUsed(ValidationResult::Success, { Add, Del, Add });
	}

	TEST(TEST_CLASS, FailureWhenMultipleDifferentAccountsAreDeleted) {
		constexpr auto expectedResult = Failure_Multisig_Multiple_Deletes;
		AssertResultWhenDifferentAccountsUsed(expectedResult, { Del, Del, Add });
		AssertResultWhenDifferentAccountsUsed(expectedResult, { Del, Add, Del });
		AssertResultWhenDifferentAccountsUsed(expectedResult, { Add, Del, Del });
		AssertResultWhenDifferentAccountsUsed(expectedResult, { Del, Del, Del });
	}

	TEST(TEST_CLASS, FailureWhenAnyAccountHasUnsupportedModification) {
		constexpr auto expectedResult = Failure_Multisig_Invalid_Modification_Action;
		for (auto action : { 2, 3, 0xFF }) {
			CATAPULT_LOG(debug) << "validating modification with action " << action;
			AssertResultWhenDifferentAccountsUsed(expectedResult, { Add, static_cast<model::CosignatoryModificationAction>(action), Add });
		}
	}

	namespace {
		void AssertResultWhenSameAccountUsed(ValidationResult expectedResult, ModificationActions modificationActions) {
			// Arrange:
			auto key = test::GenerateRandomByteArray<Key>();
			std::vector<model::CosignatoryModification> modifications;

			for (auto modificationAction : modificationActions)
				modifications.push_back({ modificationAction, key });

			// Assert:
			AssertValidationResult(expectedResult, modifications);
		}
	}

	TEST(TEST_CLASS, FailureWhenSameAccountIsAddedAndDeleted) {
		constexpr auto expectedResult = Failure_Multisig_Account_In_Both_Sets;
		AssertResultWhenSameAccountUsed(expectedResult, { Add, Del });
		AssertResultWhenSameAccountUsed(expectedResult, { Del, Add });
	}

	TEST(TEST_CLASS, FailureWhenSameAccountIsAddedMultipleTimes) {
		constexpr auto expectedResult = Failure_Multisig_Redundant_Modification;
		AssertResultWhenSameAccountUsed(expectedResult, { Add, Add });
		AssertResultWhenSameAccountUsed(expectedResult, { Add, Add, Add });
	}

	TEST(TEST_CLASS, FailureWhenSameAccountIsDeletedMultipleTimes) {
		constexpr auto expectedResult = Failure_Multisig_Redundant_Modification;
		AssertResultWhenSameAccountUsed(expectedResult, { Del, Del });
		AssertResultWhenSameAccountUsed(expectedResult, { Del, Del, Del });
	}
}}
