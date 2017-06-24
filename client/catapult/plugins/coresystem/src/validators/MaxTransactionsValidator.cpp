#include "Validators.h"

namespace catapult { namespace validators {

	using Notification = model::BlockNotification;

	stateless::NotificationValidatorPointerT<Notification> CreateMaxTransactionsValidator(uint32_t maxTransactions) {
		return std::make_unique<stateless::FunctionalNotificationValidatorT<Notification>>(
				"MaxTransactionsValidator",
				[maxTransactions](const auto& notification) {
					return notification.NumTransactions <= maxTransactions
							? ValidationResult::Success
							: Failure_Core_Too_Many_Transactions;
				});
	}
}}
