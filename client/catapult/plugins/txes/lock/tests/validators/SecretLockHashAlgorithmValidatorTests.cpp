#include "src/validators/Validators.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS SecretLockHashAlgorithmValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(SecretLockHashAlgorithm,)

	namespace {
		void AssertValidationResult(ValidationResult expectedResult, model::LockHashAlgorithm hashAlgorithm) {
			// Arrange:
			model::SecretLockHashAlgorithmNotification notification(hashAlgorithm);
			auto pValidator = CreateSecretLockHashAlgorithmValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "notification with algorithm " << utils::to_underlying_type(hashAlgorithm);
		}

		void AssertValidAlgorithms(std::initializer_list<model::LockHashAlgorithm> hashAlgorithms) {
			for (auto hashAlgorithm : hashAlgorithms)
				AssertValidationResult(ValidationResult::Success, hashAlgorithm);
		}
	}

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithKnownAlgorithm) {
		using model::LockHashAlgorithm;

		// Assert:
		AssertValidAlgorithms({
			LockHashAlgorithm::Op_Sha3,
			LockHashAlgorithm::Op_Keccak,
			LockHashAlgorithm::Op_Hash_160,
			LockHashAlgorithm::Op_Hash_256
		});
	}

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithUnknownAlgorithm) {
		// Assert:
		AssertValidationResult(Failure_Lock_Invalid_Hash_Algorithm, static_cast<model::LockHashAlgorithm>(0xFF));
	}
}}
