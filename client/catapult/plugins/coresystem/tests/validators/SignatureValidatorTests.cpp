#include "src/validators/Validators.h"
#include "catapult/crypto/Signer.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

	DEFINE_COMMON_VALIDATOR_TESTS(Signature,)

#define TEST_CLASS SignatureValidatorTests

	namespace {
		void AssertValidationResult(const model::SignatureNotification& notification, ValidationResult expectedResult) {
			// Arrange:
			auto pValidator = CreateSignatureValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	TEST(TEST_CLASS, SuccessWhenValidatingValidSignature) {
		// Arrange:
		auto signer = test::GenerateKeyPair();
		auto data = test::GenerateRandomVector(55);
		Signature signature;
		crypto::Sign(signer, data, signature);

		model::SignatureNotification notification(signer.publicKey(), signature, data);

		// Assert:
		AssertValidationResult(notification, ValidationResult::Success);
	}

	TEST(TEST_CLASS, FailureWhenSignatureIsAltered) {
		// Arrange:
		auto signer = test::GenerateKeyPair();
		auto data = test::GenerateRandomVector(55);
		Signature signature;
		crypto::Sign(signer, data, signature);

		model::SignatureNotification notification(signer.publicKey(), signature, data);
		signature[0] ^= 0xFF;

		// Assert:
		AssertValidationResult(notification, Failure_Core_Signature_Not_Verifiable);
	}

	TEST(TEST_CLASS, FailureWhenDataIsAltered) {
		// Arrange:
		auto signer = test::GenerateKeyPair();
		auto data = test::GenerateRandomVector(55);
		Signature signature;
		crypto::Sign(signer, data, signature);

		model::SignatureNotification notification(signer.publicKey(), signature, data);
		data[10] ^= 0xFF;

		// Assert:
		AssertValidationResult(notification, Failure_Core_Signature_Not_Verifiable);
	}
}}
