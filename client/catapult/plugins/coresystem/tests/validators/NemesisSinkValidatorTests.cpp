#include "src/validators/Validators.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/validators/ValidatorContext.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

	DEFINE_COMMON_VALIDATOR_TESTS(NemesisSink,)

#define TEST_CLASS NemesisSinkValidatorTests

	namespace {
		constexpr ValidationResult Success_Result = ValidationResult::Success;
		constexpr ValidationResult Failure_Result = Failure_Core_Nemesis_Account_Signed_After_Nemesis_Block;

		crypto::KeyPair GetNemesisAccount() {
			// note that the nemesis account is fake in order to ensure that it is being retrieved from the context
			return crypto::KeyPair::FromString("0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF");
		}

		void AssertValidationResult(const Key& signer, Height::ValueType height, ValidationResult expectedResult) {
			// Arrange:
			auto cache = test::CreateEmptyCatapultCache();
			auto cacheView = cache.createView();
			auto readOnlyCache = cacheView.toReadOnly();
			model::NetworkInfo networkInfo(model::NetworkIdentifier::Zero, GetNemesisAccount().publicKey(), {});
			auto pValidator = CreateNemesisSinkValidator();
			auto context = test::CreateValidatorContext(Height(height), networkInfo, readOnlyCache);

			auto signature = test::GenerateRandomData<Signature_Size>();
			model::SignatureNotification notification(signer, signature, {});

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, context);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	TEST(TEST_CLASS, SuccessWhenValidatingEntityNotSignedByTheNemesisAccount) {
		// Arrange:
		auto signer = test::GenerateRandomData<Key_Size>();

		// Sanity:
		EXPECT_NE(test::ToHexString(GetNemesisAccount().publicKey()), test::ToHexString(signer));

		// Assert: signer is allowed at all heights
		AssertValidationResult(signer, 1, Success_Result);
		AssertValidationResult(signer, 10, Success_Result);
		AssertValidationResult(signer, 100, Success_Result);
	}

	TEST(TEST_CLASS, SuccessWhenValidatingEntityAtHeightOneSignedByNemesisAccount) {
		// Arrange:
		auto signer = GetNemesisAccount().publicKey();

		// Sanity:
		EXPECT_EQ(test::ToHexString(GetNemesisAccount().publicKey()), test::ToHexString(signer));

		// Assert: allowed at height one
		AssertValidationResult(signer, 1, Success_Result);
	}

	TEST(TEST_CLASS, FailureWhenValidatingEntityNotAtHeightOneSignedByNemesisAccount) {
		// Arrange:
		auto signer = GetNemesisAccount().publicKey();

		// Sanity:
		EXPECT_EQ(test::ToHexString(GetNemesisAccount().publicKey()), test::ToHexString(signer));

		// Assert: not allowed at heights greater than one
		AssertValidationResult(signer, 2, Failure_Result);
		AssertValidationResult(signer, 10, Failure_Result);
		AssertValidationResult(signer, 100, Failure_Result);
	}
}}
