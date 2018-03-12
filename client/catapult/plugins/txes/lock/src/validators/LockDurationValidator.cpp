#include "Validators.h"

#define DEFINE_LOCK_DURATION_VALIDATOR(VALIDATOR_NAME, NOTIFICATION_TYPE) \
	DECLARE_STATELESS_VALIDATOR(VALIDATOR_NAME, NOTIFICATION_TYPE)(BlockDuration maxDuration) { \
		using ValidatorType = stateless::FunctionalNotificationValidatorT<NOTIFICATION_TYPE>; \
		return std::make_unique<ValidatorType>(#VALIDATOR_NAME "Validator", [maxDuration](const auto& notification) { \
			return notification.Duration <= maxDuration ? ValidationResult::Success : Failure_Lock_Invalid_Duration; \
		}); \
	}

namespace catapult { namespace validators {

	DEFINE_LOCK_DURATION_VALIDATOR(SecretLockDuration, model::SecretLockDurationNotification)
	DEFINE_LOCK_DURATION_VALIDATOR(HashLockDuration, model::HashLockDurationNotification)
}}
