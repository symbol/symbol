#include "Validators.h"
#include "src/cache/MosaicCache.h"
#include "plugins/coresystem/src/cache/AccountStateCache.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::MosaicSupplyChangeNotification;

	stateful::NotificationValidatorPointerT<Notification> CreateMosaicSupplyChangeAllowedValidator(Amount maxDivisibleUnits) {
		return std::make_unique<stateful::FunctionalNotificationValidatorT<Notification>>(
				"MosaicSupplyChangeAllowedValidator",
				[maxDivisibleUnits](const auto& notification, const ValidatorContext& context) {
					// notice that MosaicChangeAllowedValidator is required to run first, so both mosaic and owning account must exist
					const auto& cache = context.Cache.sub<cache::MosaicCache>();
					const auto& entry = cache.get(notification.MosaicId);

					const auto& accountStateCache = context.Cache.sub<cache::AccountStateCache>();
					auto pState = accountStateCache.findAccount(notification.Signer);
					auto ownerAmount = pState->Balances.get(notification.MosaicId);

					// only allow an "immutable" supply to change if the owner owns full supply
					const auto& properties = entry.definition().properties();
					if (!properties.is(model::MosaicFlags::Supply_Mutable) && ownerAmount != entry.supply())
						return Failure_Mosaic_Supply_Immutable;

					if (model::MosaicSupplyChangeDirection::Decrease == notification.Direction)
						return ownerAmount < notification.Delta ? Failure_Mosaic_Supply_Negative : ValidationResult::Success;

					// check that new supply does not overflow and is not too large
					auto initialSupply = entry.supply();
					auto newSupply = entry.supply() + notification.Delta;
					return newSupply < initialSupply || newSupply > maxDivisibleUnits
							? Failure_Mosaic_Supply_Exceeded
							: ValidationResult::Success;
		});
	}
}}
