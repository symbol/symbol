#include "Observers.h"
#include "src/cache/MosaicCache.h"
#include "catapult/cache_core/AccountStateCache.h"

namespace catapult { namespace observers {

	using Notification = model::MosaicSupplyChangeNotification;

	namespace {
		constexpr bool ShouldIncrease(NotifyMode mode, model::MosaicSupplyChangeDirection direction) {
			return
					(NotifyMode::Commit == mode && model::MosaicSupplyChangeDirection::Increase == direction) ||
					(NotifyMode::Rollback == mode && model::MosaicSupplyChangeDirection::Decrease == direction);
		}
	}

	DEFINE_OBSERVER(MosaicSupplyChange, Notification, [](const auto& notification, const ObserverContext& context) {
		auto& accountStateCache = context.Cache.sub<cache::AccountStateCache>();
		auto& cache = context.Cache.sub<cache::MosaicCache>();

		auto& accountState = accountStateCache.get(notification.Signer);
		auto& entry = cache.get(notification.MosaicId);
		if (ShouldIncrease(context.Mode, notification.Direction)) {
			accountState.Balances.credit(notification.MosaicId, notification.Delta);
			entry.increaseSupply(notification.Delta);
		} else {
			accountState.Balances.debit(notification.MosaicId, notification.Delta);
			entry.decreaseSupply(notification.Delta);
		}
	});
}}
