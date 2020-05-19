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

#include "AccountStateCacheUtils.h"
#include "AccountStateCacheDelta.h"
#include "AccountStateCacheView.h"

namespace catapult { namespace cache {

	namespace {
		template<typename TAccountStateCache, typename TAccountState>
		void ProcessForwardedAccountStateT(TAccountStateCache& cache, const Address& address, const consumer<TAccountState&>& action) {
			auto accountStateIter = cache.find(address);
			auto& accountState = accountStateIter.get();

			if (state::AccountType::Remote != accountState.AccountType) {
				action(accountState);
				return;
			}

			auto linkedAccountStateIter = cache.find(state::GetLinkedPublicKey(accountState));
			auto& linkedAccountState = linkedAccountStateIter.get();

			// this check is merely a precaution and will only fire if there is a bug that has corrupted links
			RequireLinkedRemoteAndMainAccounts(accountState, linkedAccountState);
			action(linkedAccountState);
		}
	}

	void ProcessForwardedAccountState(
			AccountStateCacheDelta& cache,
			const Address& address,
			const consumer<state::AccountState&>& action) {
		ProcessForwardedAccountStateT(cache, address, action);
	}

	void ProcessForwardedAccountState(
			const ReadOnlyAccountStateCache& cache,
			const Address& address,
			const consumer<const state::AccountState&>& action) {
		ProcessForwardedAccountStateT(cache, address, action);
	}
}}
