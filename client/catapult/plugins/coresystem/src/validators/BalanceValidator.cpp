#include "Validators.h"
#include "src/cache/AccountStateCache.h"
#include "catapult/model/Address.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::BalanceTransferNotification;

	namespace {
		std::shared_ptr<const state::AccountState> FindAccount(const cache::ReadOnlyAccountStateCache& cache, const Key& publicKey) {
			auto pState = cache.findAccount(publicKey);

			// if state could not be accessed by public key, try searching by address
			if (!pState)
				pState = cache.findAccount(model::PublicKeyToAddress(publicKey, cache.networkIdentifier()));

			return pState;
		}
	}

	stateful::NotificationValidatorPointerT<Notification> CreateBalanceValidator() {
		return std::make_unique<stateful::FunctionalNotificationValidatorT<Notification>>(
				"BalanceValidator",
				[](const auto& notification, const ValidatorContext& context) {
					const auto& cache = context.Cache.sub<cache::AccountStateCache>();
					auto pState = FindAccount(cache, notification.Sender);
					return pState && pState->Balances.get(notification.MosaicId) >= notification.Amount
							? ValidationResult::Success
							: Failure_Core_Insufficient_Balance;
				});
	}
}}
