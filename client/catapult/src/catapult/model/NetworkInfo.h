#pragma once
#include "catapult/types.h"

namespace catapult { namespace model {

#define NETWORK_IDENTIFIER_LIST \
	/* A default (zero) identifier that does not identify any known network. */ \
	ENUM_VALUE(Zero, 0) \
	/* The mijin network identifier. */ \
	ENUM_VALUE(Mijin, 0x60) \
	/* The mijin test network identifier. */ \
	ENUM_VALUE(Mijin_Test, 0x90) \
	/* The public main network identifier. */ \
	ENUM_VALUE(Public, 0x68) \
	/* The public test network identifier. */ \
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
		/// The network identifier.
		NetworkIdentifier Identifier;

		/// The nemesis public key.
		Key PublicKey;

		/// The nemesis generation hash.
		Hash256 GenerationHash;
	};

	/// Tries to parse \a networkName into a network identifier (\a networkIdentifier).
	bool TryParseValue(const std::string& networkName, NetworkIdentifier& networkIdentifier);
}}
