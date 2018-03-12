#include "Validators.h"

namespace catapult { namespace validators {

	using Notification = model::TransferMessageNotification;

	DECLARE_STATELESS_VALIDATOR(TransferMessage, Notification)(uint16_t maxMessageSize) {
		return MAKE_STATELESS_VALIDATOR(TransferMessage, [maxMessageSize](const auto& notification) {
			return notification.MessageSize > maxMessageSize ? Failure_Transfer_Message_Too_Large : ValidationResult::Success;
		});
	}
}}
