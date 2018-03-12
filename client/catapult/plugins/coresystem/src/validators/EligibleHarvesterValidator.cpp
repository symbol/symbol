#include "Validators.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/cache_core/ImportanceView.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::BlockNotification;

	DECLARE_STATEFUL_VALIDATOR(EligibleHarvester, Notification)(Amount minHarvesterBalance) {
		return MAKE_STATEFUL_VALIDATOR(EligibleHarvester, [minHarvesterBalance](
				const auto& notification,
				const ValidatorContext& context) {
			cache::ImportanceView view(context.Cache.sub<cache::AccountStateCache>());
			return view.canHarvest(notification.Signer, context.Height, minHarvesterBalance)
					? ValidationResult::Success
					: Failure_Core_Block_Harvester_Ineligible;
		});
	}
}}
