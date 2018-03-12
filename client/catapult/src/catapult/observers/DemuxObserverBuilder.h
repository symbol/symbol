#pragma once
#include "AggregateObserverBuilder.h"
#include "ObserverTypes.h"
#include <functional>
#include <vector>

namespace catapult { namespace observers {

	/// A demultiplexing observer builder.
	class DemuxObserverBuilder {
	private:
		using NotificationObserverPredicate = predicate<const model::Notification&>;

	public:
		/// Adds an observer (\a pObserver) to the builder that is invoked only when matching notifications are processed.
		template<typename TNotification>
		DemuxObserverBuilder& add(NotificationObserverPointerT<TNotification>&& pObserver) {
			auto predicate = [type = TNotification::Notification_Type](const auto& notification) {
				return model::AreEqualExcludingChannel(type, notification.Type);
			};
			m_builder.add(std::make_unique<ConditionalObserver<TNotification>>(std::move(pObserver), predicate));
			return *this;
		}

		/// Builds a demultiplexing observer.
		AggregateNotificationObserverPointerT<model::Notification> build() {
			return m_builder.build();
		}

	private:
		template<typename TNotification>
		class ConditionalObserver : public NotificationObserver {
		public:
			ConditionalObserver(NotificationObserverPointerT<TNotification>&& pObserver, const NotificationObserverPredicate& predicate)
					: m_pObserver(std::move(pObserver))
					, m_predicate(predicate)
			{}

		public:
			const std::string& name() const override {
				return m_pObserver->name();
			}

			void notify(const model::Notification& notification, const ObserverContext& context) const override {
				if (!m_predicate(notification))
					return;

				m_pObserver->notify(static_cast<const TNotification&>(notification), context);
			}

		private:
			NotificationObserverPointerT<TNotification> m_pObserver;
			NotificationObserverPredicate m_predicate;
		};

	private:
		AggregateObserverBuilder<model::Notification> m_builder;
	};

	/// Adds an observer (\a pObserver) to the builder that is always invoked.
	template<>
	CATAPULT_INLINE
	DemuxObserverBuilder& DemuxObserverBuilder::add(NotificationObserverPointerT<model::Notification>&& pObserver) {
		m_builder.add(std::move(pObserver));
		return *this;
	}
}}
