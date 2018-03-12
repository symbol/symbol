#include "Observers.h"
#include "catapult/cache_core/AccountStateCache.h"

namespace catapult { namespace observers {

	DEFINE_OBSERVER(HarvestFee, model::BlockNotification, [](const auto& notification, const ObserverContext& context) {
		// credit the harvester
		auto& cache = context.Cache.sub<cache::AccountStateCache>();
		auto& harvesterState = cache.get(notification.Signer);
		if (NotifyMode::Commit == context.Mode)
			harvesterState.Balances.credit(Xem_Id, notification.TotalFee);
		else
			harvesterState.Balances.debit(Xem_Id, notification.TotalFee);
	});
}}
