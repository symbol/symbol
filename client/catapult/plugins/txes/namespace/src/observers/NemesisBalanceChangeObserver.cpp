#include "Observers.h"
#include "catapult/cache/AccountStateCache.h"

namespace catapult { namespace observers {

	NotificationObserverPointerT<model::BalanceTransferNotification> CreateNemesisBalanceChangeObserver() {
		return std::make_unique<FunctionalNotificationObserverT<model::BalanceTransferNotification>>(
				"NemesisBalanceChangeObserver",
				[](const auto& notification, const ObserverContext& context) {
					// only allow execution on nemesis, which cannot be rolled back
					if (Height(1) != context.Height || NotifyMode::Commit != context.Mode)
						return;

					// the nemesis loader credits the nemesis account with sufficient mosaics to cover all outflows
					// but, when full namespace/mosaic support is enabled, those credits need to be undone because nemesis
					// should derive its mosaic supplies from mosaic supply transactions
					auto& cache = context.Cache.sub<cache::AccountStateCache>();
					auto pSenderState = cache.findAccount(notification.Sender);
					pSenderState->Balances.debit(notification.MosaicId, notification.Amount);
				});
	}
}}
