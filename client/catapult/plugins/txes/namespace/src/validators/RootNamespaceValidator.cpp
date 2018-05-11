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
