/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#include "Validators.h"

#define DEFINE_LOCK_DURATION_VALIDATOR(VALIDATOR_NAME, NOTIFICATION_TYPE) \
	DECLARE_STATELESS_VALIDATOR(VALIDATOR_NAME, NOTIFICATION_TYPE)(BlockDuration maxDuration) { \
		using ValidatorType = stateless::FunctionalNotificationValidatorT<NOTIFICATION_TYPE>; \
		return std::make_unique<ValidatorType>(#VALIDATOR_NAME "Validator", [maxDuration](const auto& notification) { \
			return notification.Duration <= maxDuration ? ValidationResult::Success : Failure_Lock_Invalid_Duration; \
		}); \
	}

namespace catapult { namespace validators {

	DEFINE_LOCK_DURATION_VALIDATOR(SecretLockDuration, model::SecretLockDurationNotification)
	DEFINE_LOCK_DURATION_VALIDATOR(HashLockDuration, model::HashLockDurationNotification)
}}
