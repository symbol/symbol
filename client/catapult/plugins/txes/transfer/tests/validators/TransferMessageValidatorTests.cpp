#include "src/validators/Validators.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

	DEFINE_COMMON_VALIDATOR_TESTS(TransferMessage, 0)

	namespace {
		void AssertValidationResult(uint16_t messageSize, uint16_t maxMessageSize, ValidationResult expectedResult) {
			// Arrange:
			auto notification = model::TransferMessageNotification(messageSize);
			auto pValidator = CreateTransferMessageValidator(maxMessageSize);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	TEST(TransferMessageValidatorTests, SuccessWhenValidatingNotificationWithMessageSizeLessThanMax) {
		// Assert:
		AssertValidationResult(100, 1234, ValidationResult::Success);
	}

	TEST(TransferMessageValidatorTests, SuccessWhenValidatingNotificationWithMessageSizeEqualToMax) {
		// Assert:
		AssertValidationResult(1234, 1234, ValidationResult::Success);
	}

	TEST(TransferMessageValidatorTests, FailureWhenValidatingNotificationWithMessageSizeGreaterThanMax) {
		// Assert:
		AssertValidationResult(1235, 1234, Failure_Transfer_Message_Too_Large);
	}
}}
