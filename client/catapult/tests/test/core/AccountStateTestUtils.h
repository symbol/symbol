#pragma once
#include "catapult/state/AccountState.h"
#include <vector>

namespace catapult { namespace test {

	/// Account states collection.
	using AccountStates = std::vector<std::shared_ptr<state::AccountState>>;

	/// Fill \a state with pseudo-random importances and \a numMosaics mosaics.
	void RandomFillAccountData(uint64_t seed, state::AccountState& state, size_t numMosaics = 10u);

	/// Asserts that account state \a actual is equal to \a expected.
	void AssertEqual(const state::AccountState& expected, const state::AccountState& actual);

	/// Creates an account state at \a height.
	/// \note The account state has no valid public key.
	std::shared_ptr<state::AccountState> CreateAccountStateWithoutPublicKey(uint64_t height);

	/// Creates \a count account states with successive public keys starting at \c 1.
	AccountStates CreateAccountStates(size_t count);
}}
