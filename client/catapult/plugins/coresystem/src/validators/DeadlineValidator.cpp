#include "Validators.h"
#include "catapult/validators/ValidatorContext.h"
#include "catapult/types.h"

namespace catapult { namespace validators {

	using Notification = model::TransactionNotification;

	namespace {
		auto ValidateTransactionDeadline(Timestamp timestamp, Timestamp deadline, const utils::TimeSpan& maxTransactionLifetime) {
			if (timestamp > deadline)
				return Failure_Core_Past_Deadline;

			if (deadline > timestamp + maxTransactionLifetime)
				return Failure_Core_Future_Deadline;

			return ValidationResult::Success;
		}
	}

	stateful::NotificationValidatorPointerT<Notification> CreateDeadlineValidator(const utils::TimeSpan& maxTransactionLifetime) {
		return std::make_unique<stateful::FunctionalNotificationValidatorT<Notification>>(
				"DeadlineValidator",
				[maxTransactionLifetime](const auto& notification, const auto& context) {
					return ValidateTransactionDeadline(context.BlockTime, notification.Deadline, maxTransactionLifetime);
				});
	}
}}
