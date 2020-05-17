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
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/model/Address.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::AccountPublicKeyNotification;

	DEFINE_STATEFUL_VALIDATOR(PublicKey, [](const Notification& notification, const ValidatorContext& context) {
		auto address = model::PublicKeyToAddress(notification.PublicKey, context.Network.Identifier);
		auto accountStateIter = context.Cache.sub<cache::AccountStateCache>().find(address);

		if (accountStateIter.tryGet()) {
			const auto& accountState = accountStateIter.get();
			if (Height() != accountState.PublicKeyHeight && notification.PublicKey != accountState.PublicKey)
				return Failure_Core_Address_Collision;
		}

		return ValidationResult::Success;
	});
}}
