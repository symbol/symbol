#pragma once
#include "LockNotificationsTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"

namespace catapult { namespace test {

	/// Lock duration validator test suite.
	template<typename TTraits>
	struct LockDurationTests {
	public:
		static void AssertFailureIfDurationIsGreaterThanMaxDurationSetting() {
			// Act + Assert:
			for (auto duration : { 1u, 100u })
				AssertDurationValidator(validators::Failure_Lock_Invalid_Duration, MaxDuration() + BlockDuration(duration));
		}

		static void AssertSuccessIfDurationIsLessThanOrEqualToMaxDurationSetting() {
			// Act + Assert:
			for (auto duration : { 10u, 1u, 0u })
				AssertDurationValidator(validators::ValidationResult::Success, MaxDuration() - BlockDuration(duration));
		}

	private:
		static constexpr BlockDuration MaxDuration() {
			return BlockDuration(100);
		}

		static void AssertDurationValidator(validators::ValidationResult expectedResult, BlockDuration notificationDuration) {
			// Arrange:
			typename TTraits::NotificationType notification(notificationDuration);
			auto pValidator = TTraits::CreateValidator(MaxDuration());

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "duration " << notification.Duration;
		}
	};
}}

#define MAKE_DURATION_VALIDATOR_TEST(TRAITS_NAME, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { test::LockDurationTests<TRAITS_NAME>::Assert##TEST_NAME(); }

#define DEFINE_DURATION_VALIDATOR_TESTS(TRAITS_NAME) \
	MAKE_DURATION_VALIDATOR_TEST(TRAITS_NAME, FailureIfDurationIsGreaterThanMaxDurationSetting) \
	MAKE_DURATION_VALIDATOR_TEST(TRAITS_NAME, SuccessIfDurationIsLessThanOrEqualToMaxDurationSetting)
