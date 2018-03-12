#include "src/validators/Validators.h"
#include "catapult/constants.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS HashLockMosaicValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(HashLockMosaic, Amount(123))

	namespace {
		void AssertValidationResult(ValidationResult expectedResult, MosaicId mosaicId, Amount bondedAmount, Amount requiredBondedAmount) {
			// Arrange:
			model::Mosaic mosaic = { mosaicId, bondedAmount };
			auto notification = model::HashLockMosaicNotification(mosaic);
			auto pValidator = CreateHashLockMosaicValidator(requiredBondedAmount);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "notification with id " << mosaicId << " and amount " << bondedAmount;
		}
	}

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithProperMosaicIdAndAmount) {
		// Assert:
		AssertValidationResult(ValidationResult::Success, Xem_Id, Amount(500), Amount(500));
	}

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithInvalidMosaicId) {
		// Assert:
		AssertValidationResult(Failure_Lock_Invalid_Mosaic_Id, test::GenerateRandomValue<MosaicId>(), Amount(500), Amount(500));
	}

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithInvalidAmount) {
		// Assert:
		for (auto amount : { 0ull, 1ull, 10ull, 100ull, 499ull, 501ull, 1000ull })
			AssertValidationResult(Failure_Lock_Invalid_Mosaic_Amount, MosaicId(123), Amount(amount), Amount(500));
	}
}}
