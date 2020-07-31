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
#include "catapult/model/Address.h"
#include "catapult/model/ResolverContext.h"

namespace catapult { namespace validators {

	DECLARE_STATELESS_VALIDATOR(ZeroAddress, model::AccountAddressNotification)(model::NetworkIdentifier networkIdentifier) {
		using Notification = model::AccountAddressNotification;

		auto zeroAddress = model::PublicKeyToAddress(Key(), networkIdentifier);
		return MAKE_STATELESS_VALIDATOR_WITH_TYPE(ZeroAddress, Notification, [zeroAddress](const Notification& notification) {
			// copy Address from unresolved to resolved in order to check it against (resolved) zeroAddress
			// if it needs to be resolved, it will never match (due to different resolved bit flag)
			return zeroAddress == notification.Address.resolved(model::ResolverContext())
					? Failure_Core_Zero_Address
					: ValidationResult::Success;
		});
	}

	DEFINE_STATELESS_VALIDATOR_WITH_TYPE(ZeroPublicKey, model::AccountPublicKeyNotification, [](
			const model::AccountPublicKeyNotification& notification) {
		return Key() == notification.PublicKey ? Failure_Core_Zero_Public_Key : ValidationResult::Success;
	})
}}
