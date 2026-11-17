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

#include "KeyLinkValidators.h"
#include "catapult/crypto/CryptoUtils.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::VrfKeyLinkNotification;

	DECLARE_STATEFUL_VALIDATOR(VrfKeyLinkPublicKey, Notification)(Height forkHeight) {
		return MAKE_STATEFUL_VALIDATOR(VrfKeyLinkPublicKey, ([forkHeight](
				const Notification& notification,
				const ValidatorContext& context) {
			// enforce only at and after the fork height so historical link transactions replay unchanged
			if (context.Height < forkHeight)
				return ValidationResult::Success;

			// an unlink does not introduce a new key, so only a link needs its linked key checked
			if (model::LinkAction::Link != notification.LinkAction)
				return ValidationResult::Success;

			return crypto::IsValidVrfPublicKey(notification.LinkedPublicKey)
					? ValidationResult::Success
					: Failure_Core_Invalid_Vrf_Key;
		}));
	}
}}
