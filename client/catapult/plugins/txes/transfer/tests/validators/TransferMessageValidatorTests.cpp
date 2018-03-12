#include "src/validators/Validators.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS TransferMessageValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(TransferMessage, 0)

	namespace {
		void AssertValidationResult(ValidationResult expectedResult, uint16_t messageSize, uint16_t maxMessageSize) {
			// Arrange:
			auto notification = model::TransferMessageNotification(messageSize);
			auto pValidator = CreateTransferMessageValidator(maxMessageSize);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithMessageSizeLessThanMax) {
		// Assert:
		AssertValidationResult(ValidationResult::Success, 100, 1234);
	}

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithMessageSizeEqualToMax) {
		// Assert:
		AssertValidationResult(ValidationResult::Success, 1234, 1234);
	}

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithMessageSizeGreaterThanMax) {
		// Assert:
		AssertValidationResult(Failure_Transfer_Message_Too_Large, 1235, 1234);
	}
}}
