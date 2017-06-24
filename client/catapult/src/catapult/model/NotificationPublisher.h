#pragma once
#include "WeakEntityInfo.h"

namespace catapult {
	namespace model {
		class NotificationSubscriber;
		class TransactionRegistry;
	}
}

namespace catapult { namespace model {

	/// A notification publisher.
	class NotificationPublisher {
	public:
		virtual ~NotificationPublisher() {}

	public:
		/// Sends all notifications from \a entityInfo to \a sub.
		virtual void publish(const WeakEntityInfo& entityInfo, NotificationSubscriber& sub) const = 0;
	};

	/// Creates a notification publisher around \a transactionRegistry.
	std::unique_ptr<NotificationPublisher> CreateNotificationPublisher(const TransactionRegistry& transactionRegistry);
}}
