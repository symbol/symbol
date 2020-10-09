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
#include <string>

namespace catapult { namespace model {

	// region NetworkIdentifier

/// \note The lower 3 bits must be cleared because they are used for different purposes, e.g. resolvers.
#define NETWORK_IDENTIFIER_LIST \
	/* Default (zero) identifier that does not identify any known network. */ \
	ENUM_VALUE(Zero, 0) \
	\
	/* Mijin network identifier. */ \
	ENUM_VALUE(Mijin, 0x60) \
	\
	/* Mijin test network identifier. */ \
	ENUM_VALUE(Mijin_Test, 0x90) \
	\
	/* Private network identifier. */ \
	ENUM_VALUE(Private, 0x78) \
	\
	/* Private test network identifier. */ \
	ENUM_VALUE(Private_Test, 0x80) \
	\
	/* Public network identifier. */ \
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

	/// Tries to parse \a networkName into a network identifier (\a networkIdentifier).
	bool TryParseValue(const std::string& networkName, NetworkIdentifier& networkIdentifier);

	// endregion

	// region UniqueNetworkFingerprint

	/// Globally unique network fingerprint.
	struct UniqueNetworkFingerprint {
	public:
		/// Creates default fingerprint.
		UniqueNetworkFingerprint();

		/// Creates fingerprint around \a identifier.
		explicit UniqueNetworkFingerprint(NetworkIdentifier identifier);

		/// Creates fingerprint around \a identifier and \a generationHashSeed.
		UniqueNetworkFingerprint(NetworkIdentifier identifier, const catapult::GenerationHashSeed& generationHashSeed);

	public:
		/// Network identifier.
		NetworkIdentifier Identifier;

		/// Nemesis generation hash seed.
		catapult::GenerationHashSeed GenerationHashSeed;

	public:
		/// Returns \c true if this fingerprint is equal to \a rhs.
		bool operator==(const UniqueNetworkFingerprint& rhs) const;

		/// Returns \c true if this fingerprint is not equal to \a rhs.
		bool operator!=(const UniqueNetworkFingerprint& rhs) const;
	};

	/// Insertion operator for outputting \a fingerprint to \a out.
	std::ostream& operator<<(std::ostream& out, const UniqueNetworkFingerprint& fingerprint);

	// endregion
}}
