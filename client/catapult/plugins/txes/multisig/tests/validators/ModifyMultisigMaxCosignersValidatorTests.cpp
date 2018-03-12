#include "src/validators/Validators.h"
#include "catapult/utils/Functional.h"
#include "tests/test/MultisigCacheTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS ModifyMultisigMaxCosignersValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(ModifyMultisigMaxCosigners, 0)

	namespace {
		constexpr auto Add = model::CosignatoryModificationType::Add;
		constexpr auto Del = model::CosignatoryModificationType::Del;

		void AssertValidationResult(
				ValidationResult expectedResult,
				uint8_t numInitialCosigners,
				const std::vector<model::CosignatoryModificationType>& modificationTypes,
				uint8_t maxCosignersPerAccount) {
			// Arrange:
			auto signer = test::GenerateRandomData<Key_Size>();

			// - setup cache
			auto cache = test::MultisigCacheFactory::Create();
			if (numInitialCosigners > 0) {
				auto cacheDelta = cache.createDelta();
				auto entry = state::MultisigEntry(signer);

				// - add cosignatories
				for (auto i = 0; i < numInitialCosigners; ++i)
					entry.cosignatories().insert(test::GenerateRandomData<Key_Size>());

				cacheDelta.sub<cache::MultisigCache>().insert(entry);
				cache.commit(Height());
			}

			// - create the validator context
			auto cacheView = cache.createView();
			auto readOnlyCache = cacheView.toReadOnly();
			auto context = test::CreateValidatorContext(Height(), readOnlyCache);

			std::vector<model::CosignatoryModification> modifications;
			for (auto modificationType : modificationTypes)
				modifications.push_back({ modificationType, test::GenerateRandomData<Key_Size>() });

			model::ModifyMultisigCosignersNotification notification(
					signer,
					static_cast<uint8_t>(modifications.size()),
					modifications.data());
			auto pValidator = CreateModifyMultisigMaxCosignersValidator(maxCosignersPerAccount);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, context);

			// Assert:
			auto change = utils::Sum(modificationTypes, [](const auto& modificationType) { return Add == modificationType ? 1 : -1; });
			EXPECT_EQ(expectedResult, result)
					<< "initial " << static_cast<uint32_t>(numInitialCosigners)
					<< ", change " << change
					<< ", max " << static_cast<uint32_t>(maxCosignersPerAccount);
		}
	}

	TEST(TEST_CLASS, CanAddCosignersIfMaxCosignersIsNotExceeded) {
		// Assert:
		AssertValidationResult(ValidationResult::Success, 0, { Add, Add, Add, Add, Add }, 10);
	}

	TEST(TEST_CLASS, CanAddAndDeleteCosignersWhenResultingNumberOfCosignerDoesNotExceedMaxCosigners) {
		// Assert:
		AssertValidationResult(ValidationResult::Success, 9, { Add, Add, Add, Del, Del }, 10);
		AssertValidationResult(ValidationResult::Success, 9, { Del, Del, Add, Add, Add }, 10);
		AssertValidationResult(ValidationResult::Success, 9, { Del, Add, Add, Del, Add }, 10);
	}

	TEST(TEST_CLASS, CannotAddCosignersIfMaxCosignersIsExceeded) {
		// Assert:
		AssertValidationResult(Failure_Multisig_Modify_Max_Cosigners, 10, { Add }, 10);
		AssertValidationResult(Failure_Multisig_Modify_Max_Cosigners, 8, { Add, Add, Add }, 10);
	}

	TEST(TEST_CLASS, CannotAddAndDeleteCosignersWhenResultingNumberOfCosignerDoesExceedMaxCosigners) {
		// Assert:
		AssertValidationResult(Failure_Multisig_Modify_Max_Cosigners, 9, { Add, Add, Add, Del }, 10);
		AssertValidationResult(Failure_Multisig_Modify_Max_Cosigners, 9, { Del, Add, Add, Add }, 10);
		AssertValidationResult(Failure_Multisig_Modify_Max_Cosigners, 9, { Del, Add, Del, Add, Add, Add }, 10);
	}
}}
