#pragma once
#include "Results.h"
#include "catapult/model/Notifications.h"
#include "catapult/validators/ValidatorTypes.h"

namespace catapult { namespace validators {

	/// A validator implementation that applies to all transaction notifications and validates that:
	/// - the entity hash is unique and has not been previously seen
	stateful::NotificationValidatorPointerT<model::TransactionNotification> CreateUniqueTransactionHashValidator();
}}
