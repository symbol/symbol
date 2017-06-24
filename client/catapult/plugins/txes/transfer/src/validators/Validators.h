#pragma once
#include "Results.h"
#include "src/model/TransferNotifications.h"
#include "catapult/validators/ValidatorTypes.h"

namespace catapult { namespace validators {

	/// A validator implementation that applies to transfer message notifications and validates that:
	/// - messages have a maximum message size of \a maxMessageSize
	stateless::NotificationValidatorPointerT<model::TransferMessageNotification> CreateTransferMessageValidator(
			uint16_t maxMessageSize);

	/// A validator implementation that applies to transfer mosaics notifications and validates that:
	/// - mosaics are ordered
	stateless::NotificationValidatorPointerT<model::TransferMosaicsNotification> CreateTransferMosaicsValidator();
}}
