/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#pragma once
#include "catapult/validators/ValidationResult.h"

namespace catapult { namespace validators {

/// Defines a lock duration validator with name \a VALIDATOR_NAME for notification \a NOTIFICATION_TYPE
/// that returns \a FAILURE_RESULT on failure.
#define DEFINE_LOCK_DURATION_VALIDATOR(VALIDATOR_NAME, NOTIFICATION_TYPE, FAILURE_RESULT) \
	DECLARE_STATELESS_VALIDATOR(VALIDATOR_NAME, NOTIFICATION_TYPE)(BlockDuration maxDuration) { \
		using ValidatorType = stateless::FunctionalNotificationValidatorT<NOTIFICATION_TYPE>; \
		return std::make_unique<ValidatorType>(#VALIDATOR_NAME "Validator", [maxDuration](const auto& notification) { \
			return BlockDuration(0) != notification.Duration && notification.Duration <= maxDuration \
					? ValidationResult::Success \
					: FAILURE_RESULT; \
		}); \
	}
}}
