#include "Observers.h"
#include "catapult/cache_core/BlockDifficultyCache.h"
#include "catapult/state/BlockDifficultyInfo.h"

namespace catapult { namespace observers {

	DEFINE_OBSERVER(BlockDifficulty, model::BlockNotification, [](const auto& notification, const ObserverContext& context) {
		auto info = state::BlockDifficultyInfo(context.Height, notification.Timestamp, notification.Difficulty);
		auto& cache = context.Cache.sub<cache::BlockDifficultyCache>();
		if (NotifyMode::Commit == context.Mode)
			cache.insert(info);
		else
			cache.remove(info);
	});
}}
