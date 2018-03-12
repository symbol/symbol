#include "Validators.h"
#include "src/cache/HashLockInfoCache.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::HashLockNotification;

	DEFINE_STATEFUL_VALIDATOR(HashCacheUnique, [](const auto& notification, const auto& context) {
		const auto& cache = context.Cache.template sub<cache::HashLockInfoCache>();
		return cache.contains(notification.Hash)
				? Failure_Lock_Hash_Exists
				: ValidationResult::Success;
	})
}}
