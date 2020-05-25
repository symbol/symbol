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
#include "catapult/state/AccountState.h"
#include <vector>

namespace catapult { namespace test {

	/// Account states collection.
	using AccountStates = std::vector<std::shared_ptr<state::AccountState>>;

	/// Fill \a accountState with pseudo-random importances and \a numMosaics mosaics.
	void RandomFillAccountData(uint64_t seed, state::AccountState& accountState, size_t numMosaics = 10);

	/// Asserts that account state \a actual is equal to \a expected with optional \a message.
	void AssertEqual(const state::AccountState& expected, const state::AccountState& actual, const std::string& message = "");

	/// Creates an account state at \a height.
	/// \note The account state has no valid public key.
	std::shared_ptr<state::AccountState> CreateAccountStateWithoutPublicKey(uint64_t height);

	/// Creates \a count account states with successive public keys starting at \c 1.
	AccountStates CreateAccountStates(size_t count);

	/// Sets all supplemental account keys specified in \a mask to random values in \a accountState.
	void SetRandomSupplementalAccountKeys(state::AccountState& accountState, state::AccountKeys::KeyType mask);

	/// Forcibly clears and sets linked public key in \a accountState to \a linkedPublicKey.
	void ForceSetLinkedPublicKey(state::AccountState& accountState, const Key& linkedPublicKey);
}}
