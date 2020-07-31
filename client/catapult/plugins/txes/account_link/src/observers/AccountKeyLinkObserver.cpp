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

#include "Observers.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/observers/ObserverUtils.h"

namespace catapult { namespace observers {

	namespace {
		void ClearLink(state::AccountState& accountState, state::AccountType accountType) {
			accountState.SupplementalPublicKeys.linked().unset();
			accountState.AccountType = accountType;
		}

		void SetLink(state::AccountState& accountState, const Key& linkedPublicKey, state::AccountType accountType) {
			accountState.SupplementalPublicKeys.linked().set(linkedPublicKey);
			accountState.AccountType = accountType;
		}
	}

	DEFINE_OBSERVER(AccountKeyLink, model::RemoteAccountKeyLinkNotification, [](
			const model::RemoteAccountKeyLinkNotification& notification,
			const ObserverContext& context) {
		auto& cache = context.Cache.sub<cache::AccountStateCache>();

		auto mainAccountStateIter = cache.find(notification.MainAccountPublicKey);
		auto& mainAccountState = mainAccountStateIter.get();

		auto remoteAccountStateIter = cache.find(notification.LinkedPublicKey);
		auto& remoteAccountState = remoteAccountStateIter.get();

		if (ShouldLink(notification.LinkAction, context.Mode)) {
			SetLink(mainAccountState, notification.LinkedPublicKey, state::AccountType::Main);
			SetLink(remoteAccountState, notification.MainAccountPublicKey, state::AccountType::Remote);
		} else {
			ClearLink(mainAccountState, state::AccountType::Unlinked);
			ClearLink(remoteAccountState, state::AccountType::Remote_Unlinked);
		}
	})
}}
