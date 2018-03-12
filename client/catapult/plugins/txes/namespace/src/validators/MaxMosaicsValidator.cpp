#include "Validators.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/state/AccountBalances.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	namespace {
		template<typename TKey>
		ValidationResult CheckAccount(uint16_t maxMosaics, MosaicId mosaicId, const TKey& key, const ValidatorContext& context) {
			const auto& accountStateCache = context.Cache.sub<cache::AccountStateCache>();
			const auto& balances = accountStateCache.get(key).Balances;
			if (balances.get(mosaicId) != Amount())
				return ValidationResult::Success;

			return maxMosaics <= balances.size()
					? Failure_Mosaic_Max_Mosaics_Exceeded
					: ValidationResult::Success;
		}
	}

	DECLARE_STATEFUL_VALIDATOR(MaxMosaicsBalanceTransfer, model::BalanceTransferNotification)(uint16_t maxMosaics) {
		using ValidatorType = stateful::FunctionalNotificationValidatorT<model::BalanceTransferNotification>;
		return std::make_unique<ValidatorType>(
				"MaxMosaicsBalanceTransferValidator",
				[maxMosaics](const auto& notification, const auto& context) {
					if (Amount() == notification.Amount)
						return ValidationResult::Success;

					return CheckAccount(maxMosaics, notification.MosaicId, notification.Recipient, context);
				});
	}

	DECLARE_STATEFUL_VALIDATOR(MaxMosaicsSupplyChange, model::MosaicSupplyChangeNotification)(uint16_t maxMosaics) {
		using ValidatorType = stateful::FunctionalNotificationValidatorT<model::MosaicSupplyChangeNotification>;
		return std::make_unique<ValidatorType>(
				"MaxMosaicsSupplyChangeValidator",
				[maxMosaics](const auto& notification, const auto& context) {
					if (model::MosaicSupplyChangeDirection::Decrease == notification.Direction)
						return ValidationResult::Success;

					return CheckAccount(maxMosaics, notification.MosaicId, notification.Signer, context);
				});
	}
}}
