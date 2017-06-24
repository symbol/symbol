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
			auto signer = test::GenerateRandomData<Key_Size>();
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
		// Assert:
		AssertValidationResult(ValidationResult::Success, {});
	}

	TEST(TEST_CLASS, SuccessWhenSingleAddModificationIsPresent) {
		// Arrange:
		auto key = test::GenerateRandomData<Key_Size>();

		// Assert:
		AssertValidationResult(ValidationResult::Success, { { Add, key } });
	}

	TEST(TEST_CLASS, SuccessWhenSingleDelModificationIsPresent) {
		// Arrange:
		auto key = test::GenerateRandomData<Key_Size>();

		// Assert:
		AssertValidationResult(ValidationResult::Success, { { Del, key } });
	}

	TEST(TEST_CLASS, FailureWhenSingleUnsupportedModificationIsPresent) {
		// Arrange:
		auto key = test::GenerateRandomData<Key_Size>();

		// Assert:
		constexpr auto expectedResult = Failure_Multisig_Modify_Unsupported_Modification_Type;
		for (auto type : { 0, 3, 0xFF }) {
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
				auto key = test::GenerateRandomData<Key_Size>();
				modifications.push_back({ modificationType, key });
			}

			// Assert:
			AssertValidationResult(expectedResult, modifications);
		}
	}

	TEST(TEST_CLASS, SuccessWhenMultipleDifferentAccountsAreAdded) {
		// Assert:
		AssertResultWhenDifferentAccountsUsed(ValidationResult::Success, { Add, Add, Add });
		AssertResultWhenDifferentAccountsUsed(ValidationResult::Success, { Add, Add, Del });
		AssertResultWhenDifferentAccountsUsed(ValidationResult::Success, { Add, Del, Add });
	}

	TEST(TEST_CLASS, FailureWhenMultipleDifferentAccountsAreDeleted) {
		// Assert:
		constexpr auto expectedResult = Failure_Multisig_Modify_Multiple_Deletes;
		AssertResultWhenDifferentAccountsUsed(expectedResult, { Del, Del, Add });
		AssertResultWhenDifferentAccountsUsed(expectedResult, { Del, Add, Del });
		AssertResultWhenDifferentAccountsUsed(expectedResult, { Add, Del, Del });
		AssertResultWhenDifferentAccountsUsed(expectedResult, { Del, Del, Del });
	}

	TEST(TEST_CLASS, FailureWhenAnyAccountHasUnsupportedModification) {
		// Assert:
		constexpr auto expectedResult = Failure_Multisig_Modify_Unsupported_Modification_Type;
		for (auto type : { 0, 3, 0xFF }) {
			CATAPULT_LOG(debug) << "validating modification with type " << type;
			AssertResultWhenDifferentAccountsUsed(expectedResult, { Add, static_cast<model::CosignatoryModificationType>(type), Add });
		}
	}

	namespace {
		void AssertResultWhenSameAccountUsed(ValidationResult expectedResult, ModificationTypes modificationTypes) {
			// Arrange:
			auto key = test::GenerateRandomData<Key_Size>();
			std::vector<model::CosignatoryModification> modifications;

			for (auto modificationType : modificationTypes)
				modifications.push_back({ modificationType, key });

			// Assert:
			AssertValidationResult(expectedResult, modifications);
		}
	}

	TEST(TEST_CLASS, FailureWhenSameAccountIsAddedAndDeleted) {
		// Assert:
		constexpr auto expectedResult = Failure_Multisig_Modify_Account_In_Both_Sets;
		AssertResultWhenSameAccountUsed(expectedResult, { Add, Del });
		AssertResultWhenSameAccountUsed(expectedResult, { Del, Add });
	}

	TEST(TEST_CLASS, FailureWhenSameAccountIsAddedMultipleTimes) {
		// Assert:
		constexpr auto expectedResult = Failure_Multisig_Modify_Redundant_Modifications;
		AssertResultWhenSameAccountUsed(expectedResult, { Add, Add });
		AssertResultWhenSameAccountUsed(expectedResult, { Add, Add, Add });
	}

	TEST(TEST_CLASS, FailureWhenSameAccountIsDeletedMultipleTimes) {
		// Assert:
		constexpr auto expectedResult = Failure_Multisig_Modify_Redundant_Modifications;
		AssertResultWhenSameAccountUsed(expectedResult, { Del, Del });
		AssertResultWhenSameAccountUsed(expectedResult, { Del, Del, Del });
	}
}}
