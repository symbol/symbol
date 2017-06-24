#pragma once
#include "NetworkInfo.h"
#include "catapult/types.h"
#include <string>

namespace catapult { namespace model {

	/// Creates an address from an encoded address (\a encoded).
	Address StringToAddress(const std::string& encoded);

	/// Creates an encoded address from an \a address.
	std::string AddressToString(const Address& address);

	/// Creates an address from a public key (\a publicKey) for the network identified by \a networkIdentifier.
	Address PublicKeyToAddress(const Key& publicKey, NetworkIdentifier networkIdentifier);

	/// Returns a value indicating whether or not the given \a address is valid for the network identified by \a networkIdentifier.
	bool IsValidAddress(const Address& address, NetworkIdentifier networkIdentifier);

	/// Returns a value indicating whether or not the given \a encoded address is valid for the
	/// network identified by \a networkIdentifier.
	bool IsValidEncodedAddress(const std::string& encoded, NetworkIdentifier networkIdentifier);
}}
