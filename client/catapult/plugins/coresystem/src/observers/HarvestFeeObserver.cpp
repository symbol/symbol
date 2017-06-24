#include "Observers.h"
#include "src/cache/AccountStateCache.h"

namespace catapult { namespace observers {

	NotificationObserverPointerT<model::BlockNotification> CreateHarvestFeeObserver() {
		return std::make_unique<FunctionalNotificationObserverT<model::BlockNotification>>(
				"HarvestFeeObserver",
				[](const auto& notification, const ObserverContext& context) {
					// credit the harvester
					auto& cache = context.Cache.sub<cache::AccountStateCache>();
					auto pHarvesterState = cache.findAccount(notification.Signer);
					if (NotifyMode::Commit == context.Mode)
						pHarvesterState->Balances.credit(Xem_Id, notification.TotalFee);
					else
						pHarvesterState->Balances.debit(Xem_Id, notification.TotalFee);
				});
	}
}}
