#include "src/validators/Validators.h"
#include "catapult/model/VerifiableEntity.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

	DEFINE_COMMON_VALIDATOR_TESTS(Deadline, utils::TimeSpan::FromSeconds(14))

	namespace {
		const auto Block_Time = Timestamp(8888);
		const auto Max_Transaction_Lifetime = []() { return utils::TimeSpan::FromHours(2); }();

		void AssertValidationResult(Timestamp deadline, ValidationResult expectedResult) {
			// Arrange:
			auto cache = test::CreateEmptyCatapultCache();
			auto cacheView = cache.createView();
			auto readOnlyCache = cacheView.toReadOnly();
			auto context = ValidatorContext(Height(123), Block_Time, model::NetworkInfo(), readOnlyCache);
			auto pValidator = CreateDeadlineValidator(Max_Transaction_Lifetime);

			model::TransactionNotification notification(Key(), Hash256(), deadline);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, context);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	// region basic tests

	TEST(DeadlineValidatorTests, SuccessWhenTransactionDeadlineIsValid) {
		// Arrange:
		auto deadline = Block_Time + utils::TimeSpan::FromHours(1);

		// Assert:
		AssertValidationResult(deadline, ValidationResult::Success);
	}

	TEST(DeadlineValidatorTests, SuccessWhenTransactionDeadlineIsEqualToTimestampPlusLifetime) {
		// Arrange:
		auto deadline = Block_Time + utils::TimeSpan::FromHours(2);

		// Assert:
		AssertValidationResult(deadline, ValidationResult::Success);
	}

	TEST(DeadlineValidatorTests, SuccessWhenTransactionDeadlineIsEqualToBlockTime) {
		// Arrange:
		auto deadline = Block_Time;

		// Assert:
		AssertValidationResult(deadline, ValidationResult::Success);
	}

	TEST(DeadlineValidatorTests, FailureWhenTransactionDeadlineIsLessThanBlockTime) {
		// Arrange:
		auto deadline = Timestamp(Block_Time.unwrap() - 1);

		// Assert:
		AssertValidationResult(deadline, Failure_Core_Past_Deadline);
	}

	TEST(DeadlineValidatorTests, FailureWhenTransactionDeadlineIsLargerThanBlockTimePlusLifetime) {
		// Arrange:
		auto deadline = Block_Time + utils::TimeSpan::FromHours(3);

		// Assert:
		AssertValidationResult(deadline, Failure_Core_Future_Deadline);
	}

	// endregion
}}
