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
