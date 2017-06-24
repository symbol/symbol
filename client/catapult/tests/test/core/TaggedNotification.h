#pragma once
#include "catapult/model/Notifications.h"

namespace catapult { namespace test {

	/// A notification with a tag.
	template<uint32_t TaggedNotificationType>
	struct TaggedNotificationT : model::Notification {
	public:
		/// The matching notification type.
		static constexpr auto Notification_Type = static_cast<model::NotificationType>(TaggedNotificationType);

	public:
		/// Creates a notification with \a tag.
		explicit TaggedNotificationT(uint8_t tag)
				: Notification(Notification_Type, sizeof(TaggedNotificationT))
				, Tag(tag)
		{}

	public:
		/// The tag value.
		uint8_t Tag;
	};

	/// A tagged notification.
	using TaggedNotification = TaggedNotificationT<0x0000FFFF>;

	/// An alternative tagged notification.
	using TaggedNotification2 = TaggedNotificationT<0x0000FFFE>;
}}
