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

#pragma once
#include "catapult/model/Notifications.h"

namespace catapult { namespace test {

	/// Notification with a tag.
	template<uint32_t TaggedNotificationType>
	struct TaggedNotificationT : model::Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = static_cast<model::NotificationType>(TaggedNotificationType);

	public:
		/// Creates a notification with \a tag.
		explicit TaggedNotificationT(uint8_t tag)
				: Notification(Notification_Type, sizeof(TaggedNotificationT))
				, Tag(tag)
		{}

	public:
		/// Tag value.
		uint8_t Tag;
	};

	/// Tagged notification.
	using TaggedNotification = TaggedNotificationT<0x0000FFFF>;

	/// Alternative tagged notification.
	using TaggedNotification2 = TaggedNotificationT<0x0000FFFE>;
}}
