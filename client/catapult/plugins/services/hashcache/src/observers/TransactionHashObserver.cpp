#include "Observers.h"
#include "src/cache/HashCache.h"

namespace catapult { namespace observers {

	NotificationObserverPointerT<model::TransactionNotification> CreateTransactionHashObserver() {
		return std::make_unique<FunctionalNotificationObserverT<model::TransactionNotification>>(
				"TransactionHashObserver",
				[](const auto& notification, const ObserverContext& context) {

					auto timestampedHash = state::TimestampedHash(notification.Deadline, notification.EntityHash);
					auto& cache = context.Cache.sub<cache::HashCache>();

					if (NotifyMode::Commit == context.Mode)
						cache.insert(timestampedHash);
					else
						cache.remove(timestampedHash);
				});
	}
}}
