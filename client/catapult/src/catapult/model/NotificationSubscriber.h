#pragma once
#include "Notifications.h"

namespace catapult { namespace model {

	/// A notification subscriber.
	class NotificationSubscriber {
	public:
		virtual ~NotificationSubscriber() {}

	public:
		/// Notifies the subscriber of \a notification.
		virtual void notify(const Notification& notification) = 0;
	};
}}
