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

namespace catapult { namespace validators {

	namespace {
		const auto& GetKeyAccessor(const state::AccountState& accountState, const model::VotingKeyLinkNotification&) {
			return accountState.SupplementalAccountKeys.votingPublicKey();
		}

		const auto& GetKeyAccessor(const state::AccountState& accountState, const model::VrfKeyLinkNotification&) {
			return accountState.SupplementalAccountKeys.vrfPublicKey();
		}

		template<typename TNotification>
		ValidationResult CheckLink(const TNotification& notification, const ValidatorContext& context) {
			const auto& cache = context.Cache.sub<cache::AccountStateCache>();
			auto accountStateIter = cache.find(notification.MainAccountPublicKey);
			const auto& accountState = accountStateIter.get();

			const auto& keyAccessor = GetKeyAccessor(accountState, notification);
			if (model::LinkAction::Link == notification.LinkAction) {
				if (keyAccessor)
					return Failure_Core_Link_Already_Exists;
			} else {
				if (!keyAccessor || notification.LinkedPublicKey != keyAccessor.get())
					return Failure_Core_Inconsistent_Unlink_Data;
			}

			return ValidationResult::Success;
		}
	}

	DEFINE_STATEFUL_VALIDATOR_WITH_TYPE(VotingKeyLink, model::VotingKeyLinkNotification, CheckLink<model::VotingKeyLinkNotification>)
	DEFINE_STATEFUL_VALIDATOR_WITH_TYPE(VrfKeyLink, model::VrfKeyLinkNotification, CheckLink<model::VrfKeyLinkNotification>)
}}
