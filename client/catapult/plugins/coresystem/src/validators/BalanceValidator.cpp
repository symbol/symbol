#include "Validators.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/model/Address.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using BalanceTransferNotification = model::BalanceTransferNotification;
	using BalanceReserveNotification = model::BalanceReserveNotification;

	namespace {
		const state::AccountState* FindAccount(const cache::ReadOnlyAccountStateCache& cache, const Key& publicKey) {
			auto pAccountState = cache.tryGet(publicKey);

			// if state could not be accessed by public key, try searching by address
			if (!pAccountState)
				pAccountState = cache.tryGet(model::PublicKeyToAddress(publicKey, cache.networkIdentifier()));

			return pAccountState;
		}

		template<typename TNotification>
		ValidationResult CheckBalance(const TNotification& notification, const ValidatorContext& context) {
			const auto& cache = context.Cache.sub<cache::AccountStateCache>();
			auto pAccountState = FindAccount(cache, notification.Sender);
			return pAccountState && pAccountState->Balances.get(notification.MosaicId) >= notification.Amount
					? ValidationResult::Success
					: Failure_Core_Insufficient_Balance;
		}
	}

	DEFINE_STATEFUL_VALIDATOR_WITH_TYPE(BalanceTransfer, BalanceTransferNotification, CheckBalance<BalanceTransferNotification>);
	DEFINE_STATEFUL_VALIDATOR_WITH_TYPE(BalanceReserve, BalanceReserveNotification, CheckBalance<BalanceReserveNotification>);
}}
