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
#include "AggregateNotificationObserver.h"
#include "EntityObserver.h"
#include "FunctionalNotificationObserver.h"
#include <memory>

namespace catapult { namespace observers {

	/// Notification observer for processing a generic Notification.
	using NotificationObserver = NotificationObserverT<model::Notification>;

	/// Notification observer (unique) pointer.
	template<typename TNotification>
	using NotificationObserverPointerT = std::unique_ptr<const NotificationObserverT<TNotification>>;

	/// Aggregate notification observer for processing a generic Notification.
	using AggregateNotificationObserver = AggregateNotificationObserverT<model::Notification>;

	/// Aggregate notification observer (unique) pointer.
	template<typename TNotification>
	using AggregateNotificationObserverPointerT = std::unique_ptr<const AggregateNotificationObserverT<TNotification>>;

/// Declares an observer with \a NAME for notifications of type \a NOTIFICATION_TYPE.
#define DECLARE_OBSERVER(NAME, NOTIFICATION_TYPE) observers::NotificationObserverPointerT<NOTIFICATION_TYPE> Create##NAME##Observer

/// Makes a functional observer with \a NAME for notifications of type \a NOTIFICATION_TYPE around \a HANDLER.
#define MAKE_OBSERVER(NAME, NOTIFICATION_TYPE, HANDLER) \
	std::make_unique<observers::FunctionalNotificationObserverT<NOTIFICATION_TYPE>>(#NAME "Observer", HANDLER)

/// Defines a functional observer with \a NAME for notifications of type \a NOTIFICATION_TYPE around \a HANDLER.
#define DEFINE_OBSERVER(NAME, NOTIFICATION_TYPE, HANDLER) \
	DECLARE_OBSERVER(NAME, NOTIFICATION_TYPE)() { \
		return MAKE_OBSERVER(NAME, NOTIFICATION_TYPE, HANDLER); \
	}
}}
