#include "Observers.h"
#include "LockInfoObserverUtils.h"
#include "src/cache/HashLockInfoCache.h"
#include "plugins/txes/aggregate/src/model/AggregateEntityType.h"

namespace catapult { namespace observers {

	using Notification = model::TransactionNotification;

	namespace {
		struct HashTraits {
		public:
			using CacheType = cache::HashLockInfoCache;
			using Notification = observers::Notification;

			static auto NotificationToKey(const Notification& notification) {
				return notification.TransactionHash;
			}

			static auto DestinationAccount(const model::HashLockInfo& lockInfo) {
				return lockInfo.Account;
			}
		};
	}

	DEFINE_OBSERVER(CompletedAggregate, Notification, [](const auto& notification, const auto& context) {
		if (model::Entity_Type_Aggregate_Bonded != notification.TransactionType)
			return;

		LockStatusAccountBalanceObserver<HashTraits>(notification, context);
	});
}}
