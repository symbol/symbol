#pragma once
#include "catapult/utils/HexFormatter.h"
#include "catapult/types.h"

namespace catapult { namespace crypto { class PrivateKey; } }

namespace catapult { namespace crypto {

	/// Formats a public \a key for printing.
	utils::ContainerHexFormatter<Key::const_iterator> FormatKey(const Key& key);

	/// Formats a private \a key for printing.
	utils::ContainerHexFormatter<Key::const_iterator> FormatKey(const PrivateKey& key);

	/// Parses a key from a string (\a keyString) and returns the result.
	Key ParseKey(const std::string& keyString);

	/// Returns \c true if \a str represents a valid public key, \c false otherwise.
	bool IsValidKeyString(const std::string& str);
}}
