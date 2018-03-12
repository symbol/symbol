#include "catapult/validators/ValidatingNotificationSubscriber.h"
#include "tests/test/core/NotificationTestUtils.h"
#include "tests/test/other/mocks/MockNotificationValidator.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS ValidatingNotificationSubscriberTests

	using MockNotificationValidator = mocks::MockStatelessNotificationValidatorT<model::Notification>;

	namespace {
		model::NotificationType MakeNotificationType(uint16_t code, bool isMatch = true) {
			auto channel = isMatch
					? model::NotificationChannel::Validator
					: model::NotificationChannel::Observer;
			return model::MakeNotificationType(channel, model::FacilityCode::Core, code);
		}
	}

	TEST(TEST_CLASS, ResultIsInitiallySuccess) {
		// Arrange:
		MockNotificationValidator validator;
		ValidatingNotificationSubscriber subscriber(validator);

		// Act:
		auto result = subscriber.result();

		// Assert:
		EXPECT_EQ(ValidationResult::Success, result);
	}

	TEST(TEST_CLASS, SubscriberDoesNotForwardNotificationsWithWrongChannel) {
		// Arrange:
		MockNotificationValidator validator;
		ValidatingNotificationSubscriber subscriber(validator);

		// Act: send a notification without the validation channel set
		subscriber.notify(test::CreateNotification(MakeNotificationType(1, false)));
		auto result = subscriber.result();

		// Assert:
		EXPECT_EQ(ValidationResult::Success, result);
		EXPECT_EQ(0u, validator.notificationTypes().size());
	}

	TEST(TEST_CLASS, SubscriberForwardsNotificationsWithMatchingChannel) {
		// Arrange:
		MockNotificationValidator validator;
		ValidatingNotificationSubscriber subscriber(validator);

		// Act:
		subscriber.notify(test::CreateNotification(MakeNotificationType(1)));
		auto result = subscriber.result();

		// Assert:
		EXPECT_EQ(ValidationResult::Success, result);
		ASSERT_EQ(1u, validator.notificationTypes().size());
		EXPECT_EQ(MakeNotificationType(1), validator.notificationTypes()[0]);
	}

	TEST(TEST_CLASS, SubscriberShortCircuitsOnFailure) {
		// Arrange:
		MockNotificationValidator validator;
		ValidatingNotificationSubscriber subscriber(validator);

		// Act: S, N, F, F, N, S
		subscriber.notify(test::CreateNotification(MakeNotificationType(1)));
		validator.setResult(ValidationResult::Neutral);
		subscriber.notify(test::CreateNotification(MakeNotificationType(2)));
		validator.setResult(ValidationResult::Failure);
		subscriber.notify(test::CreateNotification(MakeNotificationType(3)));
		subscriber.notify(test::CreateNotification(MakeNotificationType(4)));
		validator.setResult(ValidationResult::Neutral);
		subscriber.notify(test::CreateNotification(MakeNotificationType(5)));
		validator.setResult(ValidationResult::Success);
		subscriber.notify(test::CreateNotification(MakeNotificationType(6)));
		auto result = subscriber.result();

		// Assert:
		EXPECT_EQ(ValidationResult::Failure, result);
		ASSERT_EQ(3u, validator.notificationTypes().size());
		EXPECT_EQ(MakeNotificationType(1), validator.notificationTypes()[0]);
		EXPECT_EQ(MakeNotificationType(2), validator.notificationTypes()[1]);
		EXPECT_EQ(MakeNotificationType(3), validator.notificationTypes()[2]);
	}
}}
