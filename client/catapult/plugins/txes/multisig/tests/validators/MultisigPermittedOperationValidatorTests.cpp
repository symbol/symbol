#include "src/validators/Validators.h"
#include "tests/test/MultisigCacheTestUtils.h"
#include "tests/test/MultisigTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS MultisigPermittedOperationValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MultisigPermittedOperation,)

	namespace {
		void AssertValidationResult(ValidationResult expectedResult, const cache::CatapultCache& cache, const Key& signer) {
			// Arrange:
			// - create the validator context
			auto cacheView = cache.createView();
			auto readOnlyCache = cacheView.toReadOnly();
			auto context = test::CreateValidatorContext(Height(), readOnlyCache);

			model::TransactionNotification notification(signer, Hash256(), model::EntityType(), Timestamp());
			auto pValidator = CreateMultisigPermittedOperationValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, context);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}

		auto CreateCacheWithSingleLevelMultisig(const Key& embeddedSigner, const std::vector<Key>& cosignatories) {
			auto cache = test::MultisigCacheFactory::Create();
			auto cacheDelta = cache.createDelta();

			test::MakeMultisig(cacheDelta, embeddedSigner, cosignatories, 3, 4); // make a 3-4-X multisig

			cache.commit(Height());
			return cache;
		}
	}

	TEST(TEST_CLASS, NonMultisigAccountIsAllowedToMakeAnyOperation) {
		// Arrange:
		auto multisigAccountKey = test::GenerateRandomData<Key_Size>();
		auto cosignatoryAccountKey = test::GenerateRandomData<Key_Size>();
		auto cache = CreateCacheWithSingleLevelMultisig(multisigAccountKey, { cosignatoryAccountKey });

		// Assert:
		AssertValidationResult(ValidationResult::Success, cache, test::GenerateRandomData<Key_Size>());
	}

	TEST(TEST_CLASS, CosignatoryAccountIsAllowedToMakeAnyOperation) {
		// Arrange:
		auto multisigAccountKey = test::GenerateRandomData<Key_Size>();
		auto cosignatoryAccountKey = test::GenerateRandomData<Key_Size>();
		auto cache = CreateCacheWithSingleLevelMultisig(multisigAccountKey, { cosignatoryAccountKey });

		// Assert:
		AssertValidationResult(ValidationResult::Success, cache, cosignatoryAccountKey);
	}

	TEST(TEST_CLASS, MultisigAccountIsNotAllowedToMakeAnyOperation) {
		// Arrange:
		auto multisigAccountKey = test::GenerateRandomData<Key_Size>();
		auto cosignatoryAccountKey = test::GenerateRandomData<Key_Size>();
		auto cache = CreateCacheWithSingleLevelMultisig(multisigAccountKey, { cosignatoryAccountKey });

		// Assert:
		AssertValidationResult(Failure_Multisig_Operation_Not_Permitted_By_Account, cache, multisigAccountKey);
	}
}}
