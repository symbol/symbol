#include "src/validators/Validators.h"
#include "catapult/model/VerifiableEntity.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

	DEFINE_COMMON_VALIDATOR_TESTS(Network, static_cast<model::NetworkIdentifier>(123))

	namespace {
		constexpr auto Network_Identifier = static_cast<model::NetworkIdentifier>(123);

		void AssertValidationResult(uint8_t networkIdentifier, ValidationResult expectedResult) {
			// Arrange:
			model::EntityNotification notification(static_cast<model::NetworkIdentifier>(networkIdentifier));
			auto pValidator = CreateNetworkValidator(Network_Identifier);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "network identifier " << networkIdentifier;
		}
	}

	// region validation

	TEST(NetworkValidatorTests, SuccessWhenEntityHasSpecifiedCorrectNetwork) {
		// Assert:
		AssertValidationResult(123u, ValidationResult::Success);
	}

	TEST(NetworkValidatorTests, FailureWhenEntityHasSpecifiedIncorrectNetwork) {
		// Assert:
		for (uint8_t identifier = 0; identifier < 255; ++identifier) {
			if (123u == identifier)
				continue;

			AssertValidationResult(identifier, Failure_Core_Wrong_Network);
		}
	}

	// endregion
}}
