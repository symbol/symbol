#include "Observers.h"
#include "src/cache/NamespaceCache.h"

namespace catapult { namespace observers {
	namespace {
		constexpr bool ShouldPrune(const ObserverContext& context, size_t pruneInterval) {
			return NotifyMode::Commit == context.Mode && 1 == context.Height.unwrap() % pruneInterval;
		}
	}

	NotificationObserverPointerT<model::BlockNotification> CreateNamespacePruningObserver(
			ArtifactDuration gracePeriodDuration,
			size_t pruneInterval) {
		return std::make_unique<FunctionalNotificationObserverT<model::BlockNotification>>(
				"NamespacePruningObserver",
				[gracePeriodDuration, pruneInterval](const auto&, const ObserverContext& context) {
					if (!ShouldPrune(context, pruneInterval))
						return;

					auto pruneHeight = Height(1);
					if (context.Height.unwrap() > gracePeriodDuration.unwrap())
						pruneHeight = Height(context.Height.unwrap() - gracePeriodDuration.unwrap());

					auto& namespaceCache = context.Cache.sub<cache::NamespaceCache>();
					namespaceCache.prune(pruneHeight);
				});
	}
}}
