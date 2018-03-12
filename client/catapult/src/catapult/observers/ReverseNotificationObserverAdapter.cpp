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
			void notifyAll(const NotificationObserver& observer, const ObserverContext& context) const {
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

	void ReverseNotificationObserverAdapter::notify(const model::WeakEntityInfo& entityInfo, const ObserverContext& context) const {
		ObservingNotificationSubscriber sub;
		m_pPublisher->publish(entityInfo, sub);
		sub.notifyAll(*m_pObserver, context);
	}
}}
