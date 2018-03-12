#include "Validators.h"
#include "catapult/model/VerifiableEntity.h"

namespace catapult { namespace validators {

	using Notification = model::EntityNotification;

	DECLARE_STATELESS_VALIDATOR(Network, Notification)(model::NetworkIdentifier networkIdentifier) {
		return MAKE_STATELESS_VALIDATOR(Network, [networkIdentifier](const auto& notification) {
			return networkIdentifier == notification.NetworkIdentifier ? ValidationResult::Success : Failure_Core_Wrong_Network;
		});
	}
}}
