#include "Validators.h"
#include "catapult/model/VerifiableEntity.h"

namespace catapult { namespace validators {

	using Notification = model::EntityNotification;

	stateless::NotificationValidatorPointerT<Notification> CreateNetworkValidator(model::NetworkIdentifier networkIdentifier) {
		return std::make_unique<stateless::FunctionalNotificationValidatorT<Notification>>(
				"NetworkValidator",
				[networkIdentifier](const auto& notification) {
					return networkIdentifier == notification.NetworkIdentifier ? ValidationResult::Success : Failure_Core_Wrong_Network;
				});
	}
}}
