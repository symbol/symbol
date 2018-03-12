#include "Observers.h"
#include "catapult/cache_core/AccountStateCache.h"

namespace catapult { namespace observers {

	namespace {
		void Transfer(state::AccountState& debitState, state::AccountState& creditState, MosaicId mosaicId, Amount amount) {
			debitState.Balances.debit(mosaicId, amount);
			creditState.Balances.credit(mosaicId, amount);
		}
	}

	DEFINE_OBSERVER(Balance, model::BalanceTransferNotification, [](const auto& notification, const ObserverContext& context) {
		auto& cache = context.Cache.sub<cache::AccountStateCache>();
		auto& senderState = cache.get(notification.Sender);
		auto& recipientState = cache.get(notification.Recipient);

		if (NotifyMode::Commit == context.Mode)
			Transfer(senderState, recipientState, notification.MosaicId, notification.Amount);
		else
			Transfer(recipientState, senderState, notification.MosaicId, notification.Amount);
	});
}}
