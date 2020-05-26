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
#include "NetworkIdentifier.h"
#include "catapult/types.h"
#include <string>

namespace catapult { namespace model {

	/// Creates an address from an encoded address (\a encoded).
	Address StringToAddress(const std::string& encoded);

	/// Creates an encoded address from \a address.
	std::string AddressToString(const Address& address);

	/// Creates an encoded address from a public key (\a publicKey) for the network identified by \a networkIdentifier.
	std::string PublicKeyToAddressString(const Key& publicKey, NetworkIdentifier networkIdentifier);

	/// Creates an address from a public key (\a publicKey) for the network identified by \a networkIdentifier.
	Address PublicKeyToAddress(const Key& publicKey, NetworkIdentifier networkIdentifier);

	/// Gets a value indicating whether or not the given \a address is valid for the network identified by \a networkIdentifier.
	bool IsValidAddress(const Address& address, NetworkIdentifier networkIdentifier);

	/// Gets a value indicating whether or not the given \a encoded address is valid for the
	/// network identified by \a networkIdentifier.
	bool IsValidEncodedAddress(const std::string& encoded, NetworkIdentifier networkIdentifier);

	/// Tries to parse \a str into an Address (\a parsedValue).
	bool TryParseValue(const std::string& str, Address& parsedValue);
}}

/// Defines support for Address as a configuration value and conditionally allows empty values based on \a ALLOW_EMPTY.
#define DEFINE_ADDRESS_CONFIGURATION_VALUE_SUPPORT_ALLOW_EMPTY(ALLOW_EMPTY) \
	namespace catapult { namespace utils { \
		static bool TryParseValue(const std::string& str, Address& parsedValue) { \
			return (ALLOW_EMPTY && str.empty()) || model::TryParseValue(str, parsedValue); \
		} \
	}}

/// Defines support for Address as a configuration value and does not allow empty values.
#define DEFINE_ADDRESS_CONFIGURATION_VALUE_SUPPORT DEFINE_ADDRESS_CONFIGURATION_VALUE_SUPPORT_ALLOW_EMPTY(false)
