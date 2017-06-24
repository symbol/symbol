#pragma once
#include "ObserverContext.h"
#include "catapult/model/Notifications.h"

namespace catapult { namespace observers {

	/// A strongly typed notification observer.
	template<typename TNotification>
	class NotificationObserverT {
	public:
		/// The notification type.
		using NotificationType = TNotification;

	public:
		virtual ~NotificationObserverT() {}

	public:
		/// Gets the observer name.
		virtual const std::string& name() const = 0;

		/// Notifies the observer with a \a notification to process and an observer \a context.
		virtual void notify(const TNotification& notification, const ObserverContext& context) const = 0;
	};
}}
