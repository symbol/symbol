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

#include "ReverseNotificationObserverAdapter.h"
#include "catapult/model/NotificationSubscriber.h"
#include "catapult/model/TransactionPlugin.h"

namespace catapult { namespace observers {

	namespace {
		class ObservingNotificationSubscriber : public model::NotificationSubscriber {
		public:
			void notify(const model::Notification& notification) override {
				if (!IsSet(notification.Type, model::NotificationChannel::Observer))
					return;

				// store a copy of the notification buffer
				const auto* pData = reinterpret_cast<const uint8_t*>(&notification);
				m_notificationBuffers.emplace_back(pData, pData + notification.Size);
			}

		public:
			void notifyAll(const NotificationObserver& observer, ObserverContext& context) const {
				for (auto iter = m_notificationBuffers.crbegin(); m_notificationBuffers.crend() != iter; ++iter) {
					const auto* pNotification = reinterpret_cast<const model::Notification*>(iter->data());
					observer.notify(*pNotification, context);
				}
			}

		private:
			std::vector<std::vector<uint8_t>> m_notificationBuffers;
		};
	}

	ReverseNotificationObserverAdapter::ReverseNotificationObserverAdapter(
			NotificationObserverPointer&& pObserver,
			NotificationPublisherPointer&& pPublisher)
			: m_pObserver(std::move(pObserver))
			, m_pPublisher(std::move(pPublisher))
	{}

	const std::string& ReverseNotificationObserverAdapter::name() const {
		return m_pObserver->name();
	}

	void ReverseNotificationObserverAdapter::notify(const model::WeakEntityInfo& entityInfo, ObserverContext& context) const {
		ObservingNotificationSubscriber sub;
		m_pPublisher->publish(entityInfo, sub);
		sub.notifyAll(*m_pObserver, context);
	}
}}
