#include "Validators.h"
#include "src/cache/HashLockInfoCache.h"
#include "plugins/txes/aggregate/src/model/AggregateEntityType.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::TransactionNotification;

	DEFINE_STATEFUL_VALIDATOR(AggregateHashPresent, [](const auto& notification, const auto& context) {
		if (model::Entity_Type_Aggregate_Bonded != notification.TransactionType)
			return ValidationResult::Success;

		const auto& cache = context.Cache.template sub<cache::HashLockInfoCache>();
		if (!cache.contains(notification.TransactionHash))
			return Failure_Lock_Hash_Does_Not_Exist;

		if (!cache.isActive(notification.TransactionHash, context.Height))
			return Failure_Lock_Inactive_Hash;

		const auto& lockInfo = cache.get(notification.TransactionHash);
		return model::LockStatus::Unused == lockInfo.Status
				? ValidationResult::Success
				: Failure_Lock_Hash_Already_Used;
	})
}}
