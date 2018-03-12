#include "Validators.h"
#include "src/cache/SecretLockInfoCache.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::SecretLockNotification;

	DEFINE_STATEFUL_VALIDATOR(SecretCacheUnique, [](const auto& notification, const auto& context) {
		const auto& cache = context.Cache.template sub<cache::SecretLockInfoCache>();
		return cache.contains(notification.Secret)
				? Failure_Lock_Hash_Exists
				: ValidationResult::Success;
	})
}}
