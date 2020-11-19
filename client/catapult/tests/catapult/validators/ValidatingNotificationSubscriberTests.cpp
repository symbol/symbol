/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

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

	TEST(TEST_CLASS, SubscriberRespectsNotificationTypeExclusionFilter) {
		// Arrange:
		MockNotificationValidator validator;
		ValidatingNotificationSubscriber subscriber(validator);
		subscriber.setExclusionFilter([](auto notificationType) {
			return MakeNotificationType(2) == notificationType || MakeNotificationType(3) == notificationType;
		});

		// Act:
		for (uint16_t i = 1; i <= 5; ++i)
			subscriber.notify(test::CreateNotification(MakeNotificationType(i)));

		auto result = subscriber.result();

		// Assert:
		EXPECT_EQ(ValidationResult::Success, result);
		ASSERT_EQ(3u, validator.notificationTypes().size());
		EXPECT_EQ(MakeNotificationType(1), validator.notificationTypes()[0]);
		EXPECT_EQ(MakeNotificationType(4), validator.notificationTypes()[1]);
		EXPECT_EQ(MakeNotificationType(5), validator.notificationTypes()[2]);
	}
}}
