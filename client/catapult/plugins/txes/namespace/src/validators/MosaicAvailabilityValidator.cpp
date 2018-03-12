#include "Validators.h"
#include "src/cache/MosaicCache.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::MosaicDefinitionNotification;

	DEFINE_STATEFUL_VALIDATOR(MosaicAvailability, [](const auto& notification, const ValidatorContext& context) {
		const auto& cache = context.Cache.sub<cache::MosaicCache>();

		// - always allow a new mosaic
		if (!cache.isActive(notification.MosaicId, context.Height))
			return ValidationResult::Success;

		// - disallow conflicting parent namespace
		const auto& entry = cache.get(notification.MosaicId);
		if (entry.namespaceId() != notification.ParentId)
			return Failure_Mosaic_Parent_Id_Conflict;

		// - disallow a noop modification
		if (notification.Properties == entry.definition().properties())
			return Failure_Mosaic_Modification_No_Changes;

		// - only allow a modification if signer contains full balance
		const auto& accountStateCache = context.Cache.sub<cache::AccountStateCache>();
		auto pAccountState = accountStateCache.tryGet(notification.Signer);
		return !pAccountState || entry.supply() != pAccountState->Balances.get(notification.MosaicId)
				? Failure_Mosaic_Modification_Disallowed
				: ValidationResult::Success;
	});
}}
