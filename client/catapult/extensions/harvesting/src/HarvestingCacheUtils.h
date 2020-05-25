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
#include "HarvestingObservers.h"

namespace catapult { namespace cache { class AccountStateCacheDelta; } }

namespace catapult { namespace harvesting {

	/// Identifiers of accounts processed by harvesting account observers.
	struct HarvestingAffectedAccounts {
		/// Affected addresses.
		RefCountedAccountIdentifiers<Address> Addresses;

		/// Affected public keys.
		RefCountedAccountIdentifiers<Key> PublicKeys;
	};

	/// Updates \a accountStateCacheDelta to preserve all accounts referenced in \a accounts at \a height.
	void PreserveAllAccounts(
			cache::AccountStateCacheDelta& accountStateCacheDelta,
			const HarvestingAffectedAccounts& accounts,
			Height height);
}}
