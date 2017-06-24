#include "src/validators/Validators.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS MosaicSupplyChangeValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MosaicSupplyChange,)

	namespace {
		void AssertValidationResult(ValidationResult expectedResult, model::MosaicSupplyChangeDirection direction, Amount delta) {
			// Arrange:
			model::MosaicSupplyChangeNotification notification(Key(), MosaicId(), direction, delta);
			auto pValidator = CreateMosaicSupplyChangeValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "direction " << utils::to_underlying_type(direction) << ", delta " << delta;
		}
	}

	// region direction

	namespace {
		constexpr auto ToDirection(int32_t direction) {
			return static_cast<model::MosaicSupplyChangeDirection>(direction);
		}
	}

	TEST(TEST_CLASS, SuccessWhenValidatingValidDirection) {
		// Assert:
		for (auto direction : { 0x00, 0x01 })
			AssertValidationResult(ValidationResult::Success, ToDirection(direction), Amount(123));
	}

	TEST(TEST_CLASS, FailureWhenValidatingInvalidDirection) {
		// Assert:
		for (auto direction : { 0x02, 0xFF })
			AssertValidationResult(Failure_Mosaic_Invalid_Supply_Change_Direction, ToDirection(direction), Amount(123));
	}

	// endregion

	// region amount

	TEST(TEST_CLASS, SuccessWhenDeltaIsNonZero) {
		// Assert:
		for (auto direction : { 0x00, 0x01 })
			AssertValidationResult(ValidationResult::Success, ToDirection(direction), Amount(1));
	}

	TEST(TEST_CLASS, FailureWhenDeltaIsZero) {
		// Assert:
		for (auto direction : { 0x00, 0x01 })
			AssertValidationResult(Failure_Mosaic_Invalid_Supply_Change_Amount, ToDirection(direction), Amount());
	}

	// endregion
}}
