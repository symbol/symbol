#include "Validators.h"
#include "catapult/model/Address.h"

namespace catapult { namespace validators {

	using Notification = model::AccountAddressNotification;

	DECLARE_STATELESS_VALIDATOR(Address, Notification)(model::NetworkIdentifier networkIdentifier) {
		return MAKE_STATELESS_VALIDATOR(Address, [networkIdentifier](const auto& notification) {
			auto isValidAddress = IsValidAddress(notification.Address, networkIdentifier);
			return isValidAddress ? ValidationResult::Success : Failure_Core_Invalid_Address;
		});
	}
}}
