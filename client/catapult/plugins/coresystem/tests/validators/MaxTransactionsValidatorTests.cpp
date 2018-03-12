#include "src/validators/Validators.h"
#include "tests/test/core/NotificationTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS MaxTransactionsValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MaxTransactions, 123)

	namespace {
		constexpr uint32_t Max_Transactions = 10;

		void AssertValidationResult(ValidationResult expectedResult, uint32_t numTransactions) {
			// Arrange:
			auto signer = test::GenerateRandomData<Key_Size>();
			auto notification = test::CreateBlockNotification(signer);
			notification.NumTransactions = numTransactions;
			auto pValidator = CreateMaxTransactionsValidator(Max_Transactions);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	// region validation

	TEST(TEST_CLASS, SuccessWhenBlockContainsNoTransactions) {
		// Assert:
		AssertValidationResult(ValidationResult::Success, 0);
	}

	TEST(TEST_CLASS, SuccessWhenBlockContainsLessThanMaxTransactions) {
		// Assert:
		constexpr auto expectedResult = ValidationResult::Success;
		AssertValidationResult(expectedResult, 1);
		AssertValidationResult(expectedResult, 5);
		AssertValidationResult(expectedResult, Max_Transactions - 1);
	}

	TEST(TEST_CLASS, SuccessWhenBlockContainsMaxTransactions) {
		// Assert:
		AssertValidationResult(ValidationResult::Success, Max_Transactions);
	}

	TEST(TEST_CLASS, FailureWhenBlockContainsMoreThanMaxTransactions) {
		// Assert:
		constexpr auto expectedResult = Failure_Core_Too_Many_Transactions;
		AssertValidationResult(expectedResult, Max_Transactions + 1);
		AssertValidationResult(expectedResult, Max_Transactions + 10);
		AssertValidationResult(expectedResult, Max_Transactions + 100);
	}

	// endregion
}}
