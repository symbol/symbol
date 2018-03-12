#include "src/validators/Validators.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS TransferMosaicsValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(TransferMosaics,)

	namespace {
		constexpr auto Success_Result = ValidationResult::Success;

		void AssertValidationResult(ValidationResult expectedResult, const std::vector<model::Mosaic>& mosaics) {
			// Arrange:
			model::TransferMosaicsNotification notification(static_cast<uint8_t>(mosaics.size()), mosaics.data());
			auto pValidator = CreateTransferMosaicsValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithZeroMosaics) {
		// Assert:
		AssertValidationResult(Success_Result, {});
	}

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithOneMosaic) {
		// Assert:
		AssertValidationResult(Success_Result, { { MosaicId(71), Amount(5) } });
	}

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithMultipleOrderedMosaics) {
		// Assert:
		AssertValidationResult(
				Success_Result,
				{ { MosaicId(71), Amount(5) }, { MosaicId(182), Amount(4) }, { MosaicId(200), Amount(1) } });
	}

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithMultipleOutOfOrderMosaics) {
		// Assert:
		// - first and second are out of order
		AssertValidationResult(
				Failure_Transfer_Out_Of_Order_Mosaics,
				{ { MosaicId(200), Amount(5) }, { MosaicId(71), Amount(1) }, { MosaicId(182), Amount(4) }, });

		// - second and third are out of order
		AssertValidationResult(
				Failure_Transfer_Out_Of_Order_Mosaics,
				{ { MosaicId(71), Amount(5) }, { MosaicId(200), Amount(1) }, { MosaicId(182), Amount(4) }, });
	}

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithMultipleTransfersOfSameMosaic) {
		// Assert: create a transaction with multiple (in order) transfers for the same mosaic
		AssertValidationResult(
				Failure_Transfer_Out_Of_Order_Mosaics,
				{
					{ MosaicId(71), Amount(5) },
					{ MosaicId(182), Amount(4) },
					{ MosaicId(182), Amount(4) },
					{ MosaicId(200), Amount(1) }
				});
	}
}}
