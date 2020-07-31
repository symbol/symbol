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
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::NewRemoteAccountNotification;

	DEFINE_STATEFUL_VALIDATOR(NewRemoteAccountAvailability, [](const Notification& notification, const ValidatorContext& context) {
		const auto& cache = context.Cache.sub<cache::AccountStateCache>();
		auto remoteAccountStateIter = cache.find(notification.LinkedPublicKey);
		return !remoteAccountStateIter.tryGet() || state::AccountType::Remote_Unlinked == remoteAccountStateIter.get().AccountType
				? ValidationResult::Success
				: Failure_AccountLink_Remote_Account_Ineligible;
	})
}}
