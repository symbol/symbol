#include "Observers.h"
#include "src/cache/MosaicCache.h"

namespace catapult { namespace observers {

	NotificationObserverPointerT<model::MosaicDefinitionNotification> CreateMosaicDefinitionObserver() {
		return std::make_unique<FunctionalNotificationObserverT<model::MosaicDefinitionNotification>>(
				"MosaicDefinitionObserver",
				[](const auto& notification, const ObserverContext& context) {
					auto& cache = context.Cache.sub<cache::MosaicCache>();

					if (NotifyMode::Rollback == context.Mode) {
						cache.remove(notification.MosaicId);
						return;
					}

					const auto& properties = notification.Properties;
					auto definition = state::MosaicDefinition(context.Height, notification.Signer, properties);
					cache.insert(state::MosaicEntry(notification.ParentId, notification.MosaicId, definition));
				});
	}
}}
