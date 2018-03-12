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

/// Declares an observer with \a NAME for notifications of type \a NOTIFICATION_TYPE.
#define DECLARE_OBSERVER(NAME, NOTIFICATION_TYPE) NotificationObserverPointerT<NOTIFICATION_TYPE> Create##NAME##Observer

/// Makes a functional observer with \a NAME for notifications of type \a NOTIFICATION_TYPE around \a HANDLER.
#define MAKE_OBSERVER(NAME, NOTIFICATION_TYPE, HANDLER) \
	std::make_unique<FunctionalNotificationObserverT<NOTIFICATION_TYPE>>(#NAME "Observer", HANDLER);

/// Defines a functional observer with \a NAME for notifications of type \a NOTIFICATION_TYPE around \a HANDLER.
#define DEFINE_OBSERVER(NAME, NOTIFICATION_TYPE, HANDLER) \
	DECLARE_OBSERVER(NAME, NOTIFICATION_TYPE)() { \
		return MAKE_OBSERVER(NAME, NOTIFICATION_TYPE, HANDLER); \
	}
}}
