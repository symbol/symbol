#include "Observers.h"
#include "src/cache/AccountStateCache.h"

namespace catapult { namespace observers {

	namespace {
		void Transfer(state::AccountState& debitState, state::AccountState& creditState, MosaicId mosaicId, Amount amount) {
			debitState.Balances.debit(mosaicId, amount);
			creditState.Balances.credit(mosaicId, amount);
		}
	}

	NotificationObserverPointerT<model::BalanceTransferNotification> CreateBalanceObserver() {
		return std::make_unique<FunctionalNotificationObserverT<model::BalanceTransferNotification>>(
				"BalanceObserver",
				[](const auto& notification, const ObserverContext& context) {
					auto& cache = context.Cache.sub<cache::AccountStateCache>();
					auto pSenderState = cache.findAccount(notification.Sender);
					auto pRecipientState = cache.findAccount(notification.Recipient);

					if (NotifyMode::Commit == context.Mode)
						Transfer(*pSenderState, *pRecipientState, notification.MosaicId, notification.Amount);
					else
						Transfer(*pRecipientState, *pSenderState, notification.MosaicId, notification.Amount);
				});
	}
}}
