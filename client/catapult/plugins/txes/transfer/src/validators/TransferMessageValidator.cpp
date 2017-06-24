#include "Validators.h"

namespace catapult { namespace validators {

	using Notification = model::TransferMessageNotification;

	stateless::NotificationValidatorPointerT<Notification> CreateTransferMessageValidator(uint16_t maxMessageSize) {
		return std::make_unique<stateless::FunctionalNotificationValidatorT<Notification>>(
				"TransferMessageValidator",
				[maxMessageSize](const auto& notification) {
					return notification.MessageSize > maxMessageSize ? Failure_Transfer_Message_Too_Large : ValidationResult::Success;
				});
	}
}}
