#include "Observers.h"
#include "src/cache/HashCache.h"

namespace catapult { namespace observers {
	namespace {
		constexpr bool ShouldPrune(const ObserverContext& context, size_t pruneInterval) {
			return NotifyMode::Commit == context.Mode && 1 == context.Height.unwrap() % pruneInterval;
		}
	}

	NotificationObserverPointerT<model::BlockNotification> CreateTransactionHashPruningObserver(size_t pruneInterval) {
		return std::make_unique<FunctionalNotificationObserverT<model::BlockNotification>>(
				"TransactionHashPruningObserver",
				[pruneInterval](const auto& notification, const ObserverContext& context) {
					if (!ShouldPrune(context, pruneInterval))
						return;

					auto& hashCache = context.Cache.sub<cache::HashCache>();
					hashCache.prune(notification.Timestamp);
				});
	}
}}
