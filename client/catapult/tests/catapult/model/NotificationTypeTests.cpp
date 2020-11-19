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

#include "catapult/model/NotificationType.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS NotificationTypeTests

	namespace {
		constexpr NotificationType MakeNotificationType(NotificationChannel channel, uint8_t facility, uint16_t code) {
			return MakeNotificationType(channel, static_cast<FacilityCode>(facility), code);
		}

		constexpr NotificationType MakeNotificationType(uint8_t channel, uint8_t facility, uint16_t code) {
			return MakeNotificationType(static_cast<NotificationChannel>(channel), facility, code);
		}
	}

	// region MakeNotificationType / DEFINE_NOTIFICATION_TYPE

	TEST(TEST_CLASS, CanMakeNotificationType) {
		// Assert: zeros
		EXPECT_EQ(NotificationType(0x00000000), MakeNotificationType(0, 0, 0));

		// - max single component value
		EXPECT_EQ(NotificationType(0xFF000000), MakeNotificationType(0xFF, 0, 0));
		EXPECT_EQ(NotificationType(0x00FF0000), MakeNotificationType(0, 0xFF, 0));
		EXPECT_EQ(NotificationType(0x0000FFFF), MakeNotificationType(0, 0, 0xFFFF));

		// - all component values
		EXPECT_EQ(NotificationType(0x01020005), MakeNotificationType(1, 2, 5));
	}

	TEST(TEST_CLASS, CanMakeNotificationTypeViaMacro) {
		// Act
		DEFINE_NOTIFICATION_TYPE(Validator, Core, Alpha, 0x1234);
		DEFINE_NOTIFICATION_TYPE(Observer, Namespace, Beta, 0x8800);
		DEFINE_NOTIFICATION_TYPE(All, Transfer, Gamma, 0x00AB);

		// Assert:
		EXPECT_EQ(NotificationType(0x01431234), Core_Alpha_Notification);
		EXPECT_EQ(NotificationType(0x024E8800), Namespace_Beta_Notification);
		EXPECT_EQ(NotificationType(0xFF5400AB), Transfer_Gamma_Notification);
	}

	// endregion

	// region IsSet

	TEST(TEST_CLASS, CanCheckWhetherOrNotChannelFlagIsSet) {
		// Assert: none
		auto type = MakeNotificationType(NotificationChannel::None, 0, 0);
		EXPECT_FALSE(IsSet(type, static_cast<NotificationChannel>(0x01)));
		EXPECT_FALSE(IsSet(type, static_cast<NotificationChannel>(0x20)));
		EXPECT_FALSE(IsSet(type, static_cast<NotificationChannel>(0x21)));
		EXPECT_FALSE(IsSet(type, static_cast<NotificationChannel>(0xFF)));

		// - all
		type = MakeNotificationType(NotificationChannel::All, 0, 0);
		EXPECT_TRUE(IsSet(type, static_cast<NotificationChannel>(0x01)));
		EXPECT_TRUE(IsSet(type, static_cast<NotificationChannel>(0x20)));
		EXPECT_TRUE(IsSet(type, static_cast<NotificationChannel>(0x21)));
		EXPECT_TRUE(IsSet(type, static_cast<NotificationChannel>(0xFF)));

		// - some
		type = MakeNotificationType(static_cast<NotificationChannel>(0x84), 0, 0);
		EXPECT_TRUE(IsSet(type, static_cast<NotificationChannel>(0x80)));
		EXPECT_TRUE(IsSet(type, static_cast<NotificationChannel>(0x04)));
		EXPECT_TRUE(IsSet(type, static_cast<NotificationChannel>(0x84)));
		EXPECT_FALSE(IsSet(type, static_cast<NotificationChannel>(0x07)));
		EXPECT_FALSE(IsSet(type, static_cast<NotificationChannel>(0x08)));
		EXPECT_FALSE(IsSet(type, static_cast<NotificationChannel>(0x15)));
	}

	// endregion

	// region GetNotificationChannel

	TEST(TEST_CLASS, CanGetNotificationChannelFromNotificationType) {
		// Assert: none
		auto type = MakeNotificationType(NotificationChannel::None, 0, 0);
		EXPECT_EQ(NotificationChannel::None, GetNotificationChannel(type));

		// - all
		type = MakeNotificationType(NotificationChannel::All, 0, 0);
		EXPECT_EQ(NotificationChannel::All, GetNotificationChannel(type));

		// - some
		type = MakeNotificationType(static_cast<NotificationChannel>(0x84), 0, 0);
		EXPECT_EQ(static_cast<NotificationChannel>(0x84), GetNotificationChannel(type));
	}

	// endregion

	// region SetNotificationChannel

	TEST(TEST_CLASS, CanSetNotificationChannelInNotificationType) {
		// Arrange:
		auto type = MakeNotificationType(static_cast<NotificationChannel>(0x74), 0xAB, 0x9876);

		// Act:
		SetNotificationChannel(type, static_cast<NotificationChannel>(0x84));

		// Assert:
		EXPECT_EQ(static_cast<NotificationType>(0x84AB9876), type);
	}

	// endregion

	// region AreEqualExcludingChannel

	TEST(TEST_CLASS, AreEqualExcludingChannelReturnsTrueWhenTypesHaveSameFacilityAndCode) {
		// Arrange:
		auto type = MakeNotificationType(NotificationChannel::Observer, 0xAB, 0x9876);

		// Act + Assert:
		EXPECT_TRUE(AreEqualExcludingChannel(type, type));
		EXPECT_TRUE(AreEqualExcludingChannel(type, MakeNotificationType(NotificationChannel::Validator, 0xAB, 0x9876)));
		EXPECT_FALSE(AreEqualExcludingChannel(type, MakeNotificationType(NotificationChannel::Observer, 0xAC, 0x9876)));
		EXPECT_FALSE(AreEqualExcludingChannel(type, MakeNotificationType(NotificationChannel::Observer, 0xAB, 0x9875)));
	}

	// endregion
}}
