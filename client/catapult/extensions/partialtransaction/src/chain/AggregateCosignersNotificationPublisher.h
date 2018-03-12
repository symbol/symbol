#pragma once

namespace catapult {
	namespace model {
		class NotificationSubscriber;
		class WeakCosignedTransactionInfo;
	}
}

namespace catapult { namespace chain {

	/// An aggregate notification publisher that only publishes cosigner-related notifications.
	class AggregateCosignersNotificationPublisher {
	public:
		/// Sends all notifications from an aggregate \a transactionInfo to \a sub.
		void publish(const model::WeakCosignedTransactionInfo& transactionInfo, model::NotificationSubscriber& sub) const;
	};
}}
