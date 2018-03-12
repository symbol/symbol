#include "Validators.h"
#include "src/cache/HashCache.h"
#include "catapult/state/TimestampedHash.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::TransactionNotification;

	DEFINE_STATEFUL_VALIDATOR(UniqueTransactionHash, [](const auto& notification, const ValidatorContext& context) {
		const auto& hashCache = context.Cache.sub<cache::HashCache>();
		return hashCache.contains(state::TimestampedHash(notification.Deadline, notification.TransactionHash))
				? Failure_Hash_Exists
				: ValidationResult::Success;
	});
}}
