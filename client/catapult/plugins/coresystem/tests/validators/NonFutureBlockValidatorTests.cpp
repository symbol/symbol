#include "src/validators/Validators.h"
#include "catapult/utils/NetworkTime.h"
#include "tests/test/core/NotificationTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

	DEFINE_COMMON_VALIDATOR_TESTS(NonFutureBlock, utils::TimeSpan::FromSeconds(15))

	namespace {
		using TimeSpan = std::chrono::duration<uint64_t, std::milli>;
		const auto Max_Allowed_Ahead_Of_Time = []() { return utils::TimeSpan::FromSeconds(11); }();

		auto ValidateEntity(int64_t offset) {
			// Arrange:
			auto signer = test::GenerateRandomData<Key_Size>();
			auto notification = test::CreateBlockNotification(signer);
			auto pValidator = CreateNonFutureBlockValidator(Max_Allowed_Ahead_Of_Time);

			// Act:
			ValidationResult result;
			test::RunDeterministicOperation([&]() {
				auto referenceTime = utils::NetworkTime() + Max_Allowed_Ahead_Of_Time;
				notification.Timestamp = Timestamp(referenceTime.unwrap() + static_cast<uint64_t>(offset));
				result = test::ValidateNotification(*pValidator, notification);
			});
			return result;
		}
	}

	// region validation

	TEST(NonFutureBlockValidatorTests, SuccessWhenTimestampIsSmallerThanAcceptedLimit) {
		// Act:
		auto result = ValidateEntity(-1);

		// Assert:
		EXPECT_EQ(ValidationResult::Success, result);
	}

	TEST(NonFutureBlockValidatorTests, SuccessWhenTimestampIsEqualToAcceptedLimit) {
		// Act:
		auto result = ValidateEntity(0);

		// Assert:
		EXPECT_EQ(ValidationResult::Success, result);
	}

	TEST(NonFutureBlockValidatorTests, FailureWhenTimestampIsLargerThanAcceptedLimit) {
		// Act:
		auto result = ValidateEntity(1);

		// Assert:
		EXPECT_EQ(Failure_Core_Timestamp_Too_Far_In_Future, result);
	}

	// endregion
}}
