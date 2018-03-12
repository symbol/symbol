#include "Validators.h"
#include "src/model/IdGenerator.h"
#include "catapult/utils/Hashers.h"

namespace catapult { namespace validators {

	using Notification = model::RootNamespaceNotification;

	DECLARE_STATELESS_VALIDATOR(RootNamespace, Notification)(BlockDuration maxDuration) {
		return MAKE_STATELESS_VALIDATOR(RootNamespace, [maxDuration](const auto& notification) {
			// note that zero duration is acceptable because it is eternal
			if (maxDuration < notification.Duration)
				return Failure_Namespace_Invalid_Duration;

			return ValidationResult::Success;
		});
	}
}}
