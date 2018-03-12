#include "NotificationObserverAdapter.h"
#include "catapult/model/NotificationSubscriber.h"
#include "catapult/model/TransactionPlugin.h"

namespace catapult { namespace observers {

	namespace {
		class ObservingNotificationSubscriber : public model::NotificationSubscriber {
		public:
			explicit ObservingNotificationSubscriber(const NotificationObserver& observer, const ObserverContext& context)
					: m_observer(observer)
					, m_context(context)
			{}

		public:
			void notify(const model::Notification& notification) override {
				if (!IsSet(notification.Type, model::NotificationChannel::Observer))
					return;

				m_observer.notify(notification, m_context);
			}

		private:
			const NotificationObserver& m_observer;
			const ObserverContext& m_context;
		};
	}

	NotificationObserverAdapter::NotificationObserverAdapter(
			NotificationObserverPointer&& pObserver,
			NotificationPublisherPointer&& pPublisher)
			: m_pObserver(std::move(pObserver))
			, m_pPublisher(std::move(pPublisher))
	{}

	const std::string& NotificationObserverAdapter::name() const {
		return m_pObserver->name();
	}

	void NotificationObserverAdapter::notify(const model::WeakEntityInfo& entityInfo, const ObserverContext& context) const {
		ObservingNotificationSubscriber sub(*m_pObserver, context);
		m_pPublisher->publish(entityInfo, sub);
	}
}}
