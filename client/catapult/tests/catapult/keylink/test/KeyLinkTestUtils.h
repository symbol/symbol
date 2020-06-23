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
#include "catapult/cache/CatapultCacheDelta.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	/// Adds a random account to \a cacheDelta with specified linked public key (\a publicKey).
	inline Key AddAccountWithLink(cache::CatapultCacheDelta& cacheDelta, const Key& publicKey) {
		auto& accountStateCacheDelta = cacheDelta.sub<cache::AccountStateCache>();

		auto mainAccountPublicKey = GenerateRandomByteArray<Key>();
		accountStateCacheDelta.addAccount(mainAccountPublicKey, Height(1));
		auto mainAccountStateIter = accountStateCacheDelta.find(mainAccountPublicKey);

		if (Key() != publicKey)
			mainAccountStateIter.get().SupplementalPublicKeys.linked().set(publicKey);

		return mainAccountPublicKey;
	}

	/// Adds a random account to \a cacheDelta with specified voting public keys (\a publicKeys).
	inline Key AddAccountWithMultiVotingLinks(
			cache::CatapultCacheDelta& cacheDelta,
			const std::vector<model::PinnedVotingKey>& publicKeys) {
		auto& accountStateCacheDelta = cacheDelta.sub<cache::AccountStateCache>();

		auto mainAccountPublicKey = GenerateRandomByteArray<Key>();
		accountStateCacheDelta.addAccount(mainAccountPublicKey, Height(1));
		auto mainAccountStateIter = accountStateCacheDelta.find(mainAccountPublicKey);

		for (const auto& publicKey : publicKeys)
			mainAccountStateIter.get().SupplementalPublicKeys.voting().add(publicKey);

		return mainAccountPublicKey;
	}
}}
