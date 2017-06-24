#include "Observers.h"
#include "src/cache/BlockDifficultyCache.h"

namespace catapult { namespace observers {
	namespace {
		constexpr bool ShouldPrune(const ObserverContext& context, size_t pruneInterval) {
			return NotifyMode::Commit == context.Mode && 1 == context.Height.unwrap() % pruneInterval;
		}
	}

	NotificationObserverPointerT<model::BlockNotification> CreateBlockDifficultyPruningObserver(size_t pruneInterval) {
		return std::make_unique<FunctionalNotificationObserverT<model::BlockNotification>>(
				"BlockDifficultyPruningObserver",
				[pruneInterval](const auto&, const ObserverContext& context) {
					if (!ShouldPrune(context, pruneInterval))
						return;

					auto& difficultyCache = context.Cache.sub<cache::BlockDifficultyCache>();
					difficultyCache.prune(context.Height);
				});
	}
}}
