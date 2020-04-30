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

#include "KeyLinkObservers.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/observers/ObserverUtils.h"

namespace catapult { namespace observers {

	namespace {
		auto& GetKeyAccessor(state::AccountState& accountState, const model::VotingKeyLinkNotification&) {
			return accountState.SupplementalAccountKeys.votingPublicKey();
		}

		auto& GetKeyAccessor(state::AccountState& accountState, const model::VrfKeyLinkNotification&) {
			return accountState.SupplementalAccountKeys.vrfPublicKey();
		}

		template<typename TNotification>
		void KeyLinkObserverHandler(const ObserverContext& context, const TNotification& notification) {
			auto& cache = context.Cache.sub<cache::AccountStateCache>();
			auto accountStateIter = cache.find(notification.MainAccountPublicKey);
			auto& accountState = accountStateIter.get();

			auto& keyAccessor = GetKeyAccessor(accountState, notification);
			if (ShouldLink(notification.LinkAction, context.Mode))
				keyAccessor.set(notification.LinkedPublicKey);
			else
				keyAccessor.unset();
		}
	}

#define DEFINE_KEY_LINK_OBSERVER(OBSERVER_NAME, NOTIFICATION) \
	DEFINE_OBSERVER(OBSERVER_NAME, NOTIFICATION, [](const auto& notification, const ObserverContext& context) { \
		KeyLinkObserverHandler(context, notification); \
	})

	DEFINE_KEY_LINK_OBSERVER(VotingKeyLink, model::VotingKeyLinkNotification)
	DEFINE_KEY_LINK_OBSERVER(VrfKeyLink, model::VrfKeyLinkNotification)
}}
