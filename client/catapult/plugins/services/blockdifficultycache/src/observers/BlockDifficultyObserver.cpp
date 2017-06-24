#include "Observers.h"
#include "src/cache/BlockDifficultyCache.h"
#include "catapult/state/BlockDifficultyInfo.h"

namespace catapult { namespace observers {

	NotificationObserverPointerT<model::BlockNotification> CreateBlockDifficultyObserver() {
		return std::make_unique<FunctionalNotificationObserverT<model::BlockNotification>>(
				"BlockDifficultyObserver",
				[](const auto& notification, const ObserverContext& context) {
					auto info = state::BlockDifficultyInfo(context.Height, notification.Timestamp, notification.Difficulty);
					auto& cache = context.Cache.sub<cache::BlockDifficultyCache>();
					if (NotifyMode::Commit == context.Mode)
						cache.insert(info);
					else
						cache.remove(info);
				});
	}
}}
