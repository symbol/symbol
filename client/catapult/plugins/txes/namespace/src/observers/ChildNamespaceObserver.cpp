#include "Observers.h"
#include "src/cache/NamespaceCache.h"

namespace catapult { namespace observers {

	NotificationObserverPointerT<model::ChildNamespaceNotification> CreateChildNamespaceObserver() {
		return std::make_unique<FunctionalNotificationObserverT<model::ChildNamespaceNotification>>(
				"ChildNamespaceObserver",
				[](const auto& notification, const ObserverContext& context) {
					auto& cache = context.Cache.sub<cache::NamespaceCache>();

					if (NotifyMode::Rollback == context.Mode) {
						cache.remove(notification.NamespaceId);
						return;
					}

					// make copy of parent path and append child id
					const auto& parentEntry = cache.get(notification.ParentId);
					auto childPath = parentEntry.ns().path();
					childPath.push_back(notification.NamespaceId);
					cache.insert(state::Namespace(childPath));
				});
	}
}}
