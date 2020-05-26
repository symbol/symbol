/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

#include "ProcessingUndoNotificationSubscriber.h"

namespace catapult { namespace chain {

	ProcessingUndoNotificationSubscriber::ProcessingUndoNotificationSubscriber(
			const observers::NotificationObserver& observer,
			observers::ObserverContext& observerContext)
			: m_observer(observer)
			, m_observerContext(observerContext)
	{}

	void ProcessingUndoNotificationSubscriber::undo() {
		auto undoMode = observers::NotifyMode::Commit == m_observerContext.Mode
				? observers::NotifyMode::Rollback
				: observers::NotifyMode::Commit;
		auto undoObserverContext = observers::ObserverContext(
				model::NotificationContext(m_observerContext.Height, m_observerContext.Resolvers),
				observers::ObserverState(m_observerContext.Cache),
				undoMode);
		for (auto iter = m_notificationBuffers.crbegin(); m_notificationBuffers.crend() != iter; ++iter) {
			const auto* pNotification = reinterpret_cast<const model::Notification*>(iter->data());
			m_observer.notify(*pNotification, undoObserverContext);
		}

		m_notificationBuffers.clear();
	}

	void ProcessingUndoNotificationSubscriber::notify(const model::Notification& notification) {
		if (notification.Size < sizeof(model::Notification))
			CATAPULT_THROW_INVALID_ARGUMENT("cannot process notification with incorrect size");

		observe(notification);
	}

	void ProcessingUndoNotificationSubscriber::observe(const model::Notification& notification) {
		if (!IsSet(notification.Type, model::NotificationChannel::Observer))
			return;

		// don't actually execute, just store a copy of the notification buffer
		const auto* pData = reinterpret_cast<const uint8_t*>(&notification);
		m_notificationBuffers.emplace_back(pData, pData + notification.Size);
	}
}}
