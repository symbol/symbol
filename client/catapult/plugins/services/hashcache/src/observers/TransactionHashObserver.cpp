#include "Observers.h"
#include "src/cache/HashCache.h"

namespace catapult { namespace observers {

	DEFINE_OBSERVER(TransactionHash, model::TransactionNotification, [](const auto& notification, const ObserverContext& context) {
		auto timestampedHash = state::TimestampedHash(notification.Deadline, notification.TransactionHash);
		auto& cache = context.Cache.sub<cache::HashCache>();

		if (NotifyMode::Commit == context.Mode)
			cache.insert(timestampedHash);
		else
			cache.remove(timestampedHash);
	});
}}
