#pragma once
#include "AggregateNotificationObserver.h"
#include "EntityObserver.h"
#include "FunctionalNotificationObserver.h"
#include <memory>

namespace catapult { namespace observers {

	/// A notification observer for processing a generic Notification.
	using NotificationObserver = NotificationObserverT<model::Notification>;

	/// A notification observer (unique) pointer.
	template<typename TNotification>
	using NotificationObserverPointerT = std::unique_ptr<const NotificationObserverT<TNotification>>;

	/// An aggregate notification observer for processing a generic Notification.
	using AggregateNotificationObserver = AggregateNotificationObserverT<model::Notification>;

	/// An aggregate notification observer (unique) pointer.
	template<typename TNotification>
	using AggregateNotificationObserverPointerT = std::unique_ptr<const AggregateNotificationObserverT<TNotification>>;
}}
