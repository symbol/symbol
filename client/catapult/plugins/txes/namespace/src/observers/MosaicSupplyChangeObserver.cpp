#include "Observers.h"
#include "src/cache/MosaicCache.h"
#include "plugins/coresystem/src/cache/AccountStateCache.h"

namespace catapult { namespace observers {

	namespace {
		constexpr bool ShouldIncrease(NotifyMode mode, model::MosaicSupplyChangeDirection direction) {
			return
					(NotifyMode::Commit == mode && model::MosaicSupplyChangeDirection::Increase == direction) ||
					(NotifyMode::Rollback == mode && model::MosaicSupplyChangeDirection::Decrease == direction);
		}
	}

	NotificationObserverPointerT<model::MosaicSupplyChangeNotification> CreateMosaicSupplyChangeObserver() {
		return std::make_unique<FunctionalNotificationObserverT<model::MosaicSupplyChangeNotification>>(
				"MosaicSupplyChangeObserver",
				[](const auto& notification, const ObserverContext& context) {
					auto& accountStateCache = context.Cache.sub<cache::AccountStateCache>();
					auto& cache = context.Cache.sub<cache::MosaicCache>();

					auto pState = accountStateCache.findAccount(notification.Signer);
					auto& entry = cache.get(notification.MosaicId);
					if (ShouldIncrease(context.Mode, notification.Direction)) {
						pState->Balances.credit(notification.MosaicId, notification.Delta);
						entry.increaseSupply(notification.Delta);
					} else {
						pState->Balances.debit(notification.MosaicId, notification.Delta);
						entry.decreaseSupply(notification.Delta);
					}
				});
	}
}}
