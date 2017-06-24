#include "Validators.h"
#include "src/cache/HashCache.h"
#include "src/state/TimestampedHash.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::TransactionNotification;

	stateful::NotificationValidatorPointerT<Notification> CreateUniqueTransactionHashValidator() {
		return std::make_unique<stateful::FunctionalNotificationValidatorT<Notification>>(
				"UniqueTransactionHashValidator",
				[](const auto& notification, const ValidatorContext& context) {
					const auto& hashCache = context.Cache.sub<cache::HashCache>();
					return hashCache.contains(state::TimestampedHash(notification.Deadline, notification.EntityHash))
							? Failure_Hash_Exists
							: ValidationResult::Success;
				});
	}
}}
