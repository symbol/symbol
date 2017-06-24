#include "Validators.h"
#include "src/cache/MosaicCache.h"
#include "plugins/coresystem/src/cache/AccountStateCache.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::MosaicDefinitionNotification;

	stateful::NotificationValidatorPointerT<Notification> CreateMosaicAvailabilityValidator() {
		return std::make_unique<stateful::FunctionalNotificationValidatorT<Notification>>(
				"MosaicAvailabilityValidator",
				[](const auto& notification, const ValidatorContext& context) {
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
					auto pState = accountStateCache.findAccount(notification.Signer);
					return !pState || entry.supply() != pState->Balances.get(notification.MosaicId)
							? Failure_Mosaic_Modification_Disallowed
							: ValidationResult::Success;
				});
	}
}}
