#include "Validators.h"
#include "src/cache/AccountStateCache.h"
#include "src/cache/ImportanceView.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::BlockNotification;

	stateful::NotificationValidatorPointerT<Notification> CreateEligibleHarvesterValidator(Amount minHarvesterBalance) {
		return std::make_unique<stateful::FunctionalNotificationValidatorT<Notification>>(
				"EligibleHarvesterValidator",
				[minHarvesterBalance](const auto& notification, const ValidatorContext& context) {
					cache::ImportanceView view(context.Cache.sub<cache::AccountStateCache>());
					return view.canHarvest(notification.Signer, context.Height, minHarvesterBalance)
							? ValidationResult::Success
							: Failure_Core_Block_Harvester_Ineligible;
				});
	}
}}
