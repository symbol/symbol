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

#define TEST_CLASS ModifyMultisigCosignersValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(ModifyMultisigCosigners,)

	namespace {
		constexpr auto Add = model::CosignatoryModificationType::Add;
		constexpr auto Del = model::CosignatoryModificationType::Del;

		void AssertValidationResult(ValidationResult expectedResult, const std::vector<model::CosignatoryModification>& modifications) {
			// Arrange:
			auto signer = test::GenerateRandomByteArray<Key>();
			model::ModifyMultisigCosignersNotification notification(
					signer,
					static_cast<uint8_t>(modifications.size()),
					modifications.data());
			auto pValidator = CreateModifyMultisigCosignersValidator();

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
		constexpr auto expectedResult = Failure_Multisig_Modify_Unsupported_Modification_Type;
		for (auto type : { 2, 3, 0xFF }) {
			CATAPULT_LOG(debug) << "validating modification with type " << type;
			AssertValidationResult(expectedResult, { { static_cast<model::CosignatoryModificationType>(type), key } });
		}
	}

	namespace {
		using ModificationTypes = std::vector<model::CosignatoryModificationType>;

		void AssertResultWhenDifferentAccountsUsed(ValidationResult expectedResult, const ModificationTypes& modificationTypes) {
			// Arrange:
			std::vector<model::CosignatoryModification> modifications;

			for (auto modificationType : modificationTypes) {
				auto key = test::GenerateRandomByteArray<Key>();
				modifications.push_back({ modificationType, key });
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
		constexpr auto expectedResult = Failure_Multisig_Modify_Multiple_Deletes;
		AssertResultWhenDifferentAccountsUsed(expectedResult, { Del, Del, Add });
		AssertResultWhenDifferentAccountsUsed(expectedResult, { Del, Add, Del });
		AssertResultWhenDifferentAccountsUsed(expectedResult, { Add, Del, Del });
		AssertResultWhenDifferentAccountsUsed(expectedResult, { Del, Del, Del });
	}

	TEST(TEST_CLASS, FailureWhenAnyAccountHasUnsupportedModification) {
		constexpr auto expectedResult = Failure_Multisig_Modify_Unsupported_Modification_Type;
		for (auto type : { 2, 3, 0xFF }) {
			CATAPULT_LOG(debug) << "validating modification with type " << type;
			AssertResultWhenDifferentAccountsUsed(expectedResult, { Add, static_cast<model::CosignatoryModificationType>(type), Add });
		}
	}

	namespace {
		void AssertResultWhenSameAccountUsed(ValidationResult expectedResult, ModificationTypes modificationTypes) {
			// Arrange:
			auto key = test::GenerateRandomByteArray<Key>();
			std::vector<model::CosignatoryModification> modifications;

			for (auto modificationType : modificationTypes)
				modifications.push_back({ modificationType, key });

			// Assert:
			AssertValidationResult(expectedResult, modifications);
		}
	}

	TEST(TEST_CLASS, FailureWhenSameAccountIsAddedAndDeleted) {
		constexpr auto expectedResult = Failure_Multisig_Modify_Account_In_Both_Sets;
		AssertResultWhenSameAccountUsed(expectedResult, { Add, Del });
		AssertResultWhenSameAccountUsed(expectedResult, { Del, Add });
	}

	TEST(TEST_CLASS, FailureWhenSameAccountIsAddedMultipleTimes) {
		constexpr auto expectedResult = Failure_Multisig_Modify_Redundant_Modifications;
		AssertResultWhenSameAccountUsed(expectedResult, { Add, Add });
		AssertResultWhenSameAccountUsed(expectedResult, { Add, Add, Add });
	}

	TEST(TEST_CLASS, FailureWhenSameAccountIsDeletedMultipleTimes) {
		constexpr auto expectedResult = Failure_Multisig_Modify_Redundant_Modifications;
		AssertResultWhenSameAccountUsed(expectedResult, { Del, Del });
		AssertResultWhenSameAccountUsed(expectedResult, { Del, Del, Del });
	}
}}
