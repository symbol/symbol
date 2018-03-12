#include "Validators.h"
#include "src/cache/MosaicCache.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::MosaicSupplyChangeNotification;

	DECLARE_STATEFUL_VALIDATOR(MosaicSupplyChangeAllowed, Notification)(Amount maxDivisibleUnits) {
		return MAKE_STATEFUL_VALIDATOR(MosaicSupplyChangeAllowed, [maxDivisibleUnits](
				const auto& notification,
				const ValidatorContext& context) {
			// notice that MosaicChangeAllowedValidator is required to run first, so both mosaic and owning account must exist
			const auto& cache = context.Cache.sub<cache::MosaicCache>();
			const auto& entry = cache.get(notification.MosaicId);

			const auto& accountStateCache = context.Cache.sub<cache::AccountStateCache>();
			const auto& accountState = accountStateCache.get(notification.Signer);
			auto ownerAmount = accountState.Balances.get(notification.MosaicId);

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
