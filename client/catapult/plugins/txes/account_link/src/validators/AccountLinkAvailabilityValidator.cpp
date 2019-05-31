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

	using Notification = model::RemoteAccountLinkNotification;

	DEFINE_STATEFUL_VALIDATOR(AccountLinkAvailability, [](const auto& notification, const auto& context) {
		const auto& cache = context.Cache.template sub<cache::AccountStateCache>();
		auto accountStateIter = cache.find(notification.MainAccountKey);
		const auto& accountState = accountStateIter.get();

		if (model::AccountLinkAction::Link == notification.LinkAction) {
			if (state::AccountType::Unlinked != accountState.AccountType)
				return Failure_AccountLink_Link_Already_Exists;
		} else {
			// only main accounts can unlink (not remotes)
			if (state::AccountType::Main != accountState.AccountType)
				return Failure_AccountLink_Link_Does_Not_Exist;

			if (notification.RemoteAccountKey != accountState.LinkedAccountKey)
				return Failure_AccountLink_Unlink_Data_Inconsistency;
		}

		return ValidationResult::Success;
	});
}}
