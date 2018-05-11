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
#include "AccountBalances.h"
#include "AccountImportance.h"

namespace catapult { namespace state {

	/// Account state data.
	struct AccountState {
	public:
		/// Creates an account state from an \a address and a height (\a addressHeight).
		explicit AccountState(const catapult::Address& address, Height addressHeight)
				: Address(address)
				, AddressHeight(addressHeight)
				, PublicKeyHeight(0)
		{}

	public:
		/// Address of an account.
		catapult::Address Address;

		/// Height at which address has been obtained.
		Height AddressHeight;

		/// Public key of an account. Present if PublicKeyHeight > 0.
		Key PublicKey;

		/// Height at which public key has been obtained.
		Height PublicKeyHeight;

		/// Importance information of the account.
		AccountImportance ImportanceInfo;

		/// Balances of an account.
		AccountBalances Balances;
	};
}}
