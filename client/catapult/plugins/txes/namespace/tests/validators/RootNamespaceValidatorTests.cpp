#include "src/validators/Validators.h"
#include "src/model/IdGenerator.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS RootNamespaceValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(RootNamespace, BlockDuration())

	// region duration

	namespace {
		void AssertDurationValidationResult(ValidationResult expectedResult, uint16_t duration, uint16_t maxDuration) {
			// Arrange:
			auto pValidator = CreateRootNamespaceValidator(BlockDuration(maxDuration));
			auto notification = model::RootNamespaceNotification(Key(), NamespaceId(), BlockDuration(duration));

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "duration " << duration << ", maxDuration " << maxDuration;
		}
	}

	TEST(TEST_CLASS, SuccessWhenValidatingRootNamespaceWithDurationLessThanMax) {
		// Assert:
		AssertDurationValidationResult(ValidationResult::Success, 12312, 12345);
	}

	TEST(TEST_CLASS, SuccessWhenValidatingRootNamespaceWithDurationEqualToMax) {
		// Assert:
		AssertDurationValidationResult(ValidationResult::Success, 12345, 12345);
	}

	TEST(TEST_CLASS, FailureWhenValidatingRootNamespaceWithDurationGreaterThanMax) {
		// Assert:
		AssertDurationValidationResult(Failure_Namespace_Invalid_Duration, 12346, 12345);
		AssertDurationValidationResult(Failure_Namespace_Invalid_Duration, 65432, 12345);
	}

	TEST(TEST_CLASS, SuccessWhenValidatingRootNamespaceWithZeroDuration) {
		// Assert: eternal duration is allowed
		AssertDurationValidationResult(ValidationResult::Success, 0, 12345);
	}

	// endregion
}}
