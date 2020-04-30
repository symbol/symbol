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

#pragma once
#include "src/model/KeyLinkNotifications.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "tests/test/nodeps/Random.h"

namespace catapult { namespace test {

	/// Basic voting key link test traits.
	struct BasicVotingKeyLinkTestTraits {
	public:
		using KeyType = VotingKey;
		using NotificationType = model::VotingKeyLinkNotification;

	public:
		/// Gets the key accessor from \a accountState.
		static auto& GetKeyAccessor(state::AccountState& accountState) {
			return accountState.SupplementalAccountKeys.votingPublicKey();
		}
	};

	/// Basic vrf key link test traits.
	struct BasicVrfKeyLinkTestTraits {
	public:
		using KeyType = Key;
		using NotificationType = model::VrfKeyLinkNotification;

	public:
		/// Gets the key accessor from \a accountState.
		static auto& GetKeyAccessor(state::AccountState& accountState) {
			return accountState.SupplementalAccountKeys.vrfPublicKey();
		}
	};

	/// Adds a random account to \a cacheDelta with specified linked public key (\a linkedPublicKey).
	template<typename TTraits>
	auto AddAccountWithLink(cache::CatapultCacheDelta& cacheDelta, const typename TTraits::KeyType& linkedPublicKey) {
		auto& accountStateCacheDelta = cacheDelta.sub<cache::AccountStateCache>();

		auto mainAccountPublicKey = GenerateRandomByteArray<Key>();
		accountStateCacheDelta.addAccount(mainAccountPublicKey, Height(1));
		auto mainAccountStateIter = accountStateCacheDelta.find(mainAccountPublicKey);

		if (typename TTraits::KeyType() != linkedPublicKey)
			TTraits::GetKeyAccessor(mainAccountStateIter.get()).set(linkedPublicKey);

		return mainAccountPublicKey;
	}

	/// Adds a random account to \a cache with specified linked public key (\a linkedPublicKey).
	template<typename TTraits>
	auto AddAccountWithLink(cache::CatapultCache& cache, const typename TTraits::KeyType& linkedPublicKey) {
		auto cacheDelta = cache.createDelta();
		auto mainAccountPublicKey = AddAccountWithLink<TTraits>(cacheDelta, linkedPublicKey);
		cache.commit(Height(1));
		return mainAccountPublicKey;
	}
}}
