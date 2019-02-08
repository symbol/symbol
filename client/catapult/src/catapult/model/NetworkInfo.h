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
#include "catapult/types.h"

namespace catapult { namespace model {

/// \note The lower 3 bits must be cleared because they are used for different purposes, e.g. resolvers.
#define NETWORK_IDENTIFIER_LIST \
	/* A default (zero) identifier that does not identify any known network. */ \
	ENUM_VALUE(Zero, 0) \
	\
	/* Mijin network identifier. */ \
	ENUM_VALUE(Mijin, 0x60) \
	\
	/* Mijin test network identifier. */ \
	ENUM_VALUE(Mijin_Test, 0x90) \
	\
	/* Public main network identifier. */ \
	ENUM_VALUE(Public, 0x68) \
	\
	/* Public test network identifier. */ \
	ENUM_VALUE(Public_Test, 0x98)

#define ENUM_VALUE(LABEL, VALUE) LABEL = VALUE,
	/// Possible network identifiers.
	enum class NetworkIdentifier : uint8_t {
		NETWORK_IDENTIFIER_LIST
	};
#undef ENUM_VALUE

	/// Insertion operator for outputting \a value to \a out.
	std::ostream& operator<<(std::ostream& out, NetworkIdentifier value);

	/// Information about a network.
	struct NetworkInfo {
	public:
		/// Creates a default, uninitialized network info.
		constexpr NetworkInfo() : NetworkInfo(NetworkIdentifier::Zero, {}, {})
		{}

		/// Creates a network info around a network \a identifier, a nemesis public key (\a publicKey)
		/// and a nemesis generation hash (\a generationHash).
		constexpr NetworkInfo(NetworkIdentifier identifier, const Key& publicKey, const Hash256& generationHash)
				: Identifier(identifier)
				, PublicKey(publicKey)
				, GenerationHash(generationHash)
		{}

	public:
		/// Network identifier.
		NetworkIdentifier Identifier;

		/// Nemesis public key.
		Key PublicKey;

		/// Nemesis generation hash.
		Hash256 GenerationHash;
	};

	/// Tries to parse \a networkName into a network identifier (\a networkIdentifier).
	bool TryParseValue(const std::string& networkName, NetworkIdentifier& networkIdentifier);
}}
