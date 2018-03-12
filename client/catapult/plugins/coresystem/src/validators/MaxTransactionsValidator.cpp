#include "Validators.h"

namespace catapult { namespace validators {

	using Notification = model::BlockNotification;

	DECLARE_STATELESS_VALIDATOR(MaxTransactions, Notification)(uint32_t maxTransactions) {
		return MAKE_STATELESS_VALIDATOR(MaxTransactions, [maxTransactions](const auto& notification) {
			return notification.NumTransactions <= maxTransactions
					? ValidationResult::Success
					: Failure_Core_Too_Many_Transactions;
		});
	}
}}
