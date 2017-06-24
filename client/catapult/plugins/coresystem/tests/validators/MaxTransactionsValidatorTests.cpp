#include "src/validators/Validators.h"
#include "tests/test/core/NotificationTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

	DEFINE_COMMON_VALIDATOR_TESTS(MaxTransactions, 123)

	namespace {
		constexpr uint32_t Max_Transactions = 10;

		void AssertValidationResult(uint32_t numTransactions, ValidationResult expectedResult) {
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

	TEST(MaxTransactionsValidatorTests, SuccessWhenBlockContainsNoTransactions) {
		// Assert:
		AssertValidationResult(0, ValidationResult::Success);
	}

	TEST(MaxTransactionsValidatorTests, SuccessWhenBlockContainsLessThanMaxTransactions) {
		// Assert:
		AssertValidationResult(1, ValidationResult::Success);
		AssertValidationResult(5, ValidationResult::Success);
		AssertValidationResult(Max_Transactions - 1, ValidationResult::Success);
	}

	TEST(MaxTransactionsValidatorTests, SuccessWhenBlockContainsMaxTransactions) {
		// Assert:
		AssertValidationResult(Max_Transactions, ValidationResult::Success);
	}

	TEST(MaxTransactionsValidatorTests, FailureWhenBlockContainsMoreThanMaxTransactions) {
		// Assert:
		AssertValidationResult(Max_Transactions + 1, Failure_Core_Too_Many_Transactions);
		AssertValidationResult(Max_Transactions + 10, Failure_Core_Too_Many_Transactions);
		AssertValidationResult(Max_Transactions + 100, Failure_Core_Too_Many_Transactions);
	}

	// endregion
}}
