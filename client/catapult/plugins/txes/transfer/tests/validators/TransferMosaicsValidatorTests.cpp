#include "src/validators/Validators.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

	DEFINE_COMMON_VALIDATOR_TESTS(TransferMosaics,)

	namespace {
		void AssertValidationResult(const std::vector<model::Mosaic>& mosaics, ValidationResult expectedResult) {
			// Arrange:
			model::TransferMosaicsNotification notification(static_cast<uint8_t>(mosaics.size()), mosaics.data());
			auto pValidator = CreateTransferMosaicsValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	TEST(TransferMosaicsValidatorTests, SuccessWhenValidatingNotificationWithZeroMosaics) {
		// Assert:
		AssertValidationResult({}, ValidationResult::Success);
	}

	TEST(TransferMosaicsValidatorTests, SuccessWhenValidatingNotificationWithOneMosaic) {
		// Assert:
		AssertValidationResult({ { MosaicId(71), Amount(5) } }, ValidationResult::Success);
	}

	TEST(TransferMosaicsValidatorTests, SuccessWhenValidatingNotificationWithMultipleOrderedMosaics) {
		// Assert:
		AssertValidationResult(
				{ { MosaicId(71), Amount(5) }, { MosaicId(182), Amount(4) }, { MosaicId(200), Amount(1) } },
				ValidationResult::Success);
	}

	TEST(TransferMosaicsValidatorTests, FailureWhenValidatingNotificationWithMultipleOutOfOrderMosaics) {
		// Assert:
		// - first and second are out of order
		AssertValidationResult(
				{ { MosaicId(200), Amount(5) }, { MosaicId(71), Amount(1) }, { MosaicId(182), Amount(4) }, },
				Failure_Transfer_Out_Of_Order_Mosaics);

		// - second and third are out of order
		AssertValidationResult(
				{ { MosaicId(71), Amount(5) }, { MosaicId(200), Amount(1) }, { MosaicId(182), Amount(4) }, },
				Failure_Transfer_Out_Of_Order_Mosaics);
	}

	TEST(TransferMosaicsValidatorTests, FailureWhenValidatingNotificationWithMultipleTransfersOfSameMosaic) {
		// Assert: create a transaction with multiple (in order) transfers for the same mosaic
		AssertValidationResult({
					{ MosaicId(71), Amount(5) },
					{ MosaicId(182), Amount(4) },
					{ MosaicId(182), Amount(4) },
					{ MosaicId(200), Amount(1) }
				},
				Failure_Transfer_Out_Of_Order_Mosaics);
	}
}}
