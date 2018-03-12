#include "Validators.h"
#include "src/cache/SecretLockInfoCache.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::ProofPublicationNotification;

	DEFINE_STATEFUL_VALIDATOR(Proof, [](const auto& notification, const auto& context) {
		const auto& cache = context.Cache.template sub<cache::SecretLockInfoCache>();
		if (!cache.contains(notification.Secret))
			return Failure_Lock_Unknown_Secret;

		if (!cache.isActive(notification.Secret, context.Height))
			return Failure_Lock_Inactive_Secret;

		const auto& lockInfo = cache.get(notification.Secret);
		if (lockInfo.HashAlgorithm != notification.HashAlgorithm)
			return Failure_Lock_Hash_Algorithm_Mismatch;

		return model::LockStatus::Used == lockInfo.Status ? Failure_Lock_Secret_Already_Used : ValidationResult::Success;
	})
}}
